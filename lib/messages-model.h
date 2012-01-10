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


#ifndef MESSAGES_MODEL_H
#define MESSAGES_MODEL_H

#include "ktpchat_export.h"

#include <QAbstractItemModel>
#include <TelepathyQt/TextChannel>
#include "conversation-que-manager.h"


class KDE_TELEPATHY_CHAT_EXPORT MessagesModel : public QAbstractListModel, public Queable
{
    Q_OBJECT
    Q_PROPERTY(bool visibleToUser READ isVisibleToUser WRITE setVisibleToUser NOTIFY visibleToUserChanged);
    Q_PROPERTY(int unreadCount READ unreadCount NOTIFY unreadCountChanged);

public:
    MessagesModel(QObject *parent = 0);
    virtual ~MessagesModel();

    enum Roles {
        UserRole = Qt::UserRole,
        TextRole,
        TypeRole,
        TimeRole
    };

    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

    Tp::TextChannelPtr textChannel();
    void setTextChannel(Tp::TextChannelPtr channel);

    bool isVisibleToUser() const;
    void setVisibleToUser(bool visible);

    void acknowledgeAllMessages();
    int unreadCount() const;

    //debug function. will do whatever I feel like at the time ;-)
    Q_INVOKABLE void printallmessages();
Q_SIGNALS:
    void textChannelChanged(Tp::TextChannelPtr newChannel);
    void visibleToUserChanged(bool visible);
    //TODO: figure out how to check if unread messages have been acknowledged by something else
    void unreadCountChanged(int unreadMesssagesCount);
    void popoutRequested();

public Q_SLOTS:
    void sendNewMessage(const QString& message);

private Q_SLOTS:
    void onMessageReceived(Tp::ReceivedMessage);
    void onMessageSent(Tp::Message,Tp::MessageSendingFlags,QString);
    bool verifyPendingOperation(Tp::PendingOperation *op);

private:
    void setupChannelSignals(Tp::TextChannelPtr channel);
    void removeChannelSignals(Tp::TextChannelPtr channel);
    virtual void selfDequed();

    class MessagesModelPrivate;
    MessagesModelPrivate *d;
};

#endif // CONVERSATION_MODEL_H
