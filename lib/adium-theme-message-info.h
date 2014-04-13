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

#ifndef ADIUMTHEMEMESSAGEINFO_H
#define ADIUMTHEMEMESSAGEINFO_H

#include "ktpchat_export.h"

class QString;
class QDateTime;
class AdiumThemeMessageInfoPrivate;

class KDE_TELEPATHY_CHAT_EXPORT AdiumThemeMessageInfo
{
public:
    enum MessageType {
        RemoteToLocal,
        LocalToRemote,
        Status,
        HistoryRemoteToLocal,
        HistoryLocalToRemote,
        HistoryStatus,
        Invalid
    };

    explicit AdiumThemeMessageInfo();
    explicit AdiumThemeMessageInfo(MessageType);
    explicit AdiumThemeMessageInfo(const AdiumThemeMessageInfo &other);
    virtual ~AdiumThemeMessageInfo();
    AdiumThemeMessageInfo &operator=(const AdiumThemeMessageInfo &other);

    MessageType type() const;

    /** The message itself of the message/status. */
    QString message() const;
    void setMessage(const QString& message);

    /** The time at which message/status occurred*/
    QDateTime time() const;
    void setTime(const QDateTime& time);

    /** The sender of the message */
    QString sender() const;
    void setSender(const QString& sender);

    QString service() const;
    void setService(const QString& service);

    /** Will be replaced with "showIcons" if the "Show user icons" checkbox is selected,*/
    //FIXME in here or in AdiumThemeView..?
    QString userIcons() const;

    QString messageClasses() const;
    void appendMessageClass(const QString& messageClass);

    /** The text direction of the message (either rtl or ltr) */
    QString messageDirection() const;

    /** The script to be run after appending the message. */
    QString script() const;
    void setScript(const QString& script);
private:
    AdiumThemeMessageInfoPrivate *d;
};

#endif // ADIUMTHEMEMESSAGEINFO_H
