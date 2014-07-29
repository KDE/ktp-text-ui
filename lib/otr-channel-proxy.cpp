#define IN_TP_QT_HEADER
#include "otr-channel-proxy.h"

namespace Tp
{
namespace Client
{

ChannelProxyInterfaceOTRInterface::ChannelProxyInterfaceOTRInterface(const QString& busName, const QString& objectPath, QObject *parent)
    : Tp::AbstractInterface(busName, objectPath, staticInterfaceName(), QDBusConnection::sessionBus(), parent)
{
}

ChannelProxyInterfaceOTRInterface::ChannelProxyInterfaceOTRInterface(const QDBusConnection& connection, const QString& busName, const QString& objectPath, QObject *parent)
    : Tp::AbstractInterface(busName, objectPath, staticInterfaceName(), connection, parent)
{
}

ChannelProxyInterfaceOTRInterface::ChannelProxyInterfaceOTRInterface(Tp::DBusProxy *proxy)
    : Tp::AbstractInterface(proxy, staticInterfaceName())
{
}

ChannelProxyInterfaceOTRInterface::ChannelProxyInterfaceOTRInterface(const Tp::AbstractInterface& mainInterface)
    : Tp::AbstractInterface(mainInterface.service(), mainInterface.path(), staticInterfaceName(), mainInterface.connection(), mainInterface.parent())
{
}

ChannelProxyInterfaceOTRInterface::ChannelProxyInterfaceOTRInterface(const Tp::AbstractInterface& mainInterface, QObject *parent)
    : Tp::AbstractInterface(mainInterface.service(), mainInterface.path(), staticInterfaceName(), mainInterface.connection(), parent)
{
}

void ChannelProxyInterfaceOTRInterface::invalidate(Tp::DBusProxy *proxy,
        const QString &error, const QString &message)
{
    disconnect(this, SIGNAL(MessageSent(const Tp::MessagePartList&, uint, const QString&)), NULL, NULL);
    disconnect(this, SIGNAL(MessageReceived(const Tp::MessagePartList&)), NULL, NULL);
    disconnect(this, SIGNAL(PendingMessagesRemoved(const Tp::UIntList&)), NULL, NULL);
    disconnect(this, SIGNAL(SessionRefreshed()), NULL, NULL);
    disconnect(this, SIGNAL(TrustLevelChanged(uint)), NULL, NULL);

    Tp::AbstractInterface::invalidate(proxy, error, message);
}
}
}
