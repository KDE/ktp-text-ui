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

#include "adium-theme-content-info.h"
#include <QString>

class AdiumThemeContentInfoPrivate
{
public:
    QString userIconPath;
    QString senderScreenName;
    QString sender;
    QString senderColor;
    QString senderStatusIcon;
    QString senderDisplayName;
    QString textbackgroundcolor;
};

AdiumThemeContentInfo::AdiumThemeContentInfo()
    : AdiumThemeMessageInfo(),
    d(new AdiumThemeContentInfoPrivate)
{
}


AdiumThemeContentInfo::AdiumThemeContentInfo(AdiumThemeMessageInfo::MessageType type)
    : AdiumThemeMessageInfo(type),
      d(new AdiumThemeContentInfoPrivate)
{
}

AdiumThemeContentInfo::AdiumThemeContentInfo(const AdiumThemeContentInfo &other)
    : AdiumThemeMessageInfo(other),
      d(new AdiumThemeContentInfoPrivate(*other.d))
{
}

AdiumThemeContentInfo::~AdiumThemeContentInfo()
{
    delete d;
}

AdiumThemeContentInfo& AdiumThemeContentInfo::operator=(const AdiumThemeContentInfo& other)
{
    AdiumThemeMessageInfo::operator =(other);
    *d = *other.d;
    return *this;
}


QString AdiumThemeContentInfo::userIconPath() const
{
    return d->userIconPath;
}

void AdiumThemeContentInfo::setUserIconPath(const QString &userIconPath)
{
    d->userIconPath = userIconPath;
}

QString AdiumThemeContentInfo::senderScreenName() const
{
    return d->senderScreenName;
}

void AdiumThemeContentInfo::setSenderScreenName(const QString & senderScreenName)
{
    d->senderScreenName = senderScreenName;
}

QString AdiumThemeContentInfo::sender() const
{
    return d->senderDisplayName;
}

QString AdiumThemeContentInfo::senderColor() const
{
    return d->senderColor;
}

void AdiumThemeContentInfo::setSenderColor(const QString &senderColor)
{
    d->senderColor = senderColor;
}

QString AdiumThemeContentInfo::senderStatusIcon() const
{
    return d->senderStatusIcon;
}

void AdiumThemeContentInfo::setSenderStatusIcon(const QString &senderStatusIcon)
{
    d->senderStatusIcon = senderStatusIcon;
}

QString AdiumThemeContentInfo::messageDirection() const
{
    switch(type()) {
    case AdiumThemeMessageInfo::RemoteToLocal:
        return QLatin1String("trl");
    default:
        return QLatin1String("ltr");
    }
}

QString AdiumThemeContentInfo::senderDisplayName() const
{
    return d->senderDisplayName;
}

void AdiumThemeContentInfo::setSenderDisplayName(const QString &senderDisplayName)
{
    d->senderDisplayName = senderDisplayName;
}
