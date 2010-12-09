/***************************************************************************
 *   Copyright (C) 2010 by David Edmundson <kde@davidedmundson.co.uk>      *
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

#include "chatconnection.h"
#include <TelepathyQt4/PendingReady>
#include <TelepathyQt4/PendingContacts>
#include <TelepathyQt4/ContactManager>

ChatConnection::ChatConnection(QObject *parent, const AccountPtr account, const ConnectionPtr connection,  QList<ChannelPtr> channels)
        : QObject(parent),
        m_account(account),
        m_connection(connection)
{
    //FIXME loop through, find text channel
    if (channels.length() == 1) {
        m_channel = Tp::TextChannelPtr::dynamicCast(channels[0]);
        PendingReady* op = m_channel->becomeReady(Features() << TextChannel::FeatureMessageQueue
                           << TextChannel::FeatureMessageSentSignal
                           << TextChannel::FeatureChatState
                           << TextChannel::FeatureMessageCapabilities
                           << Channel::FeatureCore);


        connect(op, SIGNAL(finished(Tp::PendingOperation*)), this, SLOT(onChannelReady(Tp::PendingOperation*)));
    } else {
        qDebug() << "more than one channel?"; // I don't understand channels yet.
    }

}


//Private slots

void ChatConnection::onChannelReady(Tp::PendingOperation* op)
{
    qDebug() << "done";
    PendingContacts* p = m_connection->contactManager()->upgradeContacts(QList<ContactPtr>::fromSet(m_channel->groupContacts()),
                         Features() << Contact::FeatureAlias
                         << Contact::FeatureAvatarToken
                         << Contact::FeatureAvatarData
                         << Contact::FeatureCapabilities
                         << Contact::FeatureSimplePresence);
    connect(p, SIGNAL(finished(Tp::PendingOperation*)), this, SLOT(onPendingContactsReady(Tp::PendingOperation*)));
    qDebug() << "channel ready";

}

void ChatConnection::onPendingContactsReady(Tp::PendingOperation*)
{
    qDebug() << "contacts ready";
    //m_isChannelReady = true;
    emit(channelReadyStateChanged(true));
}
