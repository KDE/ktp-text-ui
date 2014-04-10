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
#include <QStringList>
#include <QHash>


// List of colors used by %senderColor%. Copied from
// adium/Frameworks/AIUtilities\ Framework/Source/AIColorAdditions.m
static const QString defaultColors(QLatin1String("aqua:aquamarine:blue:"
    "blueviolet:brown:burlywood:cadetblue:chartreuse:chocolate:coral:"
    "cornflowerblue:crimson:cyan:darkblue:darkcyan:darkgoldenrod:darkgreen:"
    "darkgrey:darkkhaki:darkmagenta:darkolivegreen:darkorange:darkorchid:"
    "darkred:darksalmon:darkseagreen:darkslateblue:darkslategrey:darkturquoise:"
    "darkviolet:deeppink:deepskyblue:dimgrey:dodgerblue:firebrick:forestgreen:"
    "fuchsia:gold:goldenrod:green:greenyellow:grey:hotpink:indianred:indigo:"
    "lawngreen:lightblue:lightcoral:lightgreen:lightgrey:lightpink:lightsalmon:"
    "lightseagreen:lightskyblue:lightslategrey:lightsteelblue:lime:limegreen:"
    "magenta:maroon:mediumaquamarine:mediumblue:mediumorchid:mediumpurple:"
    "mediumseagreen:mediumslateblue:mediumspringgreen:mediumturquoise:"
    "mediumvioletred:midnightblue:navy:olive:olivedrab:orange:orangered:orchid:"
    "palegreen:paleturquoise:palevioletred:peru:pink:plum:powderblue:purple:"
    "red:rosybrown:royalblue:saddlebrown:salmon:sandybrown:seagreen:sienna:"
    "silver:skyblue:slateblue:slategrey:springgreen:steelblue:tan:teal:thistle:"
    "tomato:turquoise:violet:yellowgreen"));

static const QStringList defaultColorList(defaultColors.split(QLatin1Char(':')));


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

QString AdiumThemeContentInfo::senderDisplayName() const
{
    return d->senderDisplayName;
}

void AdiumThemeContentInfo::setSenderDisplayName(const QString &senderDisplayName)
{
    d->senderDisplayName = senderDisplayName;
    // FIXME Themes can have a SenderColors.txt file to specify which colors to
    //       use instead of the default ones.
    d->senderColor = defaultColorList.at(qHash(senderDisplayName) % defaultColorList.size());
}
