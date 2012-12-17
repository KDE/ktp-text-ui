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

#ifndef CHANNEL_CONTACT_MODEL_H
#define CHANNEL_CONTACT_MODEL_H

#include <QtCore/QAbstractListModel>

#include <TelepathyQt/TextChannel>
#include <TelepathyQt/Contact>
#include <TelepathyQt/Types>
#include <TelepathyQt/Channel>
#include <TelepathyQt/Presence>

#include <KTp/presence.h>

/** A model of all users in the channel.
  Also acts as a proxy for emiting presence and alias changes of any contacts in the channel
  so that the main class doesn't need to monitor this and keep connection/disconnecting to alias change/presence
  change for notification messages
  */

class ChannelContactModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit ChannelContactModel(const Tp::TextChannelPtr &channel, QObject *parent = 0);

    void setTextChannel(const Tp::TextChannelPtr &channel);

    bool containsNick(const QString &nick);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

Q_SIGNALS:
    void contactPresenceChanged(const Tp::ContactPtr &contact, const KTp::Presence &presence);
    void contactAliasChanged(const Tp::ContactPtr &contact, const QString &alias);
    void contactBlockStatusChanged(const Tp::ContactPtr &contact, bool blocked);

private Q_SLOTS:
    void onGroupMembersChanged(const Tp::Contacts &groupMembersAdded,
                             const Tp::Contacts &groupLocalPendingMembersAdded,
                             const Tp::Contacts &groupRemotePendingMembersAdded,
                             const Tp::Contacts &groupMembersRemoved,
                             const Tp::Channel::GroupMemberChangeDetails &details);
    void onContactPresenceChanged(const Tp::Presence &presence);
    void onContactAliasChanged(const QString &alias);
    void onContactBlockStatusChanged(bool blocked);

private:
    void addContacts(const Tp::Contacts &contacts);
    void removeContacts(const Tp::Contacts &contacts);
    QList<Tp::ContactPtr> m_contacts;
};

#endif // CHANNEL_CONTACT_MODEL_H
