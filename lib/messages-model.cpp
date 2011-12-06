/*
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


#include "messages-model.h"

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

class MessagesModel::ConversationModelPrivate {
public:
    Tp::TextChannelPtr textChannel;
    QList<MessageItem> messages;
};

MessagesModel::MessagesModel(QObject* parent):
    QAbstractListModel(parent),
    d(new ConversationModelPrivate)
{
    kDebug();

    QHash<int, QByteArray> roles;
    roles[UserRole] = "user";
    roles[TextRole] = "text";
    roles[TimeRole] = "time";
    roles[TypeRole] = "type";
    setRoleNames(roles);
}

Tp::TextChannelPtr MessagesModel::textChannel()
{
    return d->textChannel;
}

bool MessagesModel::verifyPendingOperation ( Tp::PendingOperation* op )
{
    bool success = !op->isError();
    if(!success) {
        kWarning() << op->errorName() << "+" << op->errorMessage();
    }
    return success;
}

void MessagesModel::setupChannelSignals(Tp::TextChannelPtr channel)
{
    QObject::connect(channel.constData(),
                    SIGNAL(messageReceived(Tp::ReceivedMessage)),
                    SLOT(onMessageReceived(Tp::ReceivedMessage)));
    QObject::connect(channel.constData(),
                    SIGNAL(messageSent(Tp::Message,Tp::MessageSendingFlags,QString)),
                    SLOT(onMessageSent(Tp::Message,Tp::MessageSendingFlags,QString)));
}

void MessagesModel::setTextChannel(Tp::TextChannelPtr channel)
{
    kDebug();
    setupChannelSignals(channel);

    if(d->textChannel) {
        removeChannelSignals(channel);
    }
    //FIXME: check messageQue for any lost messages
    d->textChannel = channel;

    textChannelChanged(channel);
}

void MessagesModel::onMessageReceived(Tp::ReceivedMessage message)
{
    kDebug() << "unreadMessagesCount = " << d->textChannel->messageQueue().size();
    int length = rowCount();
    beginInsertRows(QModelIndex(), length, length);

    MessageItem newMessage = {
        message.sender()->alias(),
        message.text(),
        message.received(),
        MessageItem::Incoming
    };

    d->messages.append(newMessage);
    endInsertRows();
}

void MessagesModel::onMessageSent(Tp::Message message, Tp::MessageSendingFlags flags, QString token)
{
    Q_UNUSED(flags);
    Q_UNUSED(token);
    int length = rowCount();
    beginInsertRows(QModelIndex(), length, length);

    MessageItem newMessage = {
        tr("Me"),   //FIXME : use actual nickname from Tp::AccountPtr
        message.text(),
        message.sent(),
        MessageItem::Outgoing
    };

    d->messages.append(newMessage);
    endInsertRows();
}

QVariant MessagesModel::data(const QModelIndex& index, int role) const
{
    QVariant result;

    if(index.row() < d->messages.size()) {
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
    } else {
        kError() << "Attempting to access data at invalid index (" << index << ")";
    }

    return result;
}

int MessagesModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return d->messages.size();
}

Tp::PendingSendMessage* MessagesModel::sendNewMessage ( QString message )
{
    Tp::PendingSendMessage* msg = 0;

    if(message.isEmpty()) {
        kWarning() << "Attempting to send empty string";
    } else {
        msg = d->textChannel->send(message);
        connect(msg, SIGNAL(finished(Tp::PendingOperation*)),
                SLOT(verifyPendingOperation(Tp::PendingOperation*)));
    }

    return msg;
}

void MessagesModel::removeChannelSignals(Tp::TextChannelPtr channel)
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

MessagesModel::~MessagesModel()
{
    kDebug();
    delete d;
}

#include "moc_messages-model.cpp"
