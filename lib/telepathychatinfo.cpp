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

#include "telepathychatinfo.h"

//FIXME HTML escaping is maybe needed.

class TelepathyChatInfoPrivate
{
public:
    QString chatName;
    QString sourceName;
    QString destinationName;
    QString destinationDisplayName;
    QUrl incomingIconPath;
    QUrl outgoingIconPath;
    QDateTime timeOpened;
};

TelepathyChatInfo::TelepathyChatInfo()
        : d(new TelepathyChatInfoPrivate)
{
}

TelepathyChatInfo::~TelepathyChatInfo()
{
    delete d;
}

QString TelepathyChatInfo::chatName() const
{
    return d->chatName;
};

void TelepathyChatInfo::setChatName(const QString& chatName)
{
    d->chatName = chatName;
};

QString TelepathyChatInfo::sourceName() const
{
    return d->sourceName;
};

void TelepathyChatInfo::setSourceName(const QString& sourceName)
{
    d->sourceName = sourceName;
};

QString TelepathyChatInfo::destinationName() const
{
    return d->destinationName;
}
void TelepathyChatInfo::setDestinationName(const QString& destinationName)
{
    d->destinationName = destinationName;
};

QString TelepathyChatInfo::destinationDisplayName() const
{
    return d->destinationDisplayName;
}
void TelepathyChatInfo::setDestinationDisplayName(const QString& destinationDisplayName)
{
    d->destinationDisplayName = destinationDisplayName;
}

QUrl TelepathyChatInfo::incomingIconPath() const
{
    return d->incomingIconPath;
};

void TelepathyChatInfo::setIncomingIconPath(const QUrl& incomingIconPath)
{
    d->incomingIconPath = incomingIconPath;
};

QUrl TelepathyChatInfo::outgoingIconPath() const
{
    return d->outgoingIconPath;
};

void TelepathyChatInfo::setOutgoingIconPath(const QUrl& outgoingIconPath)
{
    d->outgoingIconPath = outgoingIconPath;
};

QDateTime TelepathyChatInfo::timeOpened() const
{
    return d->timeOpened;
};

void TelepathyChatInfo::setTimeOpened(const QDateTime& timeOpened)
{
    d->timeOpened = timeOpened;
};
