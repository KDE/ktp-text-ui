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
#include "telepathychatmessageinfo.h"
#include "telepathychatinfo.h"
#include <QKeyEvent>


ChatWindow::ChatWindow(ChatConnection* chat, QWidget *parent) :
        QWidget(parent),
        ui(new Ui::ChatWindow),
        m_chatConnection(chat),
        m_channelInitialised(false)
{
    ui->setupUi(this);    
    ui->statusLabel->setText("");

    updateEnabledState(false);

    //chat connection lifespan should be same as the chatwindow
    m_chatConnection->setParent(this);

    connect(m_chatConnection, SIGNAL(channelReadyStateChanged(bool)), SLOT(updateEnabledState(bool)));
    connect(m_chatConnection->channel().data(), SIGNAL(messageReceived(Tp::ReceivedMessage)), SLOT(handleIncomingMessage(Tp::ReceivedMessage)));
    connect(m_chatConnection->channel().data(), SIGNAL(messageSent(Tp::Message, Tp::MessageSendingFlags, QString)), SLOT(handleMessageSent(Tp::Message, Tp::MessageSendingFlags, QString)));
    connect(m_chatConnection->channel().data(), SIGNAL(chatStateChanged(Tp::ContactPtr, ChannelChatState)), SLOT(updateChatStatus(Tp::ContactPtr, ChannelChatState)));
    connect(ui->sendMessageButton, SIGNAL(released()), SLOT(sendMessage()));

    messageBoxEventFilter = new MessageBoxEventFilter(this);
    ui->sendMessageBox->installEventFilter(messageBoxEventFilter);
    connect(messageBoxEventFilter, SIGNAL(returnKeyPressed()), SLOT(sendMessage()));
}


ChatWindow::~ChatWindow()
{
    delete ui;
}

void ChatWindow::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type())
    {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}



void ChatWindow::handleIncomingMessage(const Tp::ReceivedMessage &message)
{
    if(m_channelInitialised)
    {
        TelepathyChatMessageInfo messageInfo(TelepathyChatMessageInfo::RemoteToLocal);
        messageInfo.setMessage(message.text());
        messageInfo.setSenderScreenName(message.sender()->id());
        messageInfo.setTime(message.received());
        messageInfo.setSenderDisplayName(message.sender()->alias());

        ui->chatArea->addMessage(messageInfo);
        m_chatConnection->channel()->acknowledge(QList<Tp::ReceivedMessage>() << message);
    }
}

void ChatWindow::handleMessageSent(const Tp::Message &message, Tp::MessageSendingFlags, const QString&) /*Not sure what these other args are for*/
{
    TelepathyChatMessageInfo messageInfo(TelepathyChatMessageInfo::LocalToRemote);
    messageInfo.setMessage(message.text());
    messageInfo.setTime(message.sent());
    messageInfo.setSenderDisplayName(m_chatConnection->account()->displayName());

    ui->chatArea->addMessage(messageInfo);
}

void ChatWindow::sendMessage()
{
    if(!ui->sendMessageBox->toPlainText().isEmpty()) {
        m_chatConnection->channel()->send(ui->sendMessageBox->toPlainText());
        ui->sendMessageBox->clear();
    }
}

void ChatWindow::updateChatStatus(Tp::ContactPtr contact, ChannelChatState state)
{
    switch (state)
    {
    case ChannelChatStateGone:
        {
            TelepathyChatMessageInfo statusMessage(TelepathyChatMessageInfo::Status);
            statusMessage.setMessage(i18n("%1 has left the chat").arg(contact->alias()));
            ui->chatArea->addMessage(statusMessage);
        }
        break;
    case ChannelChatStateInactive:
        //FIXME send a 'chat timed out' message to chatview
        break;
    case ChannelChatStateActive:
        //This is the normal state.
        ui->statusLabel->setText("");
    case ChannelChatStatePaused:
        //not sure what this means..safest to do nothing.
        break;
    case ChannelChatStateComposing:
        ui->statusLabel->setText(i18n("%1 is typing a message").arg(contact->alias()));
    }

}

void ChatWindow::updateEnabledState(bool enable)
{
    ui->sendMessageBox->setEnabled(enable);
    ui->sendMessageButton->setEnabled(enable);

    //set up the initial chat window details.
    if(enable)
    {
        TelepathyChatInfo info;
        Tp::Contacts allContacts = m_chatConnection->channel()->groupContacts();

        //normal chat - self and one other person.
        if(allContacts.size() == 2)
        {
            //find the other contact which isn't self.
            foreach(Tp::ContactPtr it, allContacts)
            {
                if(it.data() == m_chatConnection->channel()->groupSelfContact().data())
                {
                    continue;
                }
                else
                {
                    info.setDestinationDisplayName(it->alias());
                    info.setDestinationName(it->id());
                    info.setChatName(it->alias());
                    info.setIncomingIconPath(it->avatarToken());

                }
            }
        }
        else
        {
            //some sort of group chat scenario.. Not sure how to create this yet.
            info.setChatName("Group Chat");
        }

        //set up anything related to 'self'
        info.setOutgoingIconPath(m_chatConnection->channel()->groupSelfContact()->avatarToken());
        info.setTimeOpened(QDateTime::currentDateTime()); //FIXME how do I know when the channel opened? Using current time for now.

        ui->chatArea->initialise(info);
        m_channelInitialised = true;

        //inform anyone using the class of the new name for this chat.
        Q_EMIT titleChanged(info.chatName());

        //process any messages we've 'missed' whilst initialising.
        foreach(Tp::ReceivedMessage message, m_chatConnection->channel()->messageQueue())
        {
            handleIncomingMessage(message);
        }
    }
}

bool MessageBoxEventFilter::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {
            if(!keyEvent->modifiers()) {
                Q_EMIT returnKeyPressed();
                return true;
            }
        }
    }
    // standard event processing
    return QObject::eventFilter(obj, event);
}
