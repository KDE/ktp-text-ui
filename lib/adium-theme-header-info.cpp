/***************************************************************************
 *   Copyright (C) 2010 by David Edmundson <kde@davidedmundson.co.uk>      *
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

#include "adium-theme-header-info.h"

//FIXME HTML escaping is maybe needed.

class AdiumThemeHeaderInfoPrivate
{
public:
    explicit AdiumThemeHeaderInfoPrivate();

    QString chatName;
    QString sourceName;
    QString destinationName;
    QString destinationDisplayName;
    QUrl incomingIconPath;
    QUrl outgoingIconPath;
    QDateTime timeOpened;
    QString serviceIconPath;
    bool isGroupChat;
};

AdiumThemeHeaderInfoPrivate::AdiumThemeHeaderInfoPrivate():
    isGroupChat(false)
{
}

AdiumThemeHeaderInfo::AdiumThemeHeaderInfo()
    : d(new AdiumThemeHeaderInfoPrivate)
{
}

AdiumThemeHeaderInfo::AdiumThemeHeaderInfo(const AdiumThemeHeaderInfo &other)
    : d(new AdiumThemeHeaderInfoPrivate(*other.d))
{

}

AdiumThemeHeaderInfo::~AdiumThemeHeaderInfo()
{
    delete d;
}

AdiumThemeHeaderInfo& AdiumThemeHeaderInfo::operator=(const AdiumThemeHeaderInfo& other)
{
    *d = *other.d;
    return *this;
}


QString AdiumThemeHeaderInfo::chatName() const
{
    return d->chatName;
};

void AdiumThemeHeaderInfo::setChatName(const QString& chatName)
{
    d->chatName = chatName;
};

QString AdiumThemeHeaderInfo::sourceName() const
{
    return d->sourceName;
};

void AdiumThemeHeaderInfo::setSourceName(const QString& sourceName)
{
    d->sourceName = sourceName;
};

QString AdiumThemeHeaderInfo::destinationName() const
{
    return d->destinationName;
}
void AdiumThemeHeaderInfo::setDestinationName(const QString& destinationName)
{
    d->destinationName = destinationName;
};

QString AdiumThemeHeaderInfo::destinationDisplayName() const
{
    return d->destinationDisplayName;
}
void AdiumThemeHeaderInfo::setDestinationDisplayName(const QString& destinationDisplayName)
{
    d->destinationDisplayName = destinationDisplayName;
}

QUrl AdiumThemeHeaderInfo::incomingIconPath() const
{
    return d->incomingIconPath;
};

void AdiumThemeHeaderInfo::setIncomingIconPath(const QUrl& incomingIconPath)
{
    d->incomingIconPath = incomingIconPath;
};

QUrl AdiumThemeHeaderInfo::outgoingIconPath() const
{
    return d->outgoingIconPath;
};

void AdiumThemeHeaderInfo::setOutgoingIconPath(const QUrl& outgoingIconPath)
{
    d->outgoingIconPath = outgoingIconPath;
};

QDateTime AdiumThemeHeaderInfo::timeOpened() const
{
    return d->timeOpened;
};

void AdiumThemeHeaderInfo::setTimeOpened(const QDateTime& timeOpened)
{
    d->timeOpened = timeOpened;
};

QString AdiumThemeHeaderInfo::serviceIconPath() const
{
    return d->serviceIconPath;
}

void AdiumThemeHeaderInfo::setServiceIconPath(const QString& serviceIconPath)
{
    d->serviceIconPath = serviceIconPath;
}

bool AdiumThemeHeaderInfo::isGroupChat() const
{
    return d->isGroupChat;
}

void AdiumThemeHeaderInfo::setGroupChat(bool isGroupChat)
{
    d->isGroupChat = isGroupChat;
}
