/***************************************************************************
 *   Copyright (C) 2013 by Stefan Eggers <coloncolonone@gmail.com>         *
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

#ifndef TEXT_CHAT_CONFIG_H
#define TEXT_CHAT_CONFIG_H

#include <QtCore/QMutex>
#include <QtCore/QObject>

#include "shareprovider.h"

#include "ktpchat_export.h"

class TextChatConfigPrivate;

class KDE_TELEPATHY_CHAT_EXPORT TextChatConfig : QObject
{
    Q_OBJECT

  public:
    enum TabOpenMode {
        NewWindow,
        FirstWindow
    };

    // settings get loaded when instance gets created
    static TextChatConfig *instance();

    // write out current settings to file
    void sync();

    TabOpenMode openMode();
    void setOpenMode(TabOpenMode mode);

    int scrollbackLength();
    void setScrollbackLength(int length);

    bool showMeTyping();
    void setShowMeTyping(bool showTyping);

    bool showOthersTyping();
    void setShowOthersTyping(bool showTyping);

    QString nicknameCompletionSuffix() const;
    void setNicknameCompletionSuffix(const QString &suffix);

    ShareProvider::ShareService imageShareServiceType() const;
    void setImageShareServiceName(ShareProvider::ShareService serviceType);

    bool dontLeaveGroupChats() const;
    void setDontLeaveGroupChats(bool dontLeaveGroupChats);

private:
    TextChatConfig();

    static QMutex mutex;

    const QScopedPointer<TextChatConfigPrivate> d;
};

#endif // TEXT_CHAT_CONFIG_H
