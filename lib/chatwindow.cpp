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

#include "chatwindow.h"
#include "ui_chatwindow.h"
#include "adiumthemeheaderinfo.h"
#include "adiumthemecontentinfo.h"
#include "adiumthememessageinfo.h"
#include "adiumthemestatusinfo.h"


#include "channelcontactlist.h"

#include <QKeyEvent>
#include <QAction>
#include <QWidget>

#include <KColorDialog>
//#include <Sonnet/Highlighter>

#include <TelepathyQt4/Message>
#include <TelepathyQt4/Types>

class ChatWindowPrivate
{
public:
    /** Stores whether the channel is ready with all contacts upgraded*/
    bool chatviewlInitialised;
    MessageBoxEventFilter* messageBoxEventFilter;
    QAction* showFormatToolbarAction;
    bool isGroupChat;
    QString title;
};


//FIXME once TP::Factory stuff is in, remove all of ChatConnection, replace this with
//ChatWindow::ChatWindow(ConnectionPtr,TextChannelPtr, QWidget* parent) :...
ChatWindow::ChatWindow(ChatConnection* chat, QWidget *parent) :
        QWidget(parent),
        ui(new Ui::ChatWindow),
        m_chatConnection(chat),
        d(new ChatWindowPrivate)
{

    d->chatviewlInitialised = false;
    d->showFormatToolbarAction = new QAction(i18n("Show format options"), this);
    d->isGroupChat = false;

    ui->setupUi(this);
    ui->statusLabel->setText("");

    ui->formatColor->setText("");
    ui->formatColor->setIcon(KIcon("format-text-color"));

    ui->formatBold->setText("");
    ui->formatBold->setIcon(KIcon("format-text-bold"));

    ui->formatItalic->setText("");
    ui->formatItalic->setIcon(KIcon("format-text-italic"));

    ui->formatUnderline->setText("");
    ui->formatUnderline->setIcon(KIcon("format-text-underline"));

    ui->insertEmoticon->setText("");
    ui->insertEmoticon->setIcon(KIcon("face-smile"));

    updateEnabledState(false);

    //format toolbar visibility
    d->showFormatToolbarAction->setCheckable(true);
    connect(d->showFormatToolbarAction, SIGNAL(toggled(bool)), ui->formatToolbar, SLOT(setVisible(bool)));
    ui->sendMessageBox->addAction(d->showFormatToolbarAction);

    //FIXME load whether to show/hide by default from config file (do per account)
    bool formatToolbarIsVisible = false;
    ui->formatToolbar->setVisible(formatToolbarIsVisible);
    d->showFormatToolbarAction->setChecked(formatToolbarIsVisible);

    //connect signals/slots from format toolbar
    connect(ui->formatColor, SIGNAL(released()), SLOT(onFormatColorReleased()));
    connect(ui->formatBold, SIGNAL(toggled(bool)), ui->sendMessageBox, SLOT(setFontBold(bool)));
    connect(ui->formatItalic, SIGNAL(toggled(bool)), ui->sendMessageBox, SLOT(setFontItalic(bool)));
    connect(ui->formatUnderline, SIGNAL(toggled(bool)), ui->sendMessageBox, SLOT(setFontUnderline(bool)));

    //chat connection lifespan should be same as the chatwindow
    m_chatConnection->setParent(this);

    connect(m_chatConnection, SIGNAL(channelReadyStateChanged(bool)), SLOT(updateEnabledState(bool)));
    connect(m_chatConnection->channel().data(), SIGNAL(messageReceived(Tp::ReceivedMessage)), SLOT(handleIncomingMessage(Tp::ReceivedMessage)));
    connect(m_chatConnection->channel().data(), SIGNAL(messageSent(Tp::Message, Tp::MessageSendingFlags, QString)), SLOT(handleMessageSent(Tp::Message, Tp::MessageSendingFlags, QString)));
    connect(m_chatConnection->channel().data(), SIGNAL(chatStateChanged(Tp::ContactPtr, ChannelChatState)), SLOT(onChatStatusChanged(Tp::ContactPtr, ChannelChatState)));
    connect(ui->sendMessageButton, SIGNAL(released()), SLOT(sendMessage()));
    connect(ui->chatArea, SIGNAL(loadFinished(bool)), SLOT(chatViewReady()));

    connect(ui->sendMessageBox, SIGNAL(textChanged()), SLOT(onInputBoxChanged()));
    d->messageBoxEventFilter = new MessageBoxEventFilter(this);
    ui->sendMessageBox->installEventFilter(d->messageBoxEventFilter);
    connect(d->messageBoxEventFilter, SIGNAL(returnKeyPressed()), SLOT(sendMessage()));
}


ChatWindow::~ChatWindow()
{
    delete ui;
    delete d;
}

void ChatWindow::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}


QString ChatWindow::title()
{
    return d->title;
}


void ChatWindow::handleIncomingMessage(const Tp::ReceivedMessage &message)
{
    if (d->chatviewlInitialised) {
        AdiumThemeContentInfo messageInfo(AdiumThemeMessageInfo::RemoteToLocal);

        //debug the message parts (looking for HTML etc)
//        foreach(Tp::MessagePart part, message.parts())
//        {
//            qDebug() << "***";
//            foreach(QString key, part.keys())
//            {
//                qDebug() << key << part.value(key).variant();
//            }
//        }
//      turns out we have no HTML, because no CM supports it yet

        messageInfo.setMessage(message.text());
        messageInfo.setTime(message.received());
        messageInfo.setUserIconPath(message.sender()->avatarData().fileName);
        messageInfo.setSenderDisplayName(message.sender()->alias());
        messageInfo.setSenderScreenName(message.sender()->id());

        ui->chatArea->addContentMessage(messageInfo);
        m_chatConnection->channel()->acknowledge(QList<Tp::ReceivedMessage>() << message);

        emit messageReceived();
    }

    //if the window isn't ready, we don't acknowledge the mesage. We process them as soon as we are ready.
}

void ChatWindow::handleMessageSent(const Tp::Message &message, Tp::MessageSendingFlags, const QString&) /*Not sure what these other args are for*/
{
    AdiumThemeContentInfo messageInfo(AdiumThemeMessageInfo::LocalToRemote);
    messageInfo.setMessage(message.text());
    messageInfo.setTime(message.sent());

    Tp::ContactPtr sender = m_chatConnection->connection()->selfContact();
    messageInfo.setSenderDisplayName(sender->alias());
    messageInfo.setSenderScreenName(sender->id());
    messageInfo.setUserIconPath(sender->avatarData().fileName);
    ui->chatArea->addContentMessage(messageInfo);
}

void ChatWindow::chatViewReady()
{
    d->chatviewlInitialised = true;

    //process any messages we've 'missed' whilst initialising.
    foreach(Tp::ReceivedMessage message, m_chatConnection->channel()->messageQueue()) {
        handleIncomingMessage(message);
    }
}


void ChatWindow::sendMessage()
{
    if (!ui->sendMessageBox->toPlainText().isEmpty()) {
        m_chatConnection->channel()->send(ui->sendMessageBox->toPlainText());
        ui->sendMessageBox->clear();
    }
}

void ChatWindow::onChatStatusChanged(Tp::ContactPtr contact, ChannelChatState state)
{
    //don't show our own status changes.
    if (contact == m_chatConnection->connection()->selfContact())
    {
        return;
    }

    bool contactIsTyping = false;

    switch (state) {
    case ChannelChatStateGone: {
        AdiumThemeStatusInfo statusMessage;
        statusMessage.setMessage(i18n("%1 has left the chat").arg(contact->alias()));
        statusMessage.setService(m_chatConnection->connection()->protocolName());
        statusMessage.setStatus("away");
        statusMessage.setTime(QDateTime::currentDateTime());
        ui->chatArea->addStatusMessage(statusMessage);
    }
    break;
    case ChannelChatStateInactive:
        //FIXME send a 'chat timed out' message to chatview
        break;
    case ChannelChatStateActive:     
    case ChannelChatStatePaused:
        break;
    case ChannelChatStateComposing:
        contactIsTyping = true;
    }


    if (!contactIsTyping)
    {
        //In a multiperson chat just because this user is no longer typing it doesn't mean that no-one is.
        //loop through each contact, check no-one is in composing mode.
        foreach (Tp::ContactPtr contact, m_chatConnection->channel()->groupContacts())
        {
            if (contact == m_chatConnection->connection()->selfContact())
            {
                continue;
            }

            if (m_chatConnection->channel()->chatState(contact) == ChannelChatStateComposing)
            {
                contactIsTyping = true;
            }
        }
    }

    emit userTypingChanged(contactIsTyping);
}



void ChatWindow::onContactPresenceChange(Tp::ContactPtr contact, uint type)
{
    QString message;
    bool isYou = (contact.data() == m_chatConnection->channel()->groupSelfContact().data());

    switch (type) {
    case Tp::ConnectionPresenceTypeOffline:
        if (! isYou) {
            message = i18n("%1 is offline").arg(contact->alias());
        } else {
            message = i18n("You are now marked as offline");
        }
        break;
    case Tp::ConnectionPresenceTypeAvailable:
        if (! isYou) {
            message = i18n("%1 is online").arg(contact->alias());
        } else {
            message = i18n("You are now marked as online");
        }
        break;
    case Tp::ConnectionPresenceTypeAway:
        if (! isYou) {
            message = i18n("%1 is busy").arg(contact->alias());
        } else {
            message = i18n("You are now marked as busy");
        }
        break;
    default:
        /*Do nothing*/
        ;
    }

    if (!message.isNull()) {
        AdiumThemeStatusInfo statusMessage;
        statusMessage.setMessage(message);
        statusMessage.setStatus("");
        statusMessage.setService(m_chatConnection->connection()->protocolName());
        statusMessage.setTime(QDateTime::currentDateTime());
        ui->chatArea->addStatusMessage(statusMessage);
    }


    //if in a non-group chat situation, and the other contact has changed state...
    if (! d->isGroupChat && ! isYou)
    {
        KIcon icon = iconForPresence(type);
        Q_EMIT iconChanged(icon);
    }
}

void ChatWindow::updateEnabledState(bool enable)
{
    //update GUI
    ui->sendMessageBox->setEnabled(enable);
    ui->sendMessageButton->setEnabled(enable);

    //set up the initial chat window details.
    if (enable) {
        //channel is now valid, start keeping track of contacts.
        ChannelContactList* contactList = new ChannelContactList(m_chatConnection->channel(), this);
        connect(contactList, SIGNAL(contactPresenceChanged(Tp::ContactPtr, uint)), SLOT(onContactPresenceChange(Tp::ContactPtr, uint)));

        AdiumThemeHeaderInfo info;
        Tp::Contacts allContacts = m_chatConnection->channel()->groupContacts();
        //normal chat - self and one other person.
        if (allContacts.size() == 2) {
            //find the other contact which isn't self.
            foreach(Tp::ContactPtr it, allContacts) {
                if (it.data() == m_chatConnection->channel()->groupSelfContact().data()) {
                    continue;
                } else {
                    info.setDestinationDisplayName(it->alias());
                    info.setDestinationName(it->id());
                    info.setChatName(it->alias());
                    info.setIncomingIconPath(it->avatarData().fileName);
                }
            }
        } else {
            //some sort of group chat scenario.. Not sure how to create this yet.
            info.setChatName("Group Chat");
            d->isGroupChat = true;
        }

        info.setSourceName(m_chatConnection->connection()->protocolName());

        //set up anything related to 'self'
        info.setOutgoingIconPath(m_chatConnection->channel()->groupSelfContact()->avatarData().fileName);
        info.setTimeOpened(QDateTime::currentDateTime());
        ui->chatArea->initialise(info);

        //inform anyone using the class of the new name for this chat.
        d->title = info.chatName();
        Q_EMIT titleChanged(d->title);
        //FIXME emit the correct icon here too
    }
}


void ChatWindow::onInputBoxChanged()
{
    //if the box is empty
    bool currentlyTyping = ! ui->sendMessageBox->toPlainText().isEmpty();

    //FIXME buffer what we've sent to telepathy, make this more efficient.
    //FIXME check spec (with olly) as to whether we have to handle idle state etc.
    if(currentlyTyping)
    {
        m_chatConnection->channel()->requestChatState(Tp::ChannelChatStateComposing);
    }
    else
    {
        m_chatConnection->channel()->requestChatState(Tp::ChannelChatStateActive);
    }
}

bool MessageBoxEventFilter::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {
            if (!keyEvent->modifiers()) {
                Q_EMIT returnKeyPressed();
                return true;
            }
        }
    }
    // standard event processing
    return QObject::eventFilter(obj, event);
}


void ChatWindow::onFormatColorReleased()
{
    QColor color;
    KColorDialog::getColor(color,this);
    ui->sendMessageBox->setTextColor(color);
}

KIcon ChatWindow::iconForPresence(uint presence)
{
    QString iconName;

    switch (presence) {
        case Tp::ConnectionPresenceTypeAvailable:
            iconName = QLatin1String("user-online");
            break;
        case Tp::ConnectionPresenceTypeAway:
            iconName = QLatin1String("user-away");
            break;
        case Tp::ConnectionPresenceTypeExtendedAway:
            iconName = QLatin1String("user-away-extended");
            break;
        case Tp::ConnectionPresenceTypeHidden:
            iconName = QLatin1String("user-invisible");
            break;
        case Tp::ConnectionPresenceTypeBusy:
            iconName = QLatin1String("user-busy");
            break;
        default:
            iconName = QLatin1String("user-offline");
            break;
    }

    return KIcon(iconName);
}

