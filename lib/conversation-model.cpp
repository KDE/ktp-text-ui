/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) 2011  Lasath Fernando <kde@lasath.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "conversation-model.h"

#include <KDebug>
#include <TelepathyQt4/ReceivedMessage>

class MessageItem {
public:
    QString user;
    QString text;
    QDateTime time;

    //FIXME : replace with Tp::ChannelTextMessageType
    enum MessageType {
        Incoming,
        Outgoing,
        Status
    } type;
};

class ConversationModel::ConversationModelPrivate {
public:
    Tp::TextChannelPtr textChannel;
    QList<MessageItem> messages;
};

ConversationModel::ConversationModel(QObject* parent):
    QAbstractListModel(parent),
    d(new ConversationModelPrivate)
{
}

Tp::TextChannelPtr ConversationModel::textChannel()
{
    return d->textChannel;
}

void ConversationModel::setupChannelSignals(Tp::TextChannelPtr channel)
{
    QObject::connect(channel.constData(),
                    SIGNAL(messageReceived(Tp::ReceivedMessage)),
                    SLOT(messageReceived(Tp::ReceivedMessage)));
    QObject::connect(channel.constData(),
                    SIGNAL(messageSent(Tp::Message,Tp::MessageSendingFlags,QString)),
                    SLOT(messageSent(Tp::Message,Tp::MessageSendingFlags,QString)));
}

void ConversationModel::setTextChannel(Tp::TextChannelPtr channel)
{
    setupChannelSignals(channel);
    if(d->textChannel) {
        removeChannelSignals(channel);
    }

    d->textChannel = channel;

    textChannelChanged(channel);
}

void ConversationModel::onMessageReceived(Tp::ReceivedMessage message)
{
    beginInsertRows(QModelIndex(), d->messages.count(), d->messages.count());

    MessageItem newMessage = {
        message.sender()->alias(),
        message.text(),
        message.sent(),
        MessageItem::Incoming
    };

    d->messages.append(newMessage);
    endInsertRows();
}

void ConversationModel::onMessageSent(Tp::Message message, Tp::MessageSendingFlags flags, QString token)
{
    Q_UNUSED(flags);
    Q_UNUSED(token);
    beginInsertRows(QModelIndex(), d->messages.count(), d->messages.count());

    MessageItem newMessage = {
        tr("Me"),   //FIXME : use actual nickname from Tp::AccountPtr
        message.text(),
        message.sent(),
        MessageItem::Outgoing
    };

    d->messages.append(newMessage);
    endInsertRows();
}

QVariant ConversationModel::data(const QModelIndex& index, int role) const
{
    QVariant result;

    if(!index.isValid()) {
        kError() << "Attempting to access data at invalid index (" << index << ")";
    } else {
        MessageItem* requestedData = &d->messages[index.row()];

        switch(role) {
            case UserRole:
                result = requestedData->user;
                break;
            case TextRole:
                result = requestedData->text;
                break;
            case TypeRole:
                result = requestedData->type;
                break;
            case TimeRole:
                result = requestedData->time;
                break;
        };
    }

    return result;
}

int ConversationModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return d->messages.count();
}

void ConversationModel::removeChannelSignals(Tp::TextChannelPtr channel)
{
    QObject::disconnect(channel.constData(),
                        SIGNAL(messageReceived(Tp::ReceivedMessage)),
                        this,
                        SLOT(onMessageReceived(Tp::ReceivedMessage))
                    );
    QObject::disconnect(channel.constData(),
                        SIGNAL(messageSent(Tp::Message,Tp::MessageSendingFlags,QString)),
                        this,
                        SLOT(onMessageSent(Tp::Message,Tp::MessageSendingFlags,QString))
                    );
}

#include "moc_conversation-model.cpp"
