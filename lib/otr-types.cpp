#include "otr-types.h"

namespace Tp
{

TP_QT_EXPORT bool operator==(const Fingerprint& v1, const Fingerprint& v2)
{
    return ((v1.humanReadableFingerprint == v2.humanReadableFingerprint)
            && (v1.fingerprintRawData == v2.fingerprintRawData)
            );
}

TP_QT_EXPORT QDBusArgument& operator<<(QDBusArgument& arg, const Fingerprint& val)
{
    arg.beginStructure();
    arg << val.humanReadableFingerprint << val.fingerprintRawData;
    arg.endStructure();
    return arg;
}

TP_QT_EXPORT const QDBusArgument& operator>>(const QDBusArgument& arg, Fingerprint& val)
{
    arg.beginStructure();
    arg >> val.humanReadableFingerprint >> val.fingerprintRawData;
    arg.endStructure();
    return arg;
}

KDE_TELEPATHY_CHAT_EXPORT void registerOtrTypes() {
    static bool registered = false;
    if (registered)
        return;
    registered = true;

    qDBusRegisterMetaType<Tp::Fingerprint>();
}

} // namespace Tp
