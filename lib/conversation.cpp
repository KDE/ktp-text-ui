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


#include "conversation.h"
#include "conversation-model.h"

#include <TelepathyQt4/TextChannel>
#include <KDebug>

class Conversation::ConversationPrivate
{
public:
    ConversationModel* model;
    Tp::AccountPtr account;
};

Conversation::Conversation ( Tp::TextChannelPtr channel, Tp::AccountPtr account ) :
        d ( new ConversationPrivate )
{
    d->model = new ConversationModel();
    d->model->setTextChannel ( channel );

    d->account = account;
}

Conversation::Conversation ( QObject* parent ) : QObject ( parent )
{
    kError() << "Conversation should not be created directly. Use ConversationWater instead.";
}

const ConversationModel* Conversation::model() const
{
    return d->model;
}

Conversation::~Conversation()
{
    delete d->model;
}

