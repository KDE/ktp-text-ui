/*
    Copyright (C) 2011 David Edmundson <kde@davidedmundson.co.uk>

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


#ifndef CHANNELDELEGATOR_H
#define CHANNELDELEGATOR_H

#include <TelepathyQt/Account>

//BODGE CLASS to expose dispatcherInterface
class AccountProxy : public Tp::Account
{
public:
    Tp::Client::ChannelDispatcherInterface* dispatcherInterface() {
        return Tp::Account::dispatcherInterface();
    }
};

//BODGE class to allow delegating channels before I merge into TpQt properly
class ChannelDelegator {
public:
    static void delegateChannel(const Tp::AccountPtr &account, const Tp::ChannelPtr &channel, const QDateTime &userActionTime = QDateTime::currentDateTime()) {
        static_cast<AccountProxy*>(account.data())->dispatcherInterface()->DelegateChannels(Tp::ObjectPathList() << QDBusObjectPath(channel->objectPath()), userActionTime.toTime_t(), QLatin1String("org.freedesktop.Telepathy.Client.KTp.TextUi"));
    }
};

#endif // CHANNELDELEGATOR_H
