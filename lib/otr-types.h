/***************************************************************************
 *   Copyright (C) 2008 Collabora Limited <http://www.collabora.co.uk>     *
 *   Copyright (C) 2008 Nokia Corporation                                  *
 *   Copyright (C) 2014 Marcin Ziemiński <zieminn@gmail.com>
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
