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

OtrStatus::OtrStatus() 
    : otrImplemented(false) { }

OtrStatus::OtrStatus(Tp::OTRTrustLevel trustLevel) 
    : otrImplemented(true), trustLevel(trustLevel) { }

OtrStatus::operator bool() const 
{
    return otrImplemented;
}

bool OtrStatus::operator!() const 
{
    return !(OtrStatus::operator bool());
}

bool OtrStatus::operator==(const OtrStatus &other) const 
{

    if(otrImplemented != other.otrImplemented) return false;
    else if(otrImplemented) return trustLevel == other.trustLevel;
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

    QVariant waitForOperation(const Tp::PendingVariant *pendingVariant, int timeout) 
    {
        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);
        loop.connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
        loop.connect(pendingVariant, SIGNAL(finished(Tp::PendingOperation*)), &loop, SLOT(quit()));
        timer.start(timeout); // 3s
        loop.exec();

        if(timer.isActive()) timer.stop();
        return pendingVariant->result();
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
