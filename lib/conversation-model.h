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


#ifndef CONVERSATION_MODEL_H
#define CONVERSATION_MODEL_H

#include "kdetelepathychat_export.h"

#include <QAbstractItemModel>
#include <TelepathyQt4/TextChannel>


class KDE_TELEPATHY_CHAT_EXPORT ConversationModel : public QAbstractListModel
{
Q_OBJECT
// Q_PROPERTY(Tp::TextChannelPtr textChannel
// 			READ textChannel
// 			WRITE setTextChannel
// 			NOTIFY textChannelChanged
// 		  )

public:
    ConversationModel(QObject* parent = 0);
    virtual ~ConversationModel();

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

Q_SIGNALS:
	void textChannelChanged(Tp::TextChannelPtr newChannel);

private Q_SLOTS:
    void onMessageReceived(Tp::ReceivedMessage);
    void onMessageSent(Tp::Message,Tp::MessageSendingFlags,QString);

private:
	void setupChannelSignals(Tp::TextChannelPtr channel);
	void removeChannelSignals(Tp::TextChannelPtr channel);

	class ConversationModelPrivate;
	ConversationModelPrivate *d;
};

#endif // CONVERSATION_MODEL_H
