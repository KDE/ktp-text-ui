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


#ifndef CONVERSATION_H
#define CONVERSATION_H

#include "kdetelepathychat_export.h"

#include <QObject>
#include <TelepathyQt4/Account>
#include <TelepathyQt4/TextChannel>
#include "conversation-model.h"

class ConversationModel;
class KDE_TELEPATHY_CHAT_EXPORT Conversation : public QObject
{
Q_OBJECT
Q_PROPERTY(const ConversationModel* model READ model NOTIFY modelChanged)

public:
    Conversation(Tp::TextChannelPtr channel, Tp::AccountPtr account);
    virtual ~Conversation();

	const ConversationModel* model() const;

Q_SIGNALS:
	void modelChanged(ConversationModel* newModel);

private:
	class ConversationPrivate;
	ConversationPrivate *d;
};

#endif // CONVERSATION_H
