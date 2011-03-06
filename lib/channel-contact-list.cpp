/***************************************************************************
 *   Copyright (C) 2011 by David Edmundson <kde@davidedmundson.co.uk>      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#include "channel-contact-list.h"
#include <TelepathyQt4/Contact>
#include <KDebug>

/** \breif This class provides an abstracted way of getting presence changes from a contact
  * It was needed to be able to pair up a contact changing, with the name of the contact.
  * It gives a single signal for the calling code to handle for every contact, and
  * also safetly handles connect and disconnecting slots as contacts enter and leave a channel
*/

/** Internal private class*/

/*
 [23:50] <oggis_> if you want to show events just for the current members, then i'd maintain said list/set by connecting to
groupMembersChanged and adding to it everybody in the groupMembersAdded set, and removing to it everybody in any of the
other sets (taking set union of them is easiest)
 */


ChannelContactListContact::ChannelContactListContact(const Tp::ContactPtr & contact, QObject *parent)
    : QObject(parent)
{
    m_contact = contact;
    connect(m_contact.data(), SIGNAL(presenceChanged(Tp::Presence)),
            SLOT(onPresenceChanged(Tp::Presence)));
    connect(m_contact.data(), SIGNAL(aliasChanged(QString)), SLOT(onAliasChanged(QString)));
}

void ChannelContactListContact::onPresenceChanged(const Tp::Presence & presence)
{
    Q_EMIT contactPresenceChanged(m_contact, presence);
}

void ChannelContactListContact::onAliasChanged(const QString &alias)
{
    Q_EMIT contactAliasChanged(m_contact, alias);
}

ChannelContactList::ChannelContactList(const Tp::TextChannelPtr & channel, QObject *parent)
    : QObject(parent)
{
    foreach(Tp::ContactPtr contact, channel->groupContacts()) {
        //FIXME move this to a slot called "addContact" - also call this when chat gains a person.
        ChannelContactListContact*  contactProxy = new ChannelContactListContact(contact, this);
        connect(contactProxy, SIGNAL(contactPresenceChanged(Tp::ContactPtr,Tp::Presence)),
                SIGNAL(contactPresenceChanged(Tp::ContactPtr,Tp::Presence)));
        connect(contactProxy, SIGNAL(contactAliasChanged(Tp::ContactPtr,QString)),
                SIGNAL(contactAliasChanged(Tp::ContactPtr,QString)));
    }
    connect(channel.data(),
            SIGNAL(groupMembersChanged(Tp::Contacts,Tp::Contacts,Tp::Contacts,
                                       Tp::Contacts,Tp::Channel::GroupMemberChangeDetails)),
            SLOT(groupMembersChanged(Tp::Contacts,Tp::Contacts,Tp::Contacts,
                                     Tp::Contacts,Tp::Channel::GroupMemberChangeDetails)));
}

void ChannelContactList::groupMembersChanged(const Tp::Contacts & groupMembersAdded,
                                             const Tp::Contacts & groupLocalPendingMembersAdded,
                                             const Tp::Contacts & groupRemotePendingMembersAdded,
                                             const Tp::Contacts & groupMembersRemoved,
                                             const Tp::Channel::GroupMemberChangeDetails & details)
{
    kDebug() << "members changed.";
}
