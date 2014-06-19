#ifndef OTR_CHANNEL_INTERFACE_HEADER
#define OTR_CHANNEL_INTERFACE_HEADER

#include <TelepathyQt/Types>
#include "otr-types.h"

#include <QtGlobal>

#include <QString>
#include <QObject>
#include <QVariant>

#include <QDBusPendingReply>

#include <TelepathyQt/AbstractInterface>
#include <TelepathyQt/DBusProxy>
#include <TelepathyQt/Global>

namespace Tp
{
namespace Client
{

/**
 * \class ChannelInterfaceOTR1Interface
 * \headerfile TelepathyQt/channel.h <TelepathyQt/Channel>
 * \ingroup clientchannel
 *
 * Proxy class providing a 1:1 mapping of the D-Bus interface "org.freedesktop.Telepathy.Channel.Interface.OTR1".
 */
class TP_QT_EXPORT ChannelInterfaceOTR1Interface : public Tp::AbstractInterface
{
    Q_OBJECT

public:
    /**
     * Returns the name of the interface "org.freedesktop.Telepathy.Channel.Interface.OTR1", which this class
     * represents.
     *
     * \return The D-Bus interface name.
     */
    static inline QLatin1String staticInterfaceName()
    {
        return QLatin1String("org.freedesktop.Telepathy.Channel.Interface.OTR1");
    }

    /**
     * Creates a ChannelInterfaceOTR1Interface associated with the given object on the session bus.
     *
     * \param busName Name of the service the object is on.
     * \param objectPath Path to the object on the service.
     * \param parent Passed to the parent class constructor.
     */
    ChannelInterfaceOTR1Interface(
        const QString& busName,
        const QString& objectPath,
        QObject* parent = 0
    );

    /**
     * Creates a ChannelInterfaceOTR1Interface associated with the given object on the given bus.
     *
     * \param connection The bus via which the object can be reached.
     * \param busName Name of the service the object is on.
     * \param objectPath Path to the object on the service.
     * \param parent Passed to the parent class constructor.
     */
    ChannelInterfaceOTR1Interface(
        const QDBusConnection& connection,
        const QString& busName,
        const QString& objectPath,
        QObject* parent = 0
    );

    /**
     * Creates a ChannelInterfaceOTR1Interface associated with the same object as the given proxy.
     *
     * \param proxy The proxy to use. It will also be the QObject::parent()
     *               for this object.
     */
    ChannelInterfaceOTR1Interface(Tp::DBusProxy *proxy);

    /**
     * Creates a ChannelInterfaceOTR1Interface associated with the same object as the given proxy.
     * Additionally, the created proxy will have the same parent as the given
     * proxy.
     *
     * \param mainInterface The proxy to use.
     */
    explicit ChannelInterfaceOTR1Interface(const Tp::Client::ChannelInterface& mainInterface);

    /**
     * Creates a ChannelInterfaceOTR1Interface associated with the same object as the given proxy.
     * However, a different parent object can be specified.
     *
     * \param mainInterface The proxy to use.
     * \param parent Passed to the parent class constructor.
     */
    ChannelInterfaceOTR1Interface(const Tp::Client::ChannelInterface& mainInterface, QObject* parent);

    /**
     * Asynchronous getter for the remote object property \c TrustLevel of type \c uint.
     *
     * 
     * \htmlonly
     * <p>The current trust level of this channel:
     *     0=TRUST_NOT_PRIVATE, 1=TRUST_UNVERIFIED, 2=TRUST_PRIVATE,
     *     3=TRUST_FINISHED</p>
     * <p>Clients MUST listen to PropertiesChanged to update UI when trust
     * level changes.</p>
     * \endhtmlonly
     *
     * \return A pending variant which will emit finished when the property has been
     *          retrieved.
     */
    inline Tp::PendingVariant *requestPropertyTrustLevel() const
    {
        return internalRequestProperty(QLatin1String("TrustLevel"));
    }

    /**
     * Asynchronous getter for the remote object property \c LocalFingerprint of type \c Tp::Fingerprint.
     *
     * 
     * \htmlonly
     * <p>User's current fingerprint. The first element is a human readable
     * fingerprint that can be displayed to the user so he can communicate it
     * to the other end by other means so he can trust it. The 2nd element is
     * the fingerprint raw data.</p>
     * \endhtmlonly
     *
     * \return A pending variant which will emit finished when the property has been
     *          retrieved.
     */
    inline Tp::PendingVariant *requestPropertyLocalFingerprint() const
    {
        return internalRequestProperty(QLatin1String("LocalFingerprint"));
    }

    /**
     * Asynchronous getter for the remote object property \c RemoteFingerprint of type \c Tp::Fingerprint.
     *
     * 
     * \htmlonly
     * <p>The current fingerprint of the remote contact. Should be displayed
     * to the user to update its trust level. The first element of the tuple
     * is the fingerprint formatted to be displayed. The 2nd element is the
     * fingerprint raw data that can be passed to TrustFingerprint</p>
     * \endhtmlonly
     *
     * \return A pending variant which will emit finished when the property has been
     *          retrieved.
     */
    inline Tp::PendingVariant *requestPropertyRemoteFingerprint() const
    {
        return internalRequestProperty(QLatin1String("RemoteFingerprint"));
    }

    /**
     * Request all of the DBus properties on the interface.
     *
     * \return A pending variant map which will emit finished when the properties have
     *          been retrieved.
     */
    Tp::PendingVariantMap *requestAllProperties() const
    {
        return internalRequestAllProperties();
    }

public Q_SLOTS:
    /**
     * Begins a call to the D-Bus method \c TrustFingerprint on the remote object.
     * 
     * Set whether or not the user trusts the given fingerprint.
     *
     * Note that \a timeout is ignored as of now. It will be used once
     * http://bugreports.qt.nokia.com/browse/QTBUG-11775 is fixed.
     *
     *
     * \param fingerprint
     *     
     *     The fingerprint.
     *
     * \param trust
     *     
     *     %TRUE if trusted, %FALSE otherwise.
     * \param timeout The timeout in milliseconds.
     */
    inline QDBusPendingReply<> TrustFingerprint(const QByteArray& fingerprint, bool trust, int timeout = -1)
    {
        if (!invalidationReason().isEmpty()) {
            return QDBusPendingReply<>(QDBusMessage::createError(
                invalidationReason(),
                invalidationMessage()
            ));
        }

        QDBusMessage callMessage = QDBusMessage::createMethodCall(this->service(), this->path(),
                this->staticInterfaceName(), QLatin1String("TrustFingerprint"));
        callMessage << QVariant::fromValue(fingerprint) << QVariant::fromValue(trust);
        return this->connection().asyncCall(callMessage, timeout);
    }

    /**
     * Begins a call to the D-Bus method \c Initialize on the remote object.
     * 
     * Start an OTR session for this channel if the remote end supports it has 
     * well.
     *
     * Note that \a timeout is ignored as of now. It will be used once
     * http://bugreports.qt.nokia.com/browse/QTBUG-11775 is fixed.
     *
     * \param timeout The timeout in milliseconds.
     */
    inline QDBusPendingReply<> Initialize(int timeout = -1)
    {
        if (!invalidationReason().isEmpty()) {
            return QDBusPendingReply<>(QDBusMessage::createError(
                invalidationReason(),
                invalidationMessage()
            ));
        }

        QDBusMessage callMessage = QDBusMessage::createMethodCall(this->service(), this->path(),
                this->staticInterfaceName(), QLatin1String("Initialize"));
        return this->connection().asyncCall(callMessage, timeout);
    }

    /**
     * Begins a call to the D-Bus method \c Stop on the remote object.
     * 
     * Stops the OTR session.
     *
     * Note that \a timeout is ignored as of now. It will be used once
     * http://bugreports.qt.nokia.com/browse/QTBUG-11775 is fixed.
     *
     * \param timeout The timeout in milliseconds.
     */
    inline QDBusPendingReply<> Stop(int timeout = -1)
    {
        if (!invalidationReason().isEmpty()) {
            return QDBusPendingReply<>(QDBusMessage::createError(
                invalidationReason(),
                invalidationMessage()
            ));
        }

        QDBusMessage callMessage = QDBusMessage::createMethodCall(this->service(), this->path(),
                this->staticInterfaceName(), QLatin1String("Stop"));
        return this->connection().asyncCall(callMessage, timeout);
    }

protected:
    virtual void invalidate(Tp::DBusProxy *, const QString &, const QString &);
};

} // namespace Client
} // namespace Tp

Q_DECLARE_METATYPE(Tp::Client::ChannelInterfaceOTR1Interface*)

#endif
