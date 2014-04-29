/***************************************************************************
 *   Copyright (C) 2011 by Lasath Fernando <kde@lasath.org>                *
 *   Copyright (C) 2011 by David Edmundson <kde@davidedmundson.co.uk>      *
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


#include <KConfigGroup>
#include <KSharedConfig>

#include "text-chat-config.h"

#include "shareprovider.h"

QMutex TextChatConfig::mutex(QMutex::Recursive);


class TextChatConfigPrivate
{
public:
    TextChatConfig::TabOpenMode m_openMode;
    int m_scrollbackLength;
    bool m_showMeTyping;
    bool m_showOthersTyping;
    bool m_dontLeaveGroupChats;
    QString m_nicknameCompletionSuffix;
    ShareProvider::ShareService m_imageShareServiceType;
};


TextChatConfig *TextChatConfig::instance()
{
    static TextChatConfig *instance = 0;

    mutex.lock();

    if (!instance) {
        instance = new TextChatConfig();
    }

    mutex.unlock();

    // if instance was already setup the mutex protected block won't have
    // changed it; if we were the first and instance was still 0 we set it up
    // inside the mutex protected block and instance won't ever change again;
    // thus this is allowed to be outside the mutex protected block
    return instance;
}


void TextChatConfig::sync()
{
    mutex.lock();

    KSharedConfigPtr config = KSharedConfig::openConfig(QLatin1String("ktelepathyrc"));
    KConfigGroup behaviorConfig = config->group("Behavior");

    QString mode;
    if (d->m_openMode == TextChatConfig::NewWindow) {
        mode = QLatin1String("NewWindow");
    } else {
        mode = QLatin1String("FirstWindow");
    }
    behaviorConfig.writeEntry("tabOpenMode", mode);

    behaviorConfig.writeEntry("scrollbackLength", d->m_scrollbackLength);

    behaviorConfig.writeEntry("showMeTyping", d->m_showMeTyping);

    behaviorConfig.writeEntry("showOthersTyping", d->m_showOthersTyping);

    behaviorConfig.writeEntry("nicknameCompletionSuffix", d->m_nicknameCompletionSuffix);

    behaviorConfig.writeEntry("imageShareServiceType", static_cast<int>(d->m_imageShareServiceType));

    behaviorConfig.writeEntry("dontLeaveGroupChats", d->m_dontLeaveGroupChats);

    behaviorConfig.sync();

    mutex.unlock();
}


TextChatConfig::TabOpenMode TextChatConfig::openMode()
{
    TextChatConfig::TabOpenMode result;

    mutex.lock();
    result = d->m_openMode;
    mutex.unlock();

    return result;
}


void TextChatConfig::setOpenMode(TextChatConfig::TabOpenMode mode)
{
    mutex.lock();
    d->m_openMode = mode;
    mutex.unlock();
}


int TextChatConfig::scrollbackLength()
{
    int result;

    mutex.lock();
    result = d->m_scrollbackLength;
    mutex.unlock();

    return result;
}


void TextChatConfig::setScrollbackLength(int length)
{
    mutex.lock();
    d->m_scrollbackLength = length;
    mutex.unlock();
}


bool TextChatConfig::showMeTyping()
{
    bool result;

    mutex.lock();
    result = d->m_showMeTyping;
    mutex.unlock();

    return result;
}


void TextChatConfig::setShowMeTyping(bool showTyping)
{
    mutex.lock();
    d->m_showMeTyping = showTyping;
    mutex.unlock();
}


bool TextChatConfig::showOthersTyping()
{
    bool result;

    mutex.lock();
    result = d->m_showOthersTyping;
    mutex.unlock();

    return result;
}


void TextChatConfig::setShowOthersTyping(bool showTyping)
{
    mutex.lock();
    d->m_showOthersTyping = showTyping;
    mutex.unlock();
}


QString TextChatConfig::nicknameCompletionSuffix() const {
    return d->m_nicknameCompletionSuffix;
}

void TextChatConfig::setNicknameCompletionSuffix(const QString &suffix) {
    mutex.lock();
    d->m_nicknameCompletionSuffix = suffix;
    mutex.unlock();
}

ShareProvider::ShareService TextChatConfig::imageShareServiceType() const
{
    return d->m_imageShareServiceType;
}

void TextChatConfig::setImageShareServiceName(ShareProvider::ShareService serviceType)
{
    mutex.lock();
    d->m_imageShareServiceType = serviceType;
    mutex.unlock();
}


bool TextChatConfig::dontLeaveGroupChats() const
{
    return d->m_dontLeaveGroupChats;
}

void TextChatConfig::setDontLeaveGroupChats(bool dontLeaveGroupChats)
{
    mutex.lock();
    d->m_dontLeaveGroupChats = dontLeaveGroupChats;
    mutex.unlock();
}

TextChatConfig::TextChatConfig() :
    d(new TextChatConfigPrivate())
{
    // only ever called from TextChatConfig::instance() while mutex is locked
    // thus no own mutex use inside the constructor

    KSharedConfigPtr config = KSharedConfig::openConfig(QLatin1String("ktelepathyrc"));
    KConfigGroup behaviorConfig = config->group("Behavior");

    QString mode = behaviorConfig.readEntry("tabOpenMode", "FirstWindow");
    if(mode == QLatin1String("NewWindow")) {
        d->m_openMode = TextChatConfig::NewWindow;
    } else {
        d->m_openMode = TextChatConfig::FirstWindow;
    }

    d->m_scrollbackLength = behaviorConfig.readEntry("scrollbackLength", 4);

    d->m_showMeTyping = behaviorConfig.readEntry("showMeTyping", true);

    d->m_showOthersTyping = behaviorConfig.readEntry("showOthersTyping", true);

    d->m_nicknameCompletionSuffix = behaviorConfig.readEntry("nicknameCompletionSuffix", ", ");

    d->m_dontLeaveGroupChats = behaviorConfig.readEntry("dontLeaveGroupChats", false);

    // Imgur is the default image sharing service
    int shareServiceType = behaviorConfig.readEntry("imageShareServiceType", static_cast<int>(ShareProvider::Imgur));
    d->m_imageShareServiceType = static_cast<ShareProvider::ShareService>(shareServiceType);
}


#include "text-chat-config.moc"
