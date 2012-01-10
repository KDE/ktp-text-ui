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


#ifndef CONVERSATIONS_MODEL_H
#define CONVERSATIONS_MODEL_H

#include <QAbstractListModel>

#include "ktpchat_export.h"


class Conversation;

class KDE_TELEPATHY_CHAT_EXPORT ConversationsModel : public QAbstractListModel
{
Q_OBJECT
public:
    explicit ConversationsModel(QObject *parent = 0);
    virtual ~ConversationsModel();

    virtual QVariant data ( const QModelIndex& index, int role = Qt::DisplayRole ) const;
    virtual int rowCount ( const QModelIndex& parent = QModelIndex() ) const;

    enum role {
        ConversationRole = Qt::UserRole
    };

private:
    class ConversationsModelPrivate;
    ConversationsModelPrivate *d;

private Q_SLOTS:
    void onInconmingConversation(Conversation *convo);
};

#endif // CONVERSATIONS_MODEL_H
