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


#ifndef ADIUMTHEMESTATUSINFO_H
#define ADIUMTHEMESTATUSINFO_H

#include "adium-theme-message-info.h"
#include "ktpchat_export.h"

class QString;
class AdiumThemeStatusInfoPrivate;

class KDE_TELEPATHY_CHAT_EXPORT AdiumThemeStatusInfo : public AdiumThemeMessageInfo
{
public:
    explicit AdiumThemeStatusInfo();
    explicit AdiumThemeStatusInfo(const AdiumThemeStatusInfo &other);
    virtual ~AdiumThemeStatusInfo();
    AdiumThemeStatusInfo &operator=(const AdiumThemeStatusInfo &other);

    /** A description of the status event. This is neither in
     * the user's local language nor expected to be displayed */
    QString status() const;
    void setStatus(const QString& status);

private:
    AdiumThemeStatusInfoPrivate *d;
};

#endif // ADIUMTHEMESTATUSINFO_H
