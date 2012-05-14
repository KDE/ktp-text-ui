/***************************************************************************
 *   Copyright (C) 2010 by David Edmundson <kde@davidedmundson.co.uk>      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#include "chat-widget.h"

#include "ui_chat-widget.h"
#include "adium-theme-header-info.h"
#include "adium-theme-content-info.h"
#include "adium-theme-message-info.h"
#include "adium-theme-status-info.h"
#include "channel-contact-model.h"
#include "logmanager.h"

#include <QtGui/QKeyEvent>
#include <QtGui/QAction>

#include <KColorDialog>
#include <KNotification>
#include <KAboutData>
#include <KComponentData>
#include <KDebug>
#include <KColorScheme>
#include <KLineEdit>

#include <TelepathyQt/Account>
#include <TelepathyQt/Message>
#include <TelepathyQt/Types>
#include <TelepathyQt/AvatarData>
#include <TelepathyQt/Connection>
#include <TelepathyQt/Presence>

#include <KTp/presence.h>
#include "message-processor.h"

class ChatWidgetPrivate
{
public:
    ChatWidgetPrivate()
    {
        isGroupChat = false;
        remoteContactChatState = Tp::ChannelChatStateInactive;
    }
    /** Stores whether the channel is ready with all contacts upgraded*/
    bool chatviewlInitialised;
    Tp::ChannelChatState remoteContactChatState;
    QAction *showFormatToolbarAction;
    bool isGroupChat;
    QString title;
    QString contactName;
    QString yourName;
    Tp::TextChannelPtr channel;
    Tp::AccountPtr account;
    Ui::ChatWidget ui;
    ChannelContactModel *contactModel;
    LogManager *logManager;
    QTimer *pausedStateTimer;

    KComponentData telepathyComponentData();
};


//FIXME I would like this to be part of the main KDE Telepathy library as a static function somewhere.
KComponentData ChatWidgetPrivate::telepathyComponentData()
{
    KAboutData telepathySharedAboutData("ktelepathy",0,KLocalizedString(),0);
    return KComponentData(telepathySharedAboutData);
}

ChatWidget::ChatWidget(const Tp::TextChannelPtr & channel, const Tp::AccountPtr &account, QWidget *parent)
    : QWidget(parent),
      d(new ChatWidgetPrivate)
{
    d->channel = channel;
    d->account = account;
    d->logManager = new LogManager(this);


    //load translations for this library. keep this before any i18n() calls in library code
    KGlobal::locale()->insertCatalog(QLatin1String("ktpchat"));

    d->chatviewlInitialised = false;
    d->showFormatToolbarAction = new QAction(i18n("Show format options"), this);
    d->isGroupChat = (channel->targetHandleType() == Tp::HandleTypeContact ? false : true);

    d->ui.setupUi(this);
    d->ui.formatToolbar->show();
    d->ui.formatColor->setText(QString());
    d->ui.formatColor->setIcon(KIcon(QLatin1String("format-text-color")));

    d->ui.formatBold->setText(QString());
    d->ui.formatBold->setIcon(KIcon(QLatin1String("format-text-bold")));

    d->ui.formatItalic->setText(QString());
    d->ui.formatItalic->setIcon(KIcon(QLatin1String("format-text-italic")));

    d->ui.formatUnderline->setText(QString());
    d->ui.formatUnderline->setIcon(KIcon(QLatin1String("format-text-underline")));

    d->ui.insertEmoticon->setText(QString());
    d->ui.insertEmoticon->setIcon(KIcon(QLatin1String("face-smile")));

    // connect channel signals
    setupChannelSignals();

    // create contactModel and start keeping track of contacts.
    d->contactModel = new ChannelContactModel(d->channel, this);
    setupContactModelSignals();

    d->ui.contactsView->setModel(d->contactModel);

    d->yourName = channel->groupSelfContact()->alias();

    d->ui.chatArea->load((d->isGroupChat?AdiumThemeView::GroupChat:AdiumThemeView::SingleUserChat));

    AdiumThemeHeaderInfo info;

    info.setGroupChat(d->isGroupChat);
    //normal chat - self and one other person.
    if (d->isGroupChat) {
        info.setChatName(d->channel->targetId());
    }
    else
    {
        Tp::ContactPtr otherContact = d->channel->targetContact();

        Q_ASSERT(otherContact);

        d->contactName = otherContact->alias();
        info.setDestinationDisplayName(otherContact->alias());
        info.setDestinationName(otherContact->id());
        info.setChatName(otherContact->alias());
        info.setIncomingIconPath(otherContact->avatarData().fileName);
        d->ui.contactsView->hide();
    }

    info.setSourceName(d->channel->connection()->protocolName());

    //set up anything related to 'self'
    info.setOutgoingIconPath(d->channel->groupSelfContact()->avatarData().fileName);
    info.setTimeOpened(QDateTime::currentDateTime());
    info.setServiceIconImage(KIconLoader::global()->iconPath(d->account->iconName(), KIconLoader::Panel));
    connect(d->ui.chatArea, SIGNAL(loadFinished(bool)), SLOT(chatViewReady()), Qt::QueuedConnection);
    d->ui.chatArea->initialise(info);

    //set the title of this chat.
    d->title = info.chatName();

    //format toolbar visibility
    d->showFormatToolbarAction->setCheckable(true);
    connect(d->showFormatToolbarAction, SIGNAL(toggled(bool)),
            d->ui.formatToolbar, SLOT(setVisible(bool)));
    d->ui.sendMessageBox->addAction(d->showFormatToolbarAction);

    //FIXME load whether to show/hide by default from config file (do per account)
    bool formatToolbarIsVisible = false;
    d->ui.formatToolbar->setVisible(formatToolbarIsVisible);
    d->showFormatToolbarAction->setChecked(formatToolbarIsVisible);

    d->ui.sendMessageBox->setSpellCheckingLanguage(KGlobal::locale()->language());

    d->pausedStateTimer = new QTimer(this);
    d->pausedStateTimer->setSingleShot(true);

    //connect signals/slots from format toolbar
    connect(d->ui.formatColor, SIGNAL(released()), SLOT(onFormatColorReleased()));
    connect(d->ui.formatBold, SIGNAL(toggled(bool)), d->ui.sendMessageBox, SLOT(setFontBold(bool)));
    connect(d->ui.formatItalic, SIGNAL(toggled(bool)), d->ui.sendMessageBox, SLOT(setFontItalic(bool)));
    connect(d->ui.formatUnderline, SIGNAL(toggled(bool)), d->ui.sendMessageBox, SLOT(setFontUnderline(bool)));

    // make the sendMessageBox a focus proxy for the chatview
    d->ui.chatArea->setFocusProxy(d->ui.sendMessageBox);
    connect(d->ui.sendMessageBox, SIGNAL(returnKeyPressed()), SLOT(sendMessage()));

    connect(d->ui.searchBar, SIGNAL(findTextSignal(QString,QWebPage::FindFlags)), this, SLOT(findTextInChat(QString,QWebPage::FindFlags)));
    connect(d->ui.searchBar, SIGNAL(findNextSignal(QString,QWebPage::FindFlags)), this, SLOT(findNextTextInChat(QString,QWebPage::FindFlags)));
    connect(d->ui.searchBar, SIGNAL(findPreviousSignal(QString,QWebPage::FindFlags)), this, SLOT(findPreviousTextInChat(QString,QWebPage::FindFlags)));
    connect(d->ui.searchBar, SIGNAL(flagsChangedSignal(QString,QWebPage::FindFlags)), this, SLOT(findTextInChat(QString,QWebPage::FindFlags)));

    connect(this, SIGNAL(searchTextComplete(bool)), d->ui.searchBar, SLOT(onSearchTextComplete(bool)));

    connect(d->pausedStateTimer, SIGNAL(timeout()), this, SLOT(onChatPausedTimerExpired()));

    // initialize LogManager
    if (!d->isGroupChat) {
        d->logManager->setFetchAmount(3);
        d->logManager->setTextChannel(d->account, d->channel);
        m_previousConversationAvailable = d->logManager->exists();
    } else {
        m_previousConversationAvailable = false;
    }
}

ChatWidget::~ChatWidget()
{
    d->channel->requestClose(); // ensure closing; does nothing, if already closed
    delete d;
}

void ChatWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        d->ui.retranslateUi(this);
        break;
    default:
        break;
    }
}

void ChatWidget::resizeEvent(QResizeEvent *e)
{
    //set the maximum height of a text box to a third of the total window height (but no smaller than the minimum size)
    int textBoxHeight = e->size().height() / 3;
    if (textBoxHeight < d->ui.sendMessageBox->minimumSizeHint().height()) {
        textBoxHeight = d->ui.sendMessageBox->minimumSizeHint().height();
    }
    d->ui.sendMessageBox->setMaximumHeight(textBoxHeight);
    QWidget::resizeEvent(e);
}

Tp::AccountPtr ChatWidget::account() const
{
    return d->account;
}

KIcon ChatWidget::icon() const
{
    if (d->channel->connection()->status() == Tp::ConnectionStatusConnected) {
        //normal chat - self and one other person.
        if (!d->isGroupChat) {
            //find the other contact which isn't self.
            Q_FOREACH(const Tp::ContactPtr & contact, d->channel->groupContacts()) {
                if (contact != d->channel->groupSelfContact()) {
                    return KTp::Presence(contact->presence()).icon();
                }
            }
        }
        else {
            //group chat
            return KTp::Presence(Tp::Presence::available()).icon();
        }
    }
    return KTp::Presence(Tp::Presence::offline()).icon();
}

KIcon ChatWidget::accountIcon() const
{
    return KIcon(d->account->iconName());
}

bool ChatWidget::isGroupChat() const
{
    return d->isGroupChat;
}

ChatSearchBar *ChatWidget::chatSearchBar() const
{
    return d->ui.searchBar;
}

void ChatWidget::setChatEnabled(bool enable)
{
    d->ui.sendMessageBox->setEnabled(enable);

    // show a message informing the user
    AdiumThemeStatusInfo statusMessage;

    if (!enable) {
        statusMessage.setMessage(i18n("Connection closed"));
    } else {
        statusMessage.setMessage(i18nc("Connected to IM service", "Connected"));
    }
    statusMessage.setService(d->channel->connection()->protocolName());
    statusMessage.setTime(QDateTime::currentDateTime());
    d->ui.chatArea->addStatusMessage(statusMessage);

    Q_EMIT iconChanged(icon());
}

void ChatWidget::setTextChannel(const Tp::TextChannelPtr &newTextChannelPtr)
{
    d->channel = newTextChannelPtr;     // set the new channel
    d->contactModel->setTextChannel(newTextChannelPtr);

    // connect signals for the new textchannel
    setupChannelSignals();
}

Tp::TextChannelPtr ChatWidget::textChannel() const
{
    return d->channel;
}

void ChatWidget::keyPressEvent(QKeyEvent *e)
{
    if (e->matches(QKeySequence::Copy)) {
        d->ui.chatArea->triggerPageAction(QWebPage::Copy);
        return;
    }

    if (e->key() == Qt::Key_Escape && d->ui.searchBar->isVisible()) {
        d->ui.searchBar->toggleView(false);
        return;
    }

    if (e->key() == Qt::Key_PageUp ||
        e->key() == Qt::Key_PageDown) {
        d->ui.chatArea->event(e);
        return;
    }

    QWidget::keyPressEvent(e);
}

QString ChatWidget::title() const
{
    return d->title;
}

QColor ChatWidget::titleColor() const
{
    /*return a color to set the tab text as in order of importance
    typing
    unread messages
    user offline

    */

    KColorScheme scheme(QPalette::Active, KColorScheme::Window);

    if (d->remoteContactChatState == Tp::ChannelChatStateComposing) {
        kDebug() << "remote is typing";
        return scheme.foreground(KColorScheme::PositiveText).color();
    }

    if (unreadMessageCount() > 0 && !isOnTop()) {
        kDebug() << "unread messages";
        return scheme.foreground(KColorScheme::ActiveText).color();
    }

    //normal chat - self and one other person.
    if (!d->isGroupChat) {
        //find the other contact which isn't self.
        Q_FOREACH(const Tp::ContactPtr & contact, d->channel->groupContacts()) {
            if (contact != d->channel->groupSelfContact()) {
                if (contact->presence().type() == Tp::ConnectionPresenceTypeOffline ||
                    contact->presence().type() == Tp::ConnectionPresenceTypeHidden) {
                    return scheme.foreground(KColorScheme::InactiveText).color();
                }
            }
        }
    }

    return scheme.foreground(KColorScheme::NormalText).color();
}

void ChatWidget::toggleSearchBar() const
{
    if(d->ui.searchBar->isVisible()) {
        d->ui.searchBar->toggleView(false);
    } else {
        d->ui.searchBar->toggleView(true);
    }
}

void ChatWidget::setupChannelSignals()
{
    connect(d->channel.data(), SIGNAL(messageReceived(Tp::ReceivedMessage)),
            SLOT(handleIncomingMessage(Tp::ReceivedMessage)));
    connect(d->channel.data(), SIGNAL(messageReceived(Tp::ReceivedMessage)),
            SLOT(notifyAboutIncomingMessage(Tp::ReceivedMessage)));
    connect(d->channel.data(), SIGNAL(pendingMessageRemoved(Tp::ReceivedMessage)),
            SIGNAL(unreadMessagesChanged()));
    connect(d->channel.data(), SIGNAL(messageSent(Tp::Message,Tp::MessageSendingFlags,QString)),
            SLOT(handleMessageSent(Tp::Message,Tp::MessageSendingFlags,QString)));
    connect(d->channel.data(), SIGNAL(chatStateChanged(Tp::ContactPtr,Tp::ChannelChatState)),
            SLOT(onChatStatusChanged(Tp::ContactPtr,Tp::ChannelChatState)));
    connect(d->channel.data(), SIGNAL(invalidated(Tp::DBusProxy*,QString,QString)),
            this, SLOT(onChannelInvalidated()));

    if (d->channel->hasChatStateInterface()) {
        connect(d->ui.sendMessageBox, SIGNAL(textChanged()), SLOT(onInputBoxChanged()));
    }
}

void ChatWidget::setupContactModelSignals()
{
    connect(d->contactModel, SIGNAL(contactPresenceChanged(Tp::ContactPtr,KTp::Presence)),
            SLOT(onContactPresenceChange(Tp::ContactPtr,KTp::Presence)));
    connect(d->contactModel, SIGNAL(contactAliasChanged(Tp::ContactPtr,QString)),
            SLOT(onContactAliasChanged(Tp::ContactPtr,QString)));
    connect(d->contactModel, SIGNAL(contactBlockStatusChanged(Tp::ContactPtr,bool)),
       SLOT(onContactBlockStatusChanged(Tp::ContactPtr,bool)));
}


void ChatWidget::onHistoryFetched(const QList<AdiumThemeContentInfo> &messages)
{
    kDebug() << "found" << messages.count() << "messages in history";
    Q_FOREACH(const AdiumThemeContentInfo &message, messages) {
        d->ui.chatArea->addContentMessage(message);
    }

    d->chatviewlInitialised = true;

    //process any messages we've 'missed' whilst initialising.
    Q_FOREACH(const Tp::ReceivedMessage &message, d->channel->messageQueue()) {
        handleIncomingMessage(message);
    }
}

int ChatWidget::unreadMessageCount() const
{
    return d->channel->messageQueue().size();
}

void ChatWidget::acknowledgeMessages()
{
    kDebug();
    //if we're not initialised we can't have shown anything, even if we are on top, therefore ignore all requests to do so
    if (d->chatviewlInitialised) {
        //acknowledge everything in the message queue.
        d->channel->acknowledge(d->channel->messageQueue());
    }
}

bool ChatWidget::isOnTop() const
{
    kDebug() << ( isActiveWindow() && isVisible() );
    return ( isActiveWindow() && isVisible() );
}

void ChatWidget::handleIncomingMessage(const Tp::ReceivedMessage &message)
{
    kDebug() << title() << message.text();

    if (d->chatviewlInitialised) {

        //debug the message parts (looking for HTML etc)
//        Q_FOREACH(Tp::MessagePart part, message.parts())
//        {
//            qDebug() << "***";
//            Q_FOREACH(QString key, part.keys())
//            {
//                qDebug() << key << part.value(key).variant();
//            }
//        }
//      turns out we have no HTML, because no CM supports it yet

        if (message.isDeliveryReport()) {
            QString text;
            AdiumThemeStatusInfo messageInfo;
            Tp::ReceivedMessage::DeliveryDetails reportDetails = message.deliveryDetails();

            if (reportDetails.hasDebugMessage()) {
                kDebug() << "delivery report debug message: " << reportDetails.debugMessage();
            }

            if (reportDetails.isError()) {
                switch (reportDetails.error()) {
                case Tp::ChannelTextSendErrorOffline:
                    if (reportDetails.hasEchoedMessage()) {
                        if(message.sender()->isBlocked()) {
                            text = i18n("Delivery of the message \"%1\" "
                                        "failed because the remote contact is blocked",
                                        reportDetails.echoedMessage().text());
                         } else {
                            text = i18n("Delivery of the message \"%1\" "
                                        "failed because the remote contact is offline",
                                        reportDetails.echoedMessage().text());
                         }
                    } else {
                        if(message.sender()->isBlocked()) {
                            text = i18n("Delivery of a message failed "
                                        "because the remote contact is blocked");
                        } else {
                            text = i18n("Delivery of a message failed "
                                        "because the remote contact is offline");
                        }
                    }
                    break;
                case Tp::ChannelTextSendErrorInvalidContact:
                    if (reportDetails.hasEchoedMessage()) {
                        text = i18n("Delivery of the message \"%1\" "
                                    "failed because the remote contact is not valid",
                                    reportDetails.echoedMessage().text());
                    } else {
                        text = i18n("Delivery of a message failed "
                                    "because the remote contact is not valid");
                    }
                    break;
                case Tp::ChannelTextSendErrorPermissionDenied:
                    if (reportDetails.hasEchoedMessage()) {
                        text = i18n("Delivery of the message \"%1\" failed because "
                                    "you do not have permission to speak in this room",
                                    reportDetails.echoedMessage().text());
                    } else {
                        text = i18n("Delivery of a message failed because "
                                    "you do not have permission to speak in this room");
                    }
                    break;
                case Tp::ChannelTextSendErrorTooLong:
                    if (reportDetails.hasEchoedMessage()) {
                        text = i18n("Delivery of the message \"%1\" "
                                    "failed because it was too long",
                                    reportDetails.echoedMessage().text());
                    } else {
                        text = i18n("Delivery of a message failed "
                                    "because it was too long");
                    }
                    break;
                default:
                    if (reportDetails.hasEchoedMessage()) {
                        text = i18n("Delivery of the message \"%1\" failed: %2",
                                    reportDetails.echoedMessage().text(),
                                    message.text());
                    } else {
                        text = i18n("Delivery of a message failed: %1", message.text());
                    }
                    break;
                }
            } else {
                //TODO: handle delivery reports properly
                kWarning() << "Ignoring delivery report";
                d->channel->acknowledge(QList<Tp::ReceivedMessage>() << message);
                return;
            }

            messageInfo.setMessage(text);
            messageInfo.setTime(message.received());
            messageInfo.setStatus(QLatin1String("error"));

            d->ui.chatArea->addStatusMessage(messageInfo);
        } else if (message.messageType() == Tp::ChannelTextMessageTypeAction) {
            //a "/me " message

            AdiumThemeStatusInfo statusMessage;
            statusMessage.setTime(message.received());

            QString senderName;
            if (message.sender().isNull()) {
                senderName = message.senderNickname();
            } else {
                senderName = message.sender()->alias();
            }

            statusMessage.setMessage(QString::fromLatin1("%1 %2").arg(senderName, message.text()));
            d->ui.chatArea->addStatusMessage(statusMessage);
        } else {
            AdiumThemeContentInfo messageInfo(AdiumThemeMessageInfo::RemoteToLocal);

            messageInfo.setMessage(MessageProcessor::instance()->processIncomingMessage(message).finalizedMessage());

            QDateTime time = message.sent();
            if (!time.isValid()) {
                time = message.received();
            }
            messageInfo.setTime(time);

            //sender can have just an ID or be a full contactPtr. Use full contact info if available.
            if (message.sender().isNull()) {
                messageInfo.setSenderDisplayName(message.senderNickname());
            } else {
                messageInfo.setUserIconPath(message.sender()->avatarData().fileName);
                messageInfo.setSenderDisplayName(message.sender()->alias());
                messageInfo.setSenderScreenName(message.sender()->id());
            }

            d->ui.chatArea->addContentMessage(messageInfo);
        }

        //if the window is on top, ack straight away. Otherwise they stay in the message queue for acking when activated..
        if (isOnTop()) {
            d->channel->acknowledge(QList<Tp::ReceivedMessage>() << message);
        } else {
            Q_EMIT unreadMessagesChanged();
        }
    }

}

void ChatWidget::notifyAboutIncomingMessage(const Tp::ReceivedMessage & message)
{
    //send the correct notification:
    QString notificationType;
    //choose the correct notification type:
    //options are:
    // kde_telepathy_contact_incoming
    // kde_telepathy_contact_incoming_active_window - TODO - requires information not available yet.
    //FIXME: until the above is available, simply deactivate the event
    if(isOnTop()) {
        kDebug() << "Widget is on top, not doing anything";
        return;
    }
    // don't notify of messages sent by self from another computer
    if (message.sender() == d->channel->groupSelfContact()) {
        return;
    }
    
    if (message.isDeliveryReport()) {
        return;
    }
    
    // kde_telepathy_contact_highlight (contains your name)
    // kde_telepathy_info_event

    //if the message text contains sender name, it's a "highlighted message"
    //TODO DrDanz suggested this could be a configurable list of words that make it highlighted.(seems like a good idea to me)
    if(message.text().contains(d->channel->groupSelfContact()->alias())) {
        notificationType = QLatin1String("kde_telepathy_contact_highlight");
    } else if(message.messageType() == Tp::ChannelTextMessageTypeNotice) {
        notificationType = QLatin1String("kde_telepathy_info_event");
    } else {
        notificationType = QLatin1String("kde_telepathy_contact_incoming");
    }


    KNotification *notification = new KNotification(notificationType, this,
                                                    KNotification::RaiseWidgetOnActivation
                                                    | KNotification::CloseWhenWidgetActivated
                                                    | KNotification::Persistent);
    notification->setComponentData(d->telepathyComponentData());
    notification->setTitle(i18n("%1 has sent you a message", message.sender()->alias()));

    QPixmap notificationPixmap;
    if (notificationPixmap.load(message.sender()->avatarData().fileName)) {
        notification->setPixmap(notificationPixmap);
    }

    // Remove empty lines from message
    QString notifyText = message.text().simplified();
    if (notifyText.length() > 170) {
        //search for the closest space in text
        QString truncatedMsg = notifyText.left(notifyText.indexOf(QLatin1Char(' '), 150)).append(QLatin1String("..."));
        notification->setText(truncatedMsg);
    } else {
        notification->setText(notifyText);
    }

    //allows per contact notifications
    notification->addContext(QLatin1String("contact"), message.sender()->id());
    //TODO notification->addContext("group",... Requires KDE Telepathy Contact to work out which group they are in.

    notification->setActions(QStringList(i18n("View")));
    connect(notification, SIGNAL(activated(uint)), this, SIGNAL(notificationClicked()));

    notification->sendEvent();
}

void ChatWidget::handleMessageSent(const Tp::Message &message, Tp::MessageSendingFlags, const QString&) /*Not sure what these other args are for*/
{
    Tp::ContactPtr sender = d->channel->groupSelfContact();

    if (message.messageType() == Tp::ChannelTextMessageTypeAction) {
        AdiumThemeStatusInfo statusMessage;
        statusMessage.setTime(message.sent());
        statusMessage.setMessage(QString::fromLatin1("%1 %2").arg(sender->alias(), message.text()));
        d->ui.chatArea->addStatusMessage(statusMessage);
    }
    else {
        AdiumThemeContentInfo messageInfo(AdiumThemeMessageInfo::LocalToRemote);
        messageInfo.setMessage(MessageProcessor::instance()->processOutgoingMessage(message).finalizedMessage());

        messageInfo.setTime(message.sent());

        messageInfo.setSenderDisplayName(sender->alias());
        messageInfo.setSenderScreenName(sender->id());
        messageInfo.setUserIconPath(sender->avatarData().fileName);
        d->ui.chatArea->addContentMessage(messageInfo);
    }

    //send the notification that a message has been sent
    KNotification *notification = new KNotification(QLatin1String("kde_telepathy_outgoing"), this);
    notification->setComponentData(d->telepathyComponentData());
    notification->setTitle(i18n("You have sent a message"));
    QPixmap notificationPixmap;
    if (notificationPixmap.load(sender->avatarData().fileName)) {
        notification->setPixmap(notificationPixmap);
    }
    notification->setText(message.text());
    notification->sendEvent();
}

void ChatWidget::chatViewReady()
{
    connect(d->logManager, SIGNAL(fetched(QList<AdiumThemeContentInfo>)), SLOT(onHistoryFetched(QList<AdiumThemeContentInfo>)));
    d->logManager->fetchLast();
}


void ChatWidget::sendMessage()
{
    QString message = d->ui.sendMessageBox->toPlainText();

    if (!message.isEmpty()) {
        if (d->channel->supportsMessageType(Tp::ChannelTextMessageTypeAction) && message.startsWith(QLatin1String("/me "))) {
            //remove "/me " from the start of the message
            message.remove(0,4);
            d->channel->send(message, Tp::ChannelTextMessageTypeAction);
        } else {
            d->channel->send(message);
        }
        d->ui.sendMessageBox->clear();
    }
}

void ChatWidget::onChatStatusChanged(const Tp::ContactPtr & contact, Tp::ChannelChatState state)
{
    //don't show our own status changes.
    if (contact == d->channel->groupSelfContact()) {
        return;
    }

    if (state == Tp::ChannelChatStateGone) {
        AdiumThemeStatusInfo statusMessage;
        statusMessage.setMessage(i18n("%1 has left the chat", contact->alias()));
        statusMessage.setService(d->channel->connection()->protocolName());
        statusMessage.setStatus(QLatin1String("away"));
        statusMessage.setTime(QDateTime::currentDateTime());
        d->ui.chatArea->addStatusMessage(statusMessage);
    }

    if (d->isGroupChat) {
        //In a multiperson chat just because this user is no longer typing it doesn't mean that no-one is.
        //loop through each contact, check no-one is in composing mode.

        Tp::ChannelChatState tempState = Tp::ChannelChatStateInactive;

        Q_FOREACH (const Tp::ContactPtr & contact, d->channel->groupContacts()) {
            if (contact == d->channel->groupSelfContact()) {
                continue;
            }

            tempState = d->channel->chatState(contact);

            if (tempState == Tp::ChannelChatStateComposing) {
                state = tempState;
                break;
            } else if (tempState == Tp::ChannelChatStatePaused && state != Tp::ChannelChatStateComposing) {
                state = tempState;
            }
        }
    }

    if (state != d->remoteContactChatState) {
        d->remoteContactChatState = state;
        Q_EMIT userTypingChanged(state);
    }
}



void ChatWidget::onContactPresenceChange(const Tp::ContactPtr & contact, const KTp::Presence &presence)
{
    QString message;
    bool isYou = (contact == d->channel->groupSelfContact());

    if (isYou) {
        message = i18n("You are now marked as %1", presence.displayString());
    }
    else {
        if (presence.statusMessage().isEmpty()) {
            message = i18nc("User's name, with their new presence status (i.e online/away)","%1 is %2", contact->alias(), presence.displayString());
        } else {
            message = i18nc("User's name, with their new presence status (i.e online/away) and a sepecified presence message","%1 is %2 - %3",
                            contact->alias(),
                            presence.displayString(),
                            presence.statusMessage());
        }
    }

    if (!message.isNull()) {
        AdiumThemeStatusInfo statusMessage;
        statusMessage.setMessage(message);
        statusMessage.setService(d->channel->connection()->protocolName());
        statusMessage.setTime(QDateTime::currentDateTime());
        d->ui.chatArea->addStatusMessage(statusMessage);
    }

    //if in a non-group chat situation, and the other contact has changed state...
    if (!d->isGroupChat && !isYou) {
        Q_EMIT iconChanged(presence.icon());
    }

    Q_EMIT contactPresenceChanged(presence);
}

void ChatWidget::onContactAliasChanged(const Tp::ContactPtr & contact, const QString& alias)
{
    QString message;
    bool isYou = (contact == d->channel->groupSelfContact());

    if (isYou) {
        if (d->yourName != alias) {
            message = i18n("You are now known as %1", alias);
            d->yourName = alias;
        }
    } else if (!d->isGroupChat) {
        //HACK the title is the contact alias on non-groupchats,
        //but we should have a better way of keeping the previous
        //aliases of all contacts
        if (d->contactName != alias) {
            message = i18n("%1 is now known as %2", d->contactName, alias);
            d->contactName = alias;
        }
    }

    if (!message.isEmpty()) {
        AdiumThemeStatusInfo statusMessage;
        statusMessage.setMessage(message);
        statusMessage.setService(d->channel->connection()->protocolName());
        statusMessage.setTime(QDateTime::currentDateTime());
        d->ui.chatArea->addStatusMessage(statusMessage);
    }

    //if in a non-group chat situation, and the other contact has changed alias...
    if (!d->isGroupChat && !isYou) {
        Q_EMIT titleChanged(alias);
    }
}

void ChatWidget::onContactBlockStatusChanged(const Tp::ContactPtr &contact, bool blocked)
{
    QString message;
    if(blocked) {
        message = i18n("%1 is now blocked.", contact->alias());
    } else {
        message = i18n("%1 is now unblocked.", contact->alias());
    }

    AdiumThemeStatusInfo statusMessage;
    statusMessage.setMessage(message);
    statusMessage.setService(d->channel->connection()->protocolName());
    statusMessage.setTime(QDateTime::currentDateTime());
    d->ui.chatArea->addStatusMessage(statusMessage);

    Q_EMIT contactBlockStatusChanged(blocked);
}

void ChatWidget::onChannelInvalidated()
{
    setChatEnabled(false);
}

void ChatWidget::onInputBoxChanged()
{
    //if the box is empty
    bool textBoxEmpty = !d->ui.sendMessageBox->toPlainText().isEmpty();

    //FIXME buffer what we've sent to telepathy, make this more efficient.
    if(textBoxEmpty) {
        d->channel->requestChatState(Tp::ChannelChatStateComposing);
        d->pausedStateTimer->start(5000);
    } else {
        d->channel->requestChatState(Tp::ChannelChatStateActive);
        d->pausedStateTimer->stop();
    }
}

void ChatWidget::findTextInChat(const QString& text, QWebPage::FindFlags flags)
{
    // reset highlights
    d->ui.chatArea->findText(QString(), flags);

    if(d->ui.chatArea->findText(text, flags)) {
        Q_EMIT searchTextComplete(true);
    } else {
        Q_EMIT searchTextComplete(false);
    }
}

void ChatWidget::findNextTextInChat(const QString& text, QWebPage::FindFlags flags)
{
    d->ui.chatArea->findText(text, flags);
}

void ChatWidget::findPreviousTextInChat(const QString& text, QWebPage::FindFlags flags)
{
    // for "backwards" search
    flags |= QWebPage::FindBackward;
    d->ui.chatArea->findText(text, flags);
}

void ChatWidget::onFormatColorReleased()
{
    QColor color;
    KColorDialog::getColor(color,this);
    d->ui.sendMessageBox->setTextColor(color);
}

void ChatWidget::setSpellDictionary(const QString &dict)
{
    d->ui.sendMessageBox->setSpellCheckingLanguage(dict);
}

QString ChatWidget::spellDictionary() const
{
    return d->ui.sendMessageBox->spellCheckingLanguage();
}

Tp::ChannelChatState ChatWidget::remoteChatState()
{
    return d->remoteContactChatState;
}

bool ChatWidget::previousConversationAvailable()
{
    return m_previousConversationAvailable;
}

void ChatWidget::onChatPausedTimerExpired()
{
        d->channel->requestChatState(Tp::ChannelChatStatePaused);
}

#include "chat-widget.moc"
