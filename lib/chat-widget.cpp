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
#include "notify-filter.h"
#include "text-chat-config.h"
#include "contact-delegate.h"

#include <QtGui/QKeyEvent>
#include <QtGui/QAction>
#include <QtGui/QMenu>
#include <QSortFilterProxyModel>

#include <KColorDialog>
#include <KNotification>
#include <KAboutData>
#include <KComponentData>
#include <KDebug>
#include <KColorScheme>
#include <KLineEdit>
#include <KMimeType>
#include <KTemporaryFile>
#include <KFileDialog>

#include <TelepathyQt/Account>
#include <TelepathyQt/Message>
#include <TelepathyQt/Types>
#include <TelepathyQt/AvatarData>
#include <TelepathyQt/Connection>
#include <TelepathyQt/Presence>
#include <TelepathyQt/PendingChannelRequest>
#include <TelepathyQt/OutgoingFileTransferChannel>

#include <KTp/presence.h>
#include <KTp/actions.h>
#include <KTp/message-processor.h>
#include <KTp/Logger/scrollback-manager.h>
#include <KTp/contact-info-dialog.h>

#include <sonnet/speller.h>

Q_DECLARE_METATYPE(Tp::ContactPtr)

const QString groupChatOnlineIcon(QLatin1String("im-irc"));
// FIXME We should have a proper icon for this
const QString groupChatOfflineIcon(QLatin1String("im-irc"));

class ChatWidgetPrivate
{
public:
    ChatWidgetPrivate() :
        remoteContactChatState(Tp::ChannelChatStateInactive),
        isGroupChat(false),
        contactsMenu(0),
        fileResourceTransferMenu(0),
        fileTransferMenuAction(0),
        shareImageMenuAction(0),
        logsLoaded(false),
        exchangedMessagesCount(0)
    {
    }
    /** Stores whether the channel is ready with all contacts upgraded*/
    bool chatViewInitialized;
    Tp::ChannelChatState remoteContactChatState;
    bool isGroupChat;
    QString title;
    QString contactName;
    QString yourName;
    QString currentKeyboardLayoutLanguage;
    Tp::TextChannelPtr channel;
    Tp::AccountPtr account;
    ShareProvider *shareProvider;
    Ui::ChatWidget ui;
    ChannelContactModel *contactModel;
    QMenu *contactsMenu;
    QMenu *fileResourceTransferMenu;
    // Used with imageShareMenu
    QAction *fileTransferMenuAction;
    QAction *shareImageMenuAction;
    QString fileToTransferPath;
    ScrollbackManager *logManager;
    QTimer *pausedStateTimer;
    bool logsLoaded;
    uint exchangedMessagesCount;

    QList< Tp::OutgoingFileTransferChannelPtr > tmpFileTransfers;

    KComponentData telepathyComponentData();
    KTp::AbstractMessageFilter *notifyFilter;
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
    d->logManager = new ScrollbackManager(this);
    connect(d->logManager, SIGNAL(fetched(QList<KTp::Message>)), SLOT(onHistoryFetched(QList<KTp::Message>)));

    connect(d->account.data(), SIGNAL(currentPresenceChanged(Tp::Presence)),
            this, SLOT(currentPresenceChanged(Tp::Presence)));

    ShareProvider::ShareService serviceType = static_cast<ShareProvider::ShareService>(TextChatConfig::instance()->imageShareServiceType());
    d->shareProvider = new ShareProvider(serviceType, this);
    connect(d->shareProvider, SIGNAL(finishedSuccess(ShareProvider*,QString)), this, SLOT(onShareProviderFinishedSuccess(ShareProvider*,QString)));
    connect(d->shareProvider, SIGNAL(finishedError(ShareProvider*,QString)), this, SLOT(onShareProviderFinishedFailure(ShareProvider*,QString)));

    //load translations for this library. keep this before any i18n() calls in library code
    KGlobal::locale()->insertCatalog(QLatin1String("ktpchat"));

    d->chatViewInitialized = false;
    d->isGroupChat = (channel->targetHandleType() == Tp::HandleTypeContact ? false : true);

    d->ui.setupUi(this);
    if (d->isGroupChat) {
        d->contactsMenu = new QMenu(this);
        QAction *action = d->contactsMenu->addAction(KIcon::fromTheme(QLatin1String("text-x-generic")),
                                   i18n("Open chat window"),
                                   this, SLOT(onOpenContactChatWindowClicked()));
        action->setObjectName(QLatin1String("OpenChatWindowAction"));
        action = d->contactsMenu->addAction(KIcon::fromTheme(QLatin1String("mail-attachment")),
                                            i18n("Send file"),
                                            this, SLOT(onSendFileClicked()));
        action->setObjectName(QLatin1String("SendFileAction"));
        d->contactsMenu->addSeparator();
        d->contactsMenu->addAction(i18n("Show info..."),
                                   this, SLOT(onShowContactDetailsClicked()));

        d->ui.contactsView->setContextMenuPolicy(Qt::CustomContextMenu);
        d->ui.contactsView->setItemDelegate(new ContactDelegate(this));

        connect(d->ui.contactsView, SIGNAL(customContextMenuRequested(QPoint)),
                this, SLOT(onContactsViewContextMenuRequested(QPoint)));
    }

    KTp::ContactPtr targetContact = KTp::ContactPtr::qObjectCast(d->channel->targetContact());

    d->fileResourceTransferMenu = new QMenu(this);
    // This action's text is going to be changed in the dropEvent method to add the destination image service.
    d->shareImageMenuAction = new QAction(KIcon::fromTheme(QLatin1String("insert-image")), i18n("Share Image"), this);
    connect(d->shareImageMenuAction, SIGNAL(triggered(bool)), this, SLOT(onShareImageMenuActionTriggered()));
    d->fileTransferMenuAction = new QAction(KIcon::fromTheme(QLatin1String("mail-attachment")), i18n("Send File"), this);

    d->fileTransferMenuAction->setEnabled(targetContact->fileTransferCapability());
    d->fileResourceTransferMenu->addAction(d->fileTransferMenuAction);
    connect(d->fileTransferMenuAction, SIGNAL(triggered(bool)), this, SLOT(onFileTransferMenuActionTriggered()));

    // connect channel signals
    setupChannelSignals();

    // create contactModel and start keeping track of contacts.
    d->contactModel = new ChannelContactModel(d->channel, this);
    setupContactModelSignals();

    /* Enable nick completion only in group chats */
    if (d->isGroupChat) {
        d->ui.sendMessageBox->setContactModel(d->contactModel);
    }

    QSortFilterProxyModel *sortModel = new QSortFilterProxyModel(this);
    sortModel->setSourceModel(d->contactModel);
    sortModel->setSortRole(Qt::DisplayRole);
    sortModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    sortModel->setSortLocaleAware(true);
    sortModel->setDynamicSortFilter(true);
    sortModel->sort(0);

    d->ui.contactsView->setModel(sortModel);

    d->yourName = channel->groupSelfContact()->alias();

    d->ui.sendMessageBox->setAcceptDrops(false);
    d->ui.chatArea->setAcceptDrops(false);
    setAcceptDrops(true);

    /* Prepare the chat area */
    connect(d->ui.chatArea, SIGNAL(zoomFactorChanged(qreal)), SIGNAL(zoomFactorChanged(qreal)));
    connect(d->ui.chatArea, SIGNAL(textPasted()), d->ui.sendMessageBox, SLOT(pasteSelection()));
    initChatArea();

    d->pausedStateTimer = new QTimer(this);
    d->pausedStateTimer->setSingleShot(true);

    // Spellchecking set up will trigger textChanged() signal of d->ui.sendMessageBox
    // and our handler checks state of the timer created above.
    loadSpellCheckingOption();

    // make clicking in the main HTML area put focus in the input text box
    d->ui.chatArea->setFocusProxy(d->ui.sendMessageBox);
    //make activating the tab select the text area
    setFocusProxy(d->ui.sendMessageBox);

    connect(d->ui.sendMessageBox, SIGNAL(returnKeyPressed()), SLOT(sendMessage()));

    connect(d->ui.searchBar, SIGNAL(findTextSignal(QString,QWebPage::FindFlags)), this, SLOT(findTextInChat(QString,QWebPage::FindFlags)));
    connect(d->ui.searchBar, SIGNAL(findNextSignal(QString,QWebPage::FindFlags)), this, SLOT(findNextTextInChat(QString,QWebPage::FindFlags)));
    connect(d->ui.searchBar, SIGNAL(findPreviousSignal(QString,QWebPage::FindFlags)), this, SLOT(findPreviousTextInChat(QString,QWebPage::FindFlags)));
    connect(d->ui.searchBar, SIGNAL(flagsChangedSignal(QString,QWebPage::FindFlags)), this, SLOT(findTextInChat(QString,QWebPage::FindFlags)));

    connect(this, SIGNAL(searchTextComplete(bool)), d->ui.searchBar, SLOT(onSearchTextComplete(bool)));

    connect(d->pausedStateTimer, SIGNAL(timeout()), this, SLOT(onChatPausedTimerExpired()));

    // initialize LogManager
    KConfig config(QLatin1String("ktelepathyrc"));
    KConfigGroup tabConfig = config.group("Behavior");
    d->logManager->setScrollbackLength(tabConfig.readEntry<int>("scrollbackLength", 4));
    d->logManager->setTextChannel(d->account, d->channel);
    m_previousConversationAvailable = d->logManager->exists();

    d->notifyFilter = new NotifyFilter(this);
}

ChatWidget::~ChatWidget()
{
    saveSpellCheckingOption();
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
    if (!d->isGroupChat) {
        if (d->account->currentPresence() != Tp::Presence::offline()) {
            //normal chat - self and one other person.
            //find the other contact which isn't self.
            Tp::ContactPtr otherContact = d->channel->targetContact();
            KIcon presenceIcon = KTp::Presence(otherContact->presence()).icon();

            if (otherContact->clientTypes().contains(QLatin1String("phone"))) {
                //we paint a warning symbol in the right-bottom corner
                QPixmap phonePixmap = KIconLoader::global()->loadIcon(QLatin1String("phone"), KIconLoader::NoGroup, 16);
                QPixmap pixmap = presenceIcon.pixmap(32, 32);
                QPainter painter(&pixmap);
                painter.drawPixmap(8, 8, 24, 24, phonePixmap);
                return KIcon(pixmap);
            }
            return presenceIcon;
        } else {
            return KTp::Presence(Tp::Presence::offline()).icon();
        }
    } else {
        //group chat
        if (d->account->currentPresence() != Tp::Presence::offline()) {
            return KIcon(groupChatOnlineIcon);
        } else {
            return KIcon(groupChatOfflineIcon);
        }
    }
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
    d->ui.contactsView->setEnabled(enable);
    d->ui.sendMessageBox->setEnabled(enable);
    Q_EMIT iconChanged(icon());
}

void ChatWidget::setTextChannel(const Tp::TextChannelPtr &newTextChannelPtr)
{

    d->channel = newTextChannelPtr;     // set the new channel
    d->contactModel->setTextChannel(newTextChannelPtr);

    // connect signals for the new textchannel
    setupChannelSignals();

    //if the UI is ready process any messages in queue
    if (d->chatViewInitialized) {
        Q_FOREACH (const Tp::ReceivedMessage &message, d->channel->messageQueue()) {
            handleIncomingMessage(message, true);
        }
    }
    setChatEnabled(true);
    onContactPresenceChange(d->channel->groupSelfContact(), KTp::Presence(d->channel->groupSelfContact()->presence()));
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

    if (e->key() == Qt::Key_PageUp ||
        e->key() == Qt::Key_PageDown) {
        d->ui.chatArea->event(e);
        return;
    }

    QWidget::keyPressEvent(e);
}

void ChatWidget::temporaryFileTransferStateChanged(Tp::FileTransferState state, Tp::FileTransferStateChangeReason reason)
{
    Q_UNUSED(reason);

    if ((state == Tp::FileTransferStateCompleted) || (state == Tp::FileTransferStateCancelled)) {
        Tp::OutgoingFileTransferChannel *channel = qobject_cast<Tp::OutgoingFileTransferChannel*>(sender());
        Q_ASSERT(channel);

        QString localFile = QUrl(channel->uri()).toLocalFile();
        if (QFile::exists(localFile)) {
            QFile::remove(localFile);
            kDebug() << "File" << localFile << "removed";
        }

        d->tmpFileTransfers.removeAll(Tp::OutgoingFileTransferChannelPtr(channel));
    }
}


void ChatWidget::temporaryFileTransferChannelCreated(Tp::PendingOperation *operation)
{
    Tp::PendingChannelRequest *request = qobject_cast<Tp::PendingChannelRequest*>(operation);
    Q_ASSERT(request);

    Tp::OutgoingFileTransferChannelPtr transferChannel;
    transferChannel = Tp::OutgoingFileTransferChannelPtr::qObjectCast<Tp::Channel>(request->channelRequest()->channel());
    Q_ASSERT(!transferChannel.isNull());

    /* Make sure the pointer lives until the transfer is over
     * so that the signal connection below lasts until the end */
    d->tmpFileTransfers << transferChannel;

    connect(transferChannel.data(), SIGNAL(stateChanged(Tp::FileTransferState,Tp::FileTransferStateChangeReason)),
            this, SLOT(temporaryFileTransferStateChanged(Tp::FileTransferState,Tp::FileTransferStateChangeReason)));
}


void ChatWidget::dropEvent(QDropEvent *e)
{
    const QMimeData *data = e->mimeData();
    ShareProvider::ShareService shareServiceType = TextChatConfig::instance()->imageShareServiceType();
    d->shareProvider->setShareServiceType(shareServiceType);

    d->shareImageMenuAction->setText(i18n("Share Image via %1", ShareProvider::availableShareServices().key(shareServiceType)));
    d->fileResourceTransferMenu->clear();

    if (data->hasUrls()) {
        Q_FOREACH(const QUrl &url, data->urls()) {
            if (url.isLocalFile()) {
		 // Not sure if this the best way to determine the MIME type of the file
		 KMimeType::Ptr ptr = KMimeType::findByUrl(url);
		 QString mime       = ptr->name();
		 if (mime.startsWith(QLatin1String("image/"))) {
		    d->fileTransferMenuAction->setText(i18n("Send Image via File Transfer"));
		    d->fileResourceTransferMenu->addAction(d->shareImageMenuAction);
		    d->fileResourceTransferMenu->addAction(d->fileTransferMenuAction);
		 } else {
		   QFileInfo fileInfo(url.toLocalFile());
		   d->fileTransferMenuAction->setText(i18n("Send File"));
		   d->fileResourceTransferMenu->addAction(d->fileTransferMenuAction);
		 }
		 d->fileToTransferPath = url.toLocalFile();
		 d->fileResourceTransferMenu->popup(mapToGlobal(e->pos()));
            } else {
                d->ui.sendMessageBox->append(url.toString());
            }
        }
        e->acceptProposedAction();
    } else if (data->hasText()) {
        d->ui.sendMessageBox->append(data->text());
        e->acceptProposedAction();
    } else if (data->hasHtml()) {
        d->ui.sendMessageBox->insertHtml(data->html());
        e->acceptProposedAction();
    } else if (data->hasImage()) {
        QImage image = qvariant_cast<QImage>(data->imageData());

        KTemporaryFile tmpFile;
        tmpFile.setPrefix(d->account->displayName() + QLatin1String("-"));
        tmpFile.setSuffix(QLatin1String(".png"));
        tmpFile.setAutoRemove(false);
        if (!tmpFile.open()) {
            return;
        }
        tmpFile.close();

        if (!image.save(tmpFile.fileName(), "PNG")) {
            return;
        }

	d->fileToTransferPath = tmpFile.fileName();
	d->fileResourceTransferMenu->popup(mapToGlobal(e->pos()));

        kDebug() << "Starting Uploading of" << tmpFile.fileName();
        e->acceptProposedAction();
    }

    QWidget::dropEvent(e);
}

void ChatWidget::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasHtml() || e->mimeData()->hasImage() ||
        e->mimeData()->hasText() || e->mimeData()->hasUrls()) {
            e->accept();
    }

    QWidget::dragEnterEvent(e);
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

    if (TextChatConfig::instance()->showOthersTyping() && (d->remoteContactChatState == Tp::ChannelChatStateComposing)) {
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
    connect(d->channel.data(), SIGNAL(pendingMessageRemoved(Tp::ReceivedMessage)),
            SIGNAL(unreadMessagesChanged()));
    connect(d->channel.data(), SIGNAL(messageSent(Tp::Message,Tp::MessageSendingFlags,QString)),
            SLOT(handleMessageSent(Tp::Message,Tp::MessageSendingFlags,QString)));
    connect(d->channel.data(), SIGNAL(chatStateChanged(Tp::ContactPtr,Tp::ChannelChatState)),
            SLOT(onChatStatusChanged(Tp::ContactPtr,Tp::ChannelChatState)));
    connect(d->channel.data(), SIGNAL(invalidated(Tp::DBusProxy*,QString,QString)),
            this, SLOT(onChannelInvalidated()));
    connect(d->channel.data(), SIGNAL(groupMembersChanged(Tp::Contacts,
                                                          Tp::Contacts,
                                                          Tp::Contacts,
                                                          Tp::Contacts,
                                                          Tp::Channel::GroupMemberChangeDetails)),
            this, SLOT(onParticipantsChanged(Tp::Contacts,
                                             Tp::Contacts,
                                             Tp::Contacts,
                                             Tp::Contacts,
                                             Tp::Channel::GroupMemberChangeDetails)));

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
    connect(d->contactModel,SIGNAL(contactClientTypesChanged(Tp::ContactPtr,QStringList)),
            SLOT(onContactClientTypesChanged(Tp::ContactPtr,QStringList)));
}

void ChatWidget::onHistoryFetched(const QList<KTp::Message> &messages)
{
    d->chatViewInitialized = true;

    kDebug() << "found" << messages.count() << "messages in history";
    if (!messages.isEmpty()) {
        QDate date = messages.first().time().date();
        Q_FOREACH(const KTp::Message &message, messages) {
            if (message.time().date() != date) {
                date = message.time().date();
                d->ui.chatArea->addStatusMessage(date.toString(Qt::LocaleDate));
            }

            d->ui.chatArea->addMessage(message);
        }

        if (date != QDate::currentDate()) {
            d->ui.chatArea->addStatusMessage(QDate::currentDate().toString(Qt::LocaleDate));
        }
    }

    //process any messages we've 'missed' whilst initialising.
    Q_FOREACH(const Tp::ReceivedMessage &message, d->channel->messageQueue()) {
        handleIncomingMessage(message, true);
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
    if (d->chatViewInitialized) {
        //acknowledge everything in the message queue.
        d->channel->acknowledge(d->channel->messageQueue());
    }
}

void ChatWidget::updateSendMessageShortcuts(const KShortcut &shortcuts)
{
    d->ui.sendMessageBox->setSendMessageShortcuts(shortcuts);
}

bool ChatWidget::isOnTop() const
{
    kDebug() << ( isActiveWindow() && isVisible() );
    return ( isActiveWindow() && isVisible() );
}

void ChatWidget::handleIncomingMessage(const Tp::ReceivedMessage &message, bool alreadyNotified)
{
    kDebug() << title() << message.text();

    if (d->chatViewInitialized) {

        d->exchangedMessagesCount++;

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
            Tp::ReceivedMessage::DeliveryDetails reportDetails = message.deliveryDetails();

            if (reportDetails.hasDebugMessage()) {
                kDebug() << "delivery report debug message: " << reportDetails.debugMessage();
            }

            if (reportDetails.isError()) {
                switch (reportDetails.error()) {
                case Tp::ChannelTextSendErrorOffline:
                    if (reportDetails.hasEchoedMessage()) {
                        if(message.sender() && message.sender()->isBlocked()) {
                            text = i18n("Delivery of the message \"%1\" "
                                        "failed because the remote contact is blocked",
                                        reportDetails.echoedMessage().text());
                         } else {
                            text = i18n("Delivery of the message \"%1\" "
                                        "failed because the remote contact is offline",
                                        reportDetails.echoedMessage().text());
                         }
                    } else {
                        if(message.sender() && message.sender()->isBlocked()) {
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

            d->ui.chatArea->addStatusMessage(text, message.sender()->alias(), message.received());
        } else {
            KTp::Message processedMessage(KTp::MessageProcessor::instance()->processIncomingMessage(message, d->account, d->channel));

            if (!alreadyNotified) {
                d->notifyFilter->filterMessage(processedMessage,
                                               KTp::MessageContext(d->account, d->channel));
            }
            d->ui.chatArea->addMessage(processedMessage);
        }

        //if the window is on top, ack straight away. Otherwise they stay in the message queue for acking when activated..
        if (isOnTop()) {
            d->channel->acknowledge(QList<Tp::ReceivedMessage>() << message);
        } else {
            Q_EMIT unreadMessagesChanged();
        }
    }

}

void ChatWidget::handleMessageSent(const Tp::Message &message, Tp::MessageSendingFlags, const QString&)
{
    KTp::Message processedMessage(KTp::MessageProcessor::instance()->processIncomingMessage(message, d->account, d->channel));
    d->notifyFilter->filterMessage(processedMessage,
                                   KTp::MessageContext(d->account, d->channel));
    d->ui.chatArea->addMessage(processedMessage);
    d->exchangedMessagesCount++;
}

void ChatWidget::chatViewReady()
{
    disconnect(d->ui.chatArea, SIGNAL(loadFinished(bool)), this, SLOT(chatViewReady()));

    if (!d->logsLoaded || d->exchangedMessagesCount > 0) {
        if (d->exchangedMessagesCount == 0) {
            d->logManager->fetchScrollback();
        } else {
            d->logManager->fetchHistory(d->exchangedMessagesCount + d->logManager->scrollbackLength());
        }
    }

    d->logsLoaded = true;
}


void ChatWidget::sendMessage()
{
    QString message = d->ui.sendMessageBox->toPlainText();

    if (!message.isEmpty()) {
        message = KTp::MessageProcessor::instance()->processOutgoingMessage(
                    message, d->account, d->channel).text();

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
        if (d->ui.chatArea->showJoinLeaveChanges()) {
	    d->ui.chatArea->addStatusMessage(i18n("%1 has left the chat", contact->alias()), contact->alias());
	}
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
        if (presence.statusMessage().isEmpty()) {
            message = i18nc("Your presence status", "You are now marked as %1",
                            presence.displayString());
        } else {
            message = i18nc("Your presence status with status message",
                            "You are now marked as %1 - %2",
                            presence.displayString(), presence.statusMessage());
        }
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
        if (d->ui.chatArea->showPresenceChanges()) {
            d->ui.chatArea->addStatusMessage(message, contact->alias());
        }
    }

    //if in a non-group chat situation, and the other contact has changed state...
    if (!d->isGroupChat && !isYou) {
        Q_EMIT iconChanged(icon());
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
        d->ui.chatArea->addStatusMessage(i18n("%1 has left the chat", contact->alias()), contact->alias());
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

    d->ui.chatArea->addStatusMessage(message);

    Q_EMIT contactBlockStatusChanged(blocked);
}

void ChatWidget::onContactClientTypesChanged(const Tp::ContactPtr &contact, const QStringList &clientTypes)
{
    Q_UNUSED(clientTypes)
    bool isYou = (contact == d->channel->groupSelfContact());

    if (!d->isGroupChat && !isYou) {
        Q_EMIT iconChanged(icon());
    }
}

void ChatWidget::onParticipantsChanged(Tp::Contacts groupMembersAdded,
                                       Tp::Contacts groupLocalPendingMembersAdded,
                                       Tp::Contacts groupRemotePendingMembersAdded,
                                       Tp::Contacts groupMembersRemoved,
                                       Tp::Channel::GroupMemberChangeDetails details) {
    Q_UNUSED(groupLocalPendingMembersAdded);
    Q_UNUSED(groupRemotePendingMembersAdded);
    Q_UNUSED(groupMembersRemoved);
    Q_UNUSED(details);

    if (groupMembersAdded.count() > 0 && (d->ui.chatArea->showJoinLeaveChanges())) {
        d->ui.chatArea->addStatusMessage(i18n("%1 has joined the chat", groupMembersAdded.toList().value(0).data()->alias()), groupMembersAdded.toList().value(0).data()->alias());
    }
    // Temporarily detect on-demand rooms by checking for gabble-created string "private-chat"
    if (d->isGroupChat && d->channel->targetId().startsWith(QLatin1String("private-chat"))) {
        QList<QString> contactAliasList;
        if (d->channel->groupContacts().count() > 0) {
            Q_FOREACH (const Tp::ContactPtr &contact, d->channel->groupContacts()) {
                contactAliasList.append(contact->alias());
            }
            contactAliasList.removeAll(d->channel->groupSelfContact()->alias());
            qSort(contactAliasList);

            int aliasesToShow = qMin(contactAliasList.length(), 2);
            QString newTitle;

            //This filters each contact alias and tries to make a best guess at intelligently
            //shortening their alias to ensure the tab isn't too long, (hard-limited to 10) e.g.:
            //Robert@kdetalk.net is filtered at the @, giving Robert, and
            //Fred Jones is filtered by the ' ', giving Fred.
            Q_FOREACH (const QString &contactAlias, contactAliasList) {
                aliasesToShow--;
                if (contactAlias.indexOf(QLatin1Char(' ')) != -1) {
                    newTitle += contactAlias.left(contactAlias.indexOf(QLatin1Char(' '))).left(10);
                } else if (contactAlias.indexOf(QLatin1Char('@')) != -1) {
                    newTitle += contactAlias.left(contactAlias.indexOf(QLatin1Char('@'))).left(10);
                } else {
                    newTitle += contactAlias.left(10);
                }
                if (aliasesToShow > 0) {
                    newTitle += QLatin1String(", ");
                } else {
                    break;
                }
            }
            if (contactAliasList.count() > 2) {
                newTitle.append(QLatin1String(" +")).append(QString::number(contactAliasList.size()-2));
            }

            Q_EMIT titleChanged(newTitle);
        }
        if (contactAliasList.count() == 0) {
                Q_EMIT titleChanged(i18n("Group Chat"));
        }
    }
}

void ChatWidget::onChannelInvalidated()
{
    setChatEnabled(false);
}

void ChatWidget::onInputBoxChanged()
{
    //if the box is empty
    bool textBoxEmpty = d->ui.sendMessageBox->toPlainText().isEmpty();

    if(!textBoxEmpty) {
        //if the timer is active, it means the user is continuously typing
        if (d->pausedStateTimer->isActive()) {
            //just restart the timer and don't spam with chat state changes
            d->pausedStateTimer->start(5000);
        } else {
            //if the user has just typed some text, set state to Composing and start the timer
            //unless "show me typing" is off; in that case set state to Active and stop the timer
            if (TextChatConfig::instance()->showMeTyping()) {
                d->channel->requestChatState(Tp::ChannelChatStateComposing);
                d->pausedStateTimer->start(5000);
            } else {
                d->channel->requestChatState(Tp::ChannelChatStateActive);
                d->pausedStateTimer->stop();
            }
        }
    } else {
        //if the user typed no text/cleared the input field, set Active and stop the timer
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

void ChatWidget::setSpellDictionary(const QString &dict)
{
    d->ui.sendMessageBox->setSpellCheckingLanguage(dict);
}

QString ChatWidget::currentKeyboardLayoutLanguage() const
{
    return d->currentKeyboardLayoutLanguage;
}

void ChatWidget::setCurrentKeyboardLayoutLanguage(const QString &language)
{
    d->currentKeyboardLayoutLanguage = language;
}

void ChatWidget::saveSpellCheckingOption()
{
    QString spellCheckingLanguage = spellDictionary();
    KSharedConfigPtr config = KSharedConfig::openConfig(QLatin1String("ktp-text-uirc"));
    KConfigGroup configGroup = config->group(d->channel->targetId());
    if (spellCheckingLanguage != Sonnet::Speller().defaultLanguage()) {
        configGroup.writeEntry("language", spellCheckingLanguage);
    } else {
        if (configGroup.exists()) {
            configGroup.deleteEntry("language");
            configGroup.deleteGroup();
        } else {
            return;
        }
    }
    configGroup.sync();
}

void ChatWidget::loadSpellCheckingOption()
{
    // KTextEdit::setSpellCheckingLanguage() (see call below) does not do anything if there
    // is no highlighter created yet, and KTextEdit::setCheckSpellingEnabled() does not create
    // it if there is no focus on widget.
    // Therefore d->ui.sendMessageBox->setSpellCheckingLanguage() call below would be is ignored.
    // While this looks like KTextEditor bug (espesially taking into account its documentation),
    // just a call to KTextEditor::createHighlighter() before setting language fixes this
    d->ui.sendMessageBox->createHighlighter();

    KSharedConfigPtr config = KSharedConfig::openConfig(QLatin1String("ktp-text-uirc"));
    KConfigGroup configGroup = config->group(d->channel->targetId());
    QString spellCheckingLanguage;
    if (configGroup.exists()) {
        spellCheckingLanguage = configGroup.readEntry("language");
    } else {
        spellCheckingLanguage = Sonnet::Speller().defaultLanguage();
    }
    d->ui.sendMessageBox->setSpellCheckingLanguage(spellCheckingLanguage);
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

void ChatWidget::clear()
{
    // Don't reload logs when re-initializing */
    d->logsLoaded = true;
    d->exchangedMessagesCount = 0;
    d->ui.sendMessageBox->clearHistory();
    initChatArea();
}

void ChatWidget::setZoomFactor(qreal zoomFactor)
{
    d->ui.chatArea->setZoomFactor(zoomFactor);
}

qreal ChatWidget::zoomFactor() const
{
    return d->ui.chatArea->zoomFactor();
}

void ChatWidget::initChatArea()
{
    connect(d->ui.chatArea, SIGNAL(loadFinished(bool)), SLOT(chatViewReady()), Qt::QueuedConnection);

    d->ui.chatArea->load((d->isGroupChat ? AdiumThemeView::GroupChat : AdiumThemeView::SingleUserChat));

    AdiumThemeHeaderInfo info;

    info.setGroupChat(d->isGroupChat);
    //normal chat - self and one other person.
    if (d->isGroupChat) {
        // A temporary solution to display a roomname instead of a full jid
        // This should be reworked as soon as TpQt is offering the
        // room name property
        // Temporarily detect on-demand rooms by checking for
        // gabble-created string "private-chat"
        if (d->channel->targetId().contains(QLatin1String("private-chat"))) {
            info.setChatName(i18n("Group Chat"));
        } else {
            QString roomName = d->channel->targetId();
            roomName = roomName.left(roomName.indexOf(QLatin1Char('@')));
            info.setChatName(roomName);
        }
    } else {
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

    //set the message time
    if (!d->channel->messageQueue().isEmpty()) {
        info.setTimeOpened(d->channel->messageQueue().first().received());
    } else {
        info.setTimeOpened(QDateTime::currentDateTime());
    }

    info.setService(d->account->serviceName());
     // check iconPath docs for minus sign in -KIconLoader::SizeMedium
    info.setServiceIconPath(KIconLoader::global()->iconPath(d->account->iconName(), -KIconLoader::SizeMedium));
    d->ui.chatArea->initialise(info);

    //set the title of this chat.
    d->title = info.chatName();
}

void ChatWidget::onChatPausedTimerExpired()
{
     if (TextChatConfig::instance()->showMeTyping()) {
        d->channel->requestChatState(Tp::ChannelChatStatePaused);
    } else {
        d->channel->requestChatState(Tp::ChannelChatStateActive);
    }
}

void ChatWidget::currentPresenceChanged(const Tp::Presence &presence)
{
    if (presence == Tp::Presence::offline()) {
        d->ui.chatArea->addStatusMessage(i18n("You are now offline"), d->yourName);
        iconChanged(icon());
    }
}

void ChatWidget::addEmoticonToChat(const QString &emoticon)
{
    d->ui.sendMessageBox->insertPlainText(QLatin1String(" ") + emoticon);
    d->ui.sendMessageBox->setFocus();
}

void ChatWidget::reloadTheme()
{
    d->logsLoaded = false;
    d->chatViewInitialized = false;

    initChatArea();
}

void ChatWidget::onContactsViewContextMenuRequested(const QPoint& point)
{
    const QModelIndex index = d->ui.contactsView->indexAt(point);
    if (!index.isValid()) {
        return;
    }

    const KTp::ContactPtr contact = KTp::ContactPtr::qObjectCast<Tp::Contact>(index.data(KTp::ContactRole).value<Tp::ContactPtr>());

    bool isSelfContact = ((Tp::ContactPtr) contact == textChannel()->groupSelfContact());
    d->contactsMenu->findChild<QAction*>(QLatin1String("OpenChatWindowAction"))->setEnabled(!isSelfContact);

    d->contactsMenu->findChild<QAction*>(QLatin1String("SendFileAction"))->setEnabled(contact->fileTransferCapability());

    d->contactsMenu->setProperty("Contact", QVariant::fromValue(contact));
    d->contactsMenu->popup(d->ui.contactsView->mapToGlobal(point));
}

void ChatWidget::onFileTransferMenuActionTriggered()
{
    if (!d->fileToTransferPath.isEmpty()) {
	KTp::Actions::startFileTransfer(d->account, d->channel->targetContact(), d->fileToTransferPath);
    }
}

void ChatWidget::onShareImageMenuActionTriggered()
{
    if (!d->fileToTransferPath.isEmpty()) {
	d->shareProvider->publish(d->fileToTransferPath);
    }
}

void ChatWidget::onShowContactDetailsClicked()
{
    const KTp::ContactPtr contact = d->contactsMenu->property("Contact").value<KTp::ContactPtr>();
    Q_ASSERT(!contact.isNull());

    KTp::ContactInfoDialog *dlg = new KTp::ContactInfoDialog(d->account, contact, this);
    connect(dlg, SIGNAL(finished()), dlg, SLOT(deleteLater()));
    dlg->show();
}

void ChatWidget::onShareProviderFinishedSuccess(ShareProvider* provider, const QString& imageUrl)
{
    Q_UNUSED(provider);
    if (!imageUrl.isEmpty()) {
	d->channel->send(imageUrl);
    }
}

void ChatWidget::onShareProviderFinishedFailure(ShareProvider* provider, const QString& errorMessage)
{
    Q_UNUSED(provider);
    d->ui.chatArea->addStatusMessage(i18n("Uploading Image has Failed with Error: %1", errorMessage));
}


void ChatWidget::onOpenContactChatWindowClicked()
{
    const KTp::ContactPtr contact = d->contactsMenu->property("Contact").value<KTp::ContactPtr>();
    Q_ASSERT(!contact.isNull());
    KTp::Actions::startChat(d->account, contact);
}

void ChatWidget::onSendFileClicked()
{
    const KTp::ContactPtr contact = d->contactsMenu->property("Contact").value<KTp::ContactPtr>();
    Q_ASSERT(!contact.isNull());
    const QString filename = KFileDialog::getOpenFileName();
    if (filename.isEmpty() || !QFile::exists(filename)) {
        return;
    }

    KTp::Actions::startFileTransfer(d->account, contact, filename);
}

#include "chat-widget.moc"
