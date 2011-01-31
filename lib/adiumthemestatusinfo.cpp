/***************************************************************************
 *   Copyright (C) 2011 by David Edmundson <kde@davidedmundson.co.uk>      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
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

#include "adiumthemestatusinfo.h"
#include <QtCore/QString>

class AdiumThemeStatusInfoPrivate
{
public:
    QString status;
};

AdiumThemeStatusInfo::AdiumThemeStatusInfo()
    : AdiumThemeMessageInfo(AdiumThemeMessageInfo::Status),
      d(new AdiumThemeStatusInfoPrivate)
{
}

AdiumThemeStatusInfo::AdiumThemeStatusInfo(const AdiumThemeStatusInfo &other)
    : AdiumThemeMessageInfo(other),
      d(new AdiumThemeStatusInfoPrivate(*other.d))
{

}

AdiumThemeStatusInfo::~AdiumThemeStatusInfo()
{
    delete d;
}


AdiumThemeStatusInfo& AdiumThemeStatusInfo::operator=(const AdiumThemeStatusInfo& other)
{
    *d = *other.d;
    return *this;
}

QString AdiumThemeStatusInfo::status() const
{
    return d->status;
}

void AdiumThemeStatusInfo::setStatus(const QString& status)
{
    d->status = status;
}

