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

#include "kdetelepathychat_export.h"

#include "conversation-target.h"
#include <TelepathyQt4/AvatarData>
#include <TelepathyQt4/Presence>
#include <KDebug>
#include <KIconLoader>

class  ConversationTarget::ConversationTargetPrivate
{
public:
    Tp::ContactPtr contact;
};

ConversationTarget::ConversationTarget(Tp::ContactPtr contact) :
        d(new ConversationTargetPrivate)
{
    kDebug();

    if (contact) {
        setupContactSignals(contact);
    }

    d->contact = contact;
}

void ConversationTarget::setupContactSignals(Tp::ContactPtr contact)
{
    connect(contact.constData(), SIGNAL(aliasChanged(QString)), SIGNAL(nickChanged(QString)));
    connect(contact.constData(), SIGNAL(avatarDataChanged(Tp::AvatarData)), SLOT(onAvatarDataChanged(Tp::AvatarData)));
    connect(contact.constData(), SIGNAL(presenceChanged(Tp::Presence)), SLOT(onPresenceChanged(Tp::Presence)));
}

QIcon ConversationTarget::avatar() const
{
    QString path = d->contact->avatarData().fileName;

    if(path.isEmpty()) {
        return KIcon(QLatin1String("im-user"));
    } else {
        return QIcon(path);
    }
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
    return KIcon(presenceIconSource());
}

QString ConversationTarget::presenceIconSource() const
{
    return iconSourceForPresence(d->contact->presence().type());
}

QString ConversationTarget::iconSourceForPresence(Tp::ConnectionPresenceType presence)
{
    QString iconName;

    switch (presence) {
        case Tp::ConnectionPresenceTypeAvailable:
            iconName = QLatin1String("user-online");
            break;
        case Tp::ConnectionPresenceTypeAway:
            iconName = QLatin1String("user-away");
            break;
        case Tp::ConnectionPresenceTypeExtendedAway:
            iconName = QLatin1String("user-away-extended");
            break;
        case Tp::ConnectionPresenceTypeHidden:
            iconName = QLatin1String("user-invisible");
            break;
        case Tp::ConnectionPresenceTypeBusy:
            iconName = QLatin1String("user-busy");
            break;
        default:
            iconName = QLatin1String("user-offline");
            break;
    }

    return iconName;
}


void ConversationTarget::onPresenceChanged(Tp::Presence)
{
    Q_EMIT presenceIconSourceChanged(presenceIconSource());
    Q_EMIT presenceIconChanged(presenceIcon());
}

void ConversationTarget::onAvatarDataChanged(Tp::AvatarData)
{
    Q_EMIT avatarChanged(avatar());
}

Tp::ContactPtr ConversationTarget::contact() const
{
    return d->contact;
}

void ConversationTarget::setContact(Tp::ContactPtr contact)
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

