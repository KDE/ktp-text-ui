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

#include "adium-theme-message-info.h"
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QDateTime>


class AdiumThemeMessageInfoPrivate
{
public:
    QString message;
    QDateTime time;
    QString service;
    QStringList messageClasses;
    AdiumThemeMessageInfo::MessageType type;
    QString script;
};

AdiumThemeMessageInfo::AdiumThemeMessageInfo()
    : d(new AdiumThemeMessageInfoPrivate())
{
    d->type = Invalid;
}


AdiumThemeMessageInfo::AdiumThemeMessageInfo(MessageType type)
    : d(new AdiumThemeMessageInfoPrivate())
{
    d->type = type;
}

AdiumThemeMessageInfo::AdiumThemeMessageInfo(const AdiumThemeMessageInfo &other)
    : d(new AdiumThemeMessageInfoPrivate(*other.d))
{

}

AdiumThemeMessageInfo::~AdiumThemeMessageInfo()
{
    delete d;
}

AdiumThemeMessageInfo& AdiumThemeMessageInfo::operator=(const AdiumThemeMessageInfo& other)
{
    *d = *other.d;
    return *this;
}

AdiumThemeMessageInfo::MessageType AdiumThemeMessageInfo::type() const
{
    return d->type;
}

QString AdiumThemeMessageInfo::message() const
{
    return d->message;
}

void AdiumThemeMessageInfo::setMessage(const QString& message)
{
    d->message = message;
}

QDateTime AdiumThemeMessageInfo::time() const
{
    return d->time;
}

void AdiumThemeMessageInfo::setTime(const QDateTime& time)
{
    d->time = time;
}

QString AdiumThemeMessageInfo::service() const
{
    return d->service;
}

void AdiumThemeMessageInfo::setService(const QString& service)
{
    d->service = service;
}

QString AdiumThemeMessageInfo::userIcons() const
{
    //FIXME.
    return QLatin1String("showIcons");
}

QString AdiumThemeMessageInfo::messageClasses() const {
    //in the future this will also contain history, consecutive, autoreply, status, event
    //these will be stored internally as flags

    QStringList classes;

    if (d->type == RemoteToLocal) {
        classes.append(QLatin1String("incoming"));
        classes.append(QLatin1String("message"));
    }

    if (d->type == LocalToRemote) {
        classes.append(QLatin1String("outgoing"));
        classes.append(QLatin1String("message"));
    }

    if (d->type == Status) {
        classes.append(QLatin1String("status"));
    }

    if (d->type == HistoryLocalToRemote) {
        classes.append(QLatin1String("history"));
        classes.append(QLatin1String("incoming"));
        classes.append(QLatin1String("message"));
    }

    if (d->type == HistoryRemoteToLocal) {
        classes.append(QLatin1String("history"));
        classes.append(QLatin1String("outgoing"));
        classes.append(QLatin1String("message"));
    }

    if (d->type == HistoryStatus) {
        classes.append(QLatin1String("history"));
        classes.append(QLatin1String("status"));
    }

    classes << d->messageClasses;

    return classes.join(QLatin1String(" "));
}

QString AdiumThemeMessageInfo::messageDirection() const
{
    if(message().isRightToLeft()) {
        return QLatin1String("rtl");
    } else {
        return QLatin1String("ltr");
    }
}

void AdiumThemeMessageInfo::appendMessageClass(const QString &messageClass)
{
    d->messageClasses.append(messageClass);
}

QString AdiumThemeMessageInfo::script() const
{
    return d->script;
}

void AdiumThemeMessageInfo::setScript(const QString& script)
{
    d->script = script;
}
