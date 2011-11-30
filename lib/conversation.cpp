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
#include "chat-widget.h"

class Conversation::ConversationPrivate
{
public:
    ConversationModel* model;
    Tp::AccountPtr account;
    Tp::ContactPtr contact;
};

Conversation::Conversation ( Tp::TextChannelPtr channel, Tp::AccountPtr account ) :
        d ( new ConversationPrivate )
{
    kDebug();

    d->model = new ConversationModel();
    d->model->setTextChannel ( channel );

    d->account = account;
    d->contact = channel->targetContact();

    connect(d->contact.constData(), SIGNAL(aliasChanged(QString)), SIGNAL(nickChanged(QString)));
    connect(d->contact.constData(), SIGNAL(avatarDataChanged(Tp::AvatarData)), SLOT(onAvatarDataChanged(Tp::AvatarData)));
    connect(d->contact.constData(), SIGNAL(presenceChanged(Tp::Presence)), SLOT(onPresenceChanged(Tp::Presence)));
}

Conversation::Conversation ( QObject* parent ) : QObject ( parent )
{
    kError() << "Conversation should not be created directly. Use ConversationWatcher instead.";
}

ConversationModel* Conversation::model() const
{
    return d->model;
}

QIcon Conversation::avatar() const
{
    return QIcon(d->contact->avatarData().fileName);
}

QString Conversation::nick() const
{
    return d->contact->alias();
}

QIcon Conversation::presenceIcon() const
{
    return ChatWidget::iconForPresence(d->contact->presence().type());
}

Conversation::~Conversation()
{
    kDebug();
    delete d->model;
}

void Conversation::onPresenceChanged ( Tp::Presence )
{
    Q_EMIT presenceIconChanged(presenceIcon());
}

void Conversation::onAvatarDataChanged ( Tp::AvatarData )
{
    Q_EMIT avatarChanged(avatar());
}
