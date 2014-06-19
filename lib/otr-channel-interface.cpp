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

