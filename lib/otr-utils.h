/*
    Copyright (C) 2014  Marcin Ziemiński   <zieminn@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef OTR_UTILS_HEADER
#define OTR_UTILS_HEADER

#include "otr-constants.h"
#include "otr-channel-proxy.h"

#include <ktpchat_export.h>
#include <TelepathyQt/PendingVariant>
#include <TelepathyQt/Message>
#include <TelepathyQt/Types>
#include <TelepathyQt/ReceivedMessage>

class KDE_TELEPATHY_CHAT_EXPORT OtrStatus
{
    public:
        /** Creates OtrStatus with bool() returning false */
        OtrStatus();
        /** Creates for given trust level. bool() returns true */
        OtrStatus(Tp::OTRTrustLevel trustLevel);

        /** Returns true if otr is supported */
        operator bool() const;
        bool operator!() const;

        bool operator==(const OtrStatus &other) const;
        bool operator!=(const OtrStatus &other) const;

        /** Returns valid OTRTrustLevel if and only if operator bool() returns true */
        Tp::OTRTrustLevel otrTrustLevel() const;

    private:
        bool otrConnected;
        Tp::OTRTrustLevel trustLevel;
};

namespace Tp {
namespace Utils {

    /** Extracts pending-messages-ids from message list */
    Tp::UIntList getPendingMessagesIDs(const QList<Tp::ReceivedMessage> &messageQueue);

    /** Extracts pending-mesage-id from message */
    uint getId(const Tp::MessagePartList &message);

    /** Returns an object path for the otr proxy channel given a text channel it corresponds to */
    QString getOtrProxyObjectPathFor(const Tp::TextChannelPtr &textChannel);

    /** Returns true if message is generated internally by OTR implementation */
    bool isOtrEvent(const Tp::ReceivedMessage &message);

    /** Returns notification for a user assuming that the message is an otr event */
    QString processOtrMessage(const Tp::ReceivedMessage &message);
}
}

#endif
