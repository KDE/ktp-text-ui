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

#include <KDebug>
#include <KIcon>

ChannelContactList::ChannelContactList(const Tp::TextChannelPtr & channel, QObject *parent)
    : QAbstractListModel(parent)
{

    addContacts(channel->groupContacts());

    connect(channel.data(),
            SIGNAL(groupMembersChanged(Tp::Contacts,Tp::Contacts,Tp::Contacts,
                                       Tp::Contacts,Tp::Channel::GroupMemberChangeDetails)),
            SLOT(onGroupMembersChanged(Tp::Contacts,Tp::Contacts,Tp::Contacts,
                                     Tp::Contacts,Tp::Channel::GroupMemberChangeDetails)));
}

void ChannelContactList::onGroupMembersChanged(const Tp::Contacts & groupMembersAdded,
                                             const Tp::Contacts & groupLocalPendingMembersAdded,
                                             const Tp::Contacts & groupRemotePendingMembersAdded,
                                             const Tp::Contacts & groupMembersRemoved,
                                             const Tp::Channel::GroupMemberChangeDetails & details)
{
    kDebug();

    Q_UNUSED(groupLocalPendingMembersAdded);
    Q_UNUSED(groupRemotePendingMembersAdded);
    Q_UNUSED(details);

    addContacts(groupMembersAdded);
    removeContacts(groupMembersRemoved);
}

void ChannelContactList::onContactPresenceChanged(const Tp::Presence &presence)
{
    Tp::ContactPtr contact(qobject_cast<Tp::Contact*>(sender()));

    QModelIndex index = createIndex(m_contacts.lastIndexOf(contact), 0);
    emit dataChanged(index, index);

    emit contactPresenceChanged(contact, presence);}

void ChannelContactList::onContactAliasChanged(const QString &alias)
{
    Tp::ContactPtr contact(qobject_cast<Tp::Contact*>(sender()));

    QModelIndex index = createIndex(m_contacts.lastIndexOf(contact), 0);
    emit dataChanged(index, index);

    emit contactAliasChanged(contact, alias);
}

int ChannelContactList::rowCount(const QModelIndex &parent) const
{
    if (! parent.isValid()) {
        return m_contacts.size();
    }
    return 0;
}

QVariant ChannelContactList::data(const QModelIndex &index, int role) const
{
    if(!index.isValid()) {
        return QVariant();
    }

    int row = index.row();

    switch (role) {
    case Qt::DisplayRole:
        return QVariant(m_contacts[row]->alias());
    case Qt::DecorationRole:
        switch(m_contacts[row]->presence().type()) {
        case Tp::ConnectionPresenceTypeAvailable:
            return QVariant(KIcon("im-user"));
        case Tp::ConnectionPresenceTypeAway:
        case Tp::ConnectionPresenceTypeExtendedAway:
            return QVariant(KIcon("im-user-away"));
        case Tp::ConnectionPresenceTypeBusy:
            return QVariant(KIcon("im-user-busy"));
        case Tp::ConnectionPresenceTypeOffline:
        case Tp::ConnectionPresenceTypeHidden:
            return QVariant(KIcon("im-user-offline"));
        default:
            return QVariant(KIcon("im-user"));
        }
         //icon for presence stuff here? im-user-away etc.
    default:
        return QVariant();
    }
}

void ChannelContactList::addContacts(const Tp::Contacts &contacts)
{
    QList<Tp::ContactPtr> newContacts = contacts.toList();

    foreach(Tp::ContactPtr contact, newContacts) {
        connect(contact.data(), SIGNAL(aliasChanged(QString)), SLOT(onContactAliasChanged(QString)));
        connect(contact.data(), SIGNAL(presenceChanged(Tp::Presence)), SLOT(onContactPresenceChanged(Tp::Presence)));
    }

    beginInsertRows(QModelIndex(), m_contacts.size(), m_contacts.size() + newContacts.size());
    m_contacts << newContacts;
    endInsertRows();
}

void ChannelContactList::removeContacts(const Tp::Contacts &contacts)
{
    foreach(Tp::ContactPtr contact, contacts) {
        //does it make sense to disconnect the signals here too? as technically the contact itself hasn't actually been deleted yet...
        beginRemoveRows(QModelIndex(), m_contacts.indexOf(contact), m_contacts.indexOf(contact));
        m_contacts.removeAll(contact);
        endRemoveRows();
    }
}

