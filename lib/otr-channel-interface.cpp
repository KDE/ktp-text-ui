/***************************************************************************
 *   Copyright (C) 2008 Collabora Limited <http://www.collabora.co.uk>     *
 *   Copyright (C) 2008 Nokia Corporation                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2.1 of the License, or   *
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

#include <TelepathyQt/Channel>
#include "otr-channel-interface.h"

namespace Tp
{
namespace Client
{

ChannelInterfaceOTR1Interface::ChannelInterfaceOTR1Interface(const QString& busName, const QString& objectPath, QObject *parent)
    : Tp::AbstractInterface(busName, objectPath, staticInterfaceName(), QDBusConnection::sessionBus(), parent)
{
}

ChannelInterfaceOTR1Interface::ChannelInterfaceOTR1Interface(const QDBusConnection& connection, const QString& busName, const QString& objectPath, QObject *parent)
    : Tp::AbstractInterface(busName, objectPath, staticInterfaceName(), connection, parent)
{
}

ChannelInterfaceOTR1Interface::ChannelInterfaceOTR1Interface(Tp::DBusProxy *proxy)
    : Tp::AbstractInterface(proxy, staticInterfaceName())
{
}

ChannelInterfaceOTR1Interface::ChannelInterfaceOTR1Interface(const Tp::Client::ChannelInterface& mainInterface)
    : Tp::AbstractInterface(mainInterface.service(), mainInterface.path(), staticInterfaceName(), mainInterface.connection(), mainInterface.parent())
{
}

ChannelInterfaceOTR1Interface::ChannelInterfaceOTR1Interface(const Tp::Client::ChannelInterface& mainInterface, QObject *parent)
    : Tp::AbstractInterface(mainInterface.service(), mainInterface.path(), staticInterfaceName(), mainInterface.connection(), parent)
{
}

void ChannelInterfaceOTR1Interface::invalidate(Tp::DBusProxy *proxy,
        const QString &error, const QString &message)
{

    Tp::AbstractInterface::invalidate(proxy, error, message);
}

} // namespace Client
} // namespace Tp

