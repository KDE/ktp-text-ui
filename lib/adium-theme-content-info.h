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


#ifndef ADIUMTHEMECONTENTINFO_H
#define ADIUMTHEMECONTENTINFO_H

#include "adium-theme-message-info.h"

#include "kdetelepathychat_export.h"

class QString;
class AdiumThemeContentInfoPrivate;

class KDE_TELEPATHY_CHAT_EXPORT AdiumThemeContentInfo : public AdiumThemeMessageInfo
{
public:
    explicit AdiumThemeContentInfo(AdiumThemeMessageInfo::MessageType);
    explicit AdiumThemeContentInfo(const AdiumThemeContentInfo &other);
    ~AdiumThemeContentInfo();
    AdiumThemeContentInfo &operator=(const AdiumThemeContentInfo &other);


    /** Path to the user icon associated with this message */
    QString userIconPath() const;
    void setUserIconPath(const QString& userIconPath);

    /** The screen name (UID, ID, member name, etc.) of the sender of this message.*/
    QString senderScreenName() const;
    void setSenderScreenName(const QString & senderScreenName);

    /** The name of the sender of this message as the user's preferences request it.*/
    QString sender() const;

    /** A color derived from the user's name*/
    //FIXME what is this talking about...?
    QString senderColor() const;
    void setSenderColor(const QString& senderColor);

    /** The path to the status icon of the sender (available, away, etc...) */
    QString senderStatusIcon() const;
    void setSenderStatusIcon(const QString& senderStatusIcon);

    /** The text direction of the message (either rtl or ltr) */
    QString messageDirection() const;

    /** The serverside (remotely set) name of the sender, such as an MSN display name.*/
    QString senderDisplayName() const;
    void setSenderDisplayName(const QString& senderDisplayName);

    //textBackgroundColor

private:
    AdiumThemeContentInfoPrivate* d;
};

#endif // ADIUMTHEMECONTENTINFO_H
