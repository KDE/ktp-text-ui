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
