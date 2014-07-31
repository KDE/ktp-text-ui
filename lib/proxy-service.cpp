#define IN_TP_QT_HEADER
#include "proxy-service.h"

namespace Tp
{
namespace Client
{

ProxyServiceInterface::ProxyServiceInterface(const QString& busName, const QString& objectPath, QObject *parent)
    : Tp::AbstractInterface(busName, objectPath, staticInterfaceName(), QDBusConnection::sessionBus(), parent)
{
}

ProxyServiceInterface::ProxyServiceInterface(const QDBusConnection& connection, const QString& busName, const QString& objectPath, QObject *parent)
    : Tp::AbstractInterface(busName, objectPath, staticInterfaceName(), connection, parent)
{
}

ProxyServiceInterface::ProxyServiceInterface(Tp::DBusProxy *proxy)
    : Tp::AbstractInterface(proxy, staticInterfaceName())
{
}

ProxyServiceInterface::ProxyServiceInterface(const Tp::AbstractInterface& mainInterface)
    : Tp::AbstractInterface(mainInterface.service(), mainInterface.path(), staticInterfaceName(), mainInterface.connection(), mainInterface.parent())
{
}

ProxyServiceInterface::ProxyServiceInterface(const Tp::AbstractInterface& mainInterface, QObject *parent)
    : Tp::AbstractInterface(mainInterface.service(), mainInterface.path(), staticInterfaceName(), mainInterface.connection(), parent)
{
}

void ProxyServiceInterface::invalidate(Tp::DBusProxy *proxy,
        const QString &error, const QString &message)
{
    disconnect(this, SIGNAL(ProxyConnected(const QDBusObjectPath&)), NULL, NULL);
    disconnect(this, SIGNAL(ProxyDisconnected(const QDBusObjectPath&)), NULL, NULL);
    disconnect(this, SIGNAL(KeyGenerationStarted(const QDBusObjectPath&)), NULL, NULL);
    disconnect(this, SIGNAL(KeyGenerationFinished(const QDBusObjectPath&, bool)), NULL, NULL);

    Tp::AbstractInterface::invalidate(proxy, error, message);
}
}
}
