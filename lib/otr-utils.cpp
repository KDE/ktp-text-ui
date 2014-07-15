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

#include "otr-utils.h"
#include "otr-constants.h"

#include <QEventLoop>
#include <QTimer>
#include <KLocalizedString>
#include <KDebug>

#include <TelepathyQt/TextChannel>
#include <TelepathyQt/Connection>

OtrStatus::OtrStatus()
    : otrConnected(false) { }

OtrStatus::OtrStatus(Tp::OTRTrustLevel trustLevel)
    : otrConnected(true), trustLevel(trustLevel) { }

OtrStatus::operator bool() const
{
    return otrConnected;
}

bool OtrStatus::operator!() const
{
    return !(OtrStatus::operator bool());
}

bool OtrStatus::operator==(const OtrStatus &other) const
{

    if(otrConnected != other.otrConnected) return false;
    else if(otrConnected) return trustLevel == other.trustLevel;
    else return true;
}

bool OtrStatus::operator!=(const OtrStatus &other) const
{
    return !(*this == other);
}

Tp::OTRTrustLevel OtrStatus::otrTrustLevel() const
{
    return trustLevel;
}

namespace Tp {
namespace Utils {

    Tp::UIntList getPendingMessagesIDs(const QList<Tp::ReceivedMessage> &messageQueue)
    {
        Tp::UIntList ids;
        Q_FOREACH(const Tp::ReceivedMessage &mes, messageQueue) {
            ids << getId(mes.parts());
        }
        return ids;
    }

    uint getId(const Tp::MessagePartList &message) {
        return message.first()[QLatin1String("pending-message-id")].variant().toUInt(NULL);
    }

    QString getOtrProxyObjectPathFor(const Tp::TextChannelPtr &textChannel)
    {
        int index;
        QString connectionId = textChannel->connection()->objectPath();
        index = connectionId.lastIndexOf(QChar::fromLatin1('/'));
        connectionId = connectionId.mid(index+1);

        QString channelId = textChannel->objectPath();
        index = channelId.lastIndexOf(QChar::fromLatin1('/'));
        channelId = channelId.mid(index+1);

        return QString::fromLatin1("%1%2/%3").arg(KTP_PROXY_CHANNEL_OBJECT_PATH_PREFIX, connectionId, channelId);
    }

    bool isOtrEvent(const Tp::ReceivedMessage &message)
    {
        return message.part(0).contains(OTR_MESSAGE_EVENT_HEADER);
    }

    QString processOtrMessage(const Tp::ReceivedMessage &message)
    {
        Tp::MessagePart messagePart = message.part(0);
        OTRMessageEvent otrEvent = static_cast<OTRMessageEvent>(
                messagePart[OTR_MESSAGE_EVENT_HEADER].variant().toUInt(0));

        switch(otrEvent) {

            case Tp::OTRL_MSGEVENT_SETUP_ERROR:
            case Tp::OTRL_MSGEVENT_RCVDMSG_GENERAL_ERR:
                {
                    QString otrError = messagePart[OTR_ERROR_HEADER].variant().toString();
                    return i18n("OTR error: %1", otrError);
                }

            case Tp::OTRL_MSGEVENT_RCVDMSG_UNENCRYPTED:
                {
                    QString unencryptedMessage = messagePart[OTR_UNENCRYPTED_MESSAGE_HEADER].variant().toString();
                    return i18n("Received unencrypted message: [%1]", unencryptedMessage);
                }

            default:
                return message.text();
        }
    }
}
}
