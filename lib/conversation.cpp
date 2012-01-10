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


#include "conversation.h"
#include "messages-model.h"

#include <TelepathyQt/TextChannel>
#include <KDebug>
#include "conversation-target.h"

class Conversation::ConversationPrivate
{
public:
    MessagesModel* model;
    ConversationTarget* target;
//     Tp::AccountPtr account;
};

Conversation::Conversation(const Tp::TextChannelPtr& channel, const Tp::AccountPtr& account, QObject* parent) :
        QObject(parent),
        d (new ConversationPrivate)
{
    kDebug();

    d->model = new MessagesModel(this);
    d->model->setTextChannel(channel);

    d->target = new ConversationTarget(channel->targetContact(), this);

//     connect(model(), SIGNAL(unreadCountChanged(int)), SLOT(onUnreadMessagesChanged()));
//     d->account = account;
}

Conversation::Conversation(QObject* parent) : QObject(parent)
{
    kError() << "Conversation should not be created directly. Use ConversationWatcher instead.";
    Q_ASSERT(false);
}

MessagesModel* Conversation::model() const
{
    return d->model;
}

ConversationTarget* Conversation::target() const
{
    return d->target;
}

// void Conversation::onUnreadMessagesChanged()
// {
//     enqueSelf();
// }
// 
// void Conversation::selfDequed()
// {
//     Q_EMIT popoutRequested();
// }

Conversation::~Conversation()
{
    kDebug();
    delete d;
}
