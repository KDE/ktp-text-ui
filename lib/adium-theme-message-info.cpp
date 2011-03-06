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
#include <QtGui/QTextDocument> //needed for Qt::escape


class AdiumThemeMessageInfoPrivate
{
public:
    QString message;
    QDateTime time;
    QString service;
    QStringList messageClasses;
    AdiumThemeMessageInfo::MessageType type;
};

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
    QString htmlMessage= Qt::escape(d->message);
    htmlMessage.replace('\n', "<br/>");
    htmlMessage.replace('\\', "\\\\"); //replace a single backslash with two backslashes.

    return htmlMessage;
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
    return QString("showIcons");
}

QString AdiumThemeMessageInfo::messageClasses() const {
    //in the future this will also contain history, consecutive, autoreply, status, event
    //these will be stored internally as flags

    QStringList classes = d->messageClasses;

    if (d->type == RemoteToLocal) {
        classes.append("incoming");
        classes.append("message");
    }

    if (d->type == LocalToRemote) {
        classes.append("outgoing");
        classes.append("message");
    }

    if (d->type == Status) {
        classes.append("status");
    }

    return classes.join(" ");
}


void AdiumThemeMessageInfo::appendMessageClass(const QString &messageClass)
{
    d->messageClasses.append(messageClass);
}
