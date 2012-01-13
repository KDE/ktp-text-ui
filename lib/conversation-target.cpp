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

#include "conversation-target.h"
#include <TelepathyQt/AvatarData>
#include <TelepathyQt/Presence>
#include <KDebug>
#include <KIconLoader>

class  ConversationTarget::ConversationTargetPrivate
{
public:
    Tp::ContactPtr contact;
    KIcon avatar;
};

ConversationTarget::ConversationTarget(Tp::ContactPtr contact, QObject *parent) :
    QObject(parent),
    d(new ConversationTargetPrivate)
{
    kDebug();

    if (contact) {
        setupContactSignals(contact);
    }

    d->contact = contact;
    updateAvatar();
}

void ConversationTarget::setupContactSignals(Tp::ContactPtr contact)
{
    connect(contact.constData(), SIGNAL(aliasChanged(QString)), SIGNAL(nickChanged(QString)));
    connect(contact.constData(), SIGNAL(avatarDataChanged(Tp::AvatarData)), SLOT(onAvatarDataChanged(Tp::AvatarData)));
    connect(contact.constData(), SIGNAL(presenceChanged(Tp::Presence)), SLOT(onPresenceChanged(Tp::Presence)));
}

QIcon ConversationTarget::avatar() const
{
    return d->avatar;
}

QString ConversationTarget::id() const
{
    return d->contact->id();
}

QString ConversationTarget::nick() const
{
    return d->contact->alias();
}

QIcon ConversationTarget::presenceIcon() const
{
    return KTp::Presence(d->contact->presence()).icon();
}

void ConversationTarget::onPresenceChanged(const Tp::Presence&)
{
    Q_EMIT presenceIconChanged(presenceIcon());
}

void ConversationTarget::onAvatarDataChanged(const Tp::AvatarData&)
{
    updateAvatar();
    Q_EMIT avatarChanged(avatar());
}

void ConversationTarget::updateAvatar()
{
    QString path = d->contact->avatarData().fileName;

    if(path.isEmpty()) {
        path = QLatin1String("im-user");
    }

    d->avatar = KIcon(path);
}

Tp::ContactPtr ConversationTarget::contact() const
{
    return d->contact;
}

void ConversationTarget::setContact(const Tp::ContactPtr &contact)
{
    if (d->contact) {
        removeContactSignals(d->contact);
    }

    d->contact = contact;
    setupContactSignals(d->contact);
    Q_EMIT contactChanged(contact);
}

void ConversationTarget::removeContactSignals(Tp::ContactPtr contact)
{
    disconnect(contact.constData(), SIGNAL(aliasChanged(QString)), this, SIGNAL(nickChanged(QString)));
    disconnect(contact.constData(), SIGNAL(avatarDataChanged(Tp::AvatarData)), this, SLOT(onAvatarDataChanged(Tp::AvatarData)));
    disconnect(contact.constData(), SIGNAL(presenceChanged(Tp::Presence)), this, SLOT(onPresenceChanged(Tp::Presence)));
}

ConversationTarget::~ConversationTarget()
{
    delete d;
}
