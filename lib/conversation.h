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


#ifndef CONVERSATION_H
#define CONVERSATION_H

#include "ktpchat_export.h"

#include <QObject>
#include <TelepathyQt/Account>
#include <TelepathyQt/TextChannel>
// #include "conversation-model.h"
#include <KIcon>
#include "conversation-que-manager.h"
#include "conversation-target.h"

// class ConversationTarget;
class MessagesModel;
class KDE_TELEPATHY_CHAT_EXPORT Conversation : public QObject
{
Q_OBJECT

// Q_PROPERTY(ConversationTarget* target READ target CONSTANT);
Q_PROPERTY(QObject* target READ target CONSTANT);
//TODO: rename this to messages
Q_PROPERTY(MessagesModel* model READ model CONSTANT);

public:
    Conversation(Tp::TextChannelPtr channel, Tp::AccountPtr account, QObject* parent = 0);
    Conversation(QObject* parent = 0);
    virtual ~Conversation();

    //FIXME: rename model to messages
    MessagesModel* model() const;
    ConversationTarget* target() const;

private:
    class ConversationPrivate;
    ConversationPrivate *d;
};

#endif // CONVERSATION_H
