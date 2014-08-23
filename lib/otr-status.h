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

#ifndef OTR_STATUS_HEADER
#define OTR_STATUS_HEADER

#include <KTp/OTR/constants.h>

#include <ktpchat_export.h>

class KDE_TELEPATHY_CHAT_EXPORT OtrStatus
{
    public:
        /** Creates OtrStatus with bool() returning false */
        OtrStatus();
        /** Creates for given trust level. bool() returns true */
        OtrStatus(KTp::OTRTrustLevel trustLevel);

        /** Returns true if otr is supported */
        operator bool() const;
        bool operator!() const;

        bool operator==(const OtrStatus &other) const;
        bool operator!=(const OtrStatus &other) const;

        /** Returns valid OTRTrustLevel if and only if operator bool() returns true */
        KTp::OTRTrustLevel otrTrustLevel() const;

    private:
        bool otrConnected;
        KTp::OTRTrustLevel trustLevel;
};


#endif
