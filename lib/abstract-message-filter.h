/*
    Copyright (C) 2012  Lasath Fernando <kde@lasath.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef ABSTRACTPLUGIN_H
#define ABSTRACTPLUGIN_H

#include "message.h"
#include "ktpchat_export.h"
#include <KConfigGroup>

class KDE_TELEPATHY_CHAT_EXPORT AbstractMessageFilter : public QObject
{
Q_OBJECT
public:
    AbstractMessageFilter(QObject* parent = 0);
    virtual ~AbstractMessageFilter();

    /** Filter messages to show on the UI recieved by another contact*/
    virtual void filterIncomingMessage(Message &message);

    /** Filter messages to show in the UI that you have sent
        This does _not_ affect the actual message sent, only the visual representation on your screen.
    */
    virtual void filterOutgoingMessage(Message &message);

    /** Filter messages in either direction. Base implementation calls this for messages sent/recived in either direction.*/
    virtual void filterMessage(Message &message);

    /** Scripts that must be included in the <head> section of the html required by this message filter.*/
    virtual QStringList requiredScripts();

    /** Scripts that must be included in the <head> section of the html required by this message filter.*/
    virtual QStringList requiredStylesheets();

protected:
    KConfigGroup config();
};

#endif // ABSTRACTPLUGIN_H

