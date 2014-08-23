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

#include "otr-status.h"

OtrStatus::OtrStatus()
    : otrConnected(false) { }

OtrStatus::OtrStatus(KTp::OTRTrustLevel trustLevel)
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

KTp::OTRTrustLevel OtrStatus::otrTrustLevel() const
{
    return trustLevel;
}
