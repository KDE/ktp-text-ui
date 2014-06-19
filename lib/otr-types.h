#ifndef OTR_TYPES_HEADER
#define OTR_TYPES_HEADER

#include <QtGlobal>

#include <QByteArray>
#include <QString>

#include <QDBusArgument>
#include <QDBusMetaType>
#include <QDBusSignature>
#include <QDBusVariant>

#include <TelepathyQt/Global>

#include <ktpchat_export.h>

namespace Tp
{

/**
 * \struct Fingerprint
 * \ingroup struct
 * \headerfile TelepathyQt/types.h <TelepathyQt/Types>
 *
 * Structure type generated from the specification.
 * 
 * An OTR fingerprint.
 */
struct TP_QT_EXPORT Fingerprint
{
    /**
     * Human readable fingerprint
     */
    QString humanReadableFingerprint;
    /**
     * Raw data of fingerprint
     */
    QByteArray fingerprintRawData;
};

TP_QT_EXPORT bool operator==(const Fingerprint& v1, const Fingerprint& v2);
inline bool operator!=(const Fingerprint& v1, const Fingerprint& v2)
{
    return !operator==(v1, v2);
}
TP_QT_EXPORT QDBusArgument& operator<<(QDBusArgument& arg, const Fingerprint& val);
TP_QT_EXPORT const QDBusArgument& operator>>(const QDBusArgument& arg, Fingerprint& val);

KDE_TELEPATHY_CHAT_EXPORT void registerOtrTypes();

} // namespace Tp

Q_DECLARE_METATYPE(Tp::Fingerprint)

#endif
