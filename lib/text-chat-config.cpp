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


QMutex TextChatConfig::mutex(QMutex::Recursive);


class TextChatConfigPrivate
{
public:
    TextChatConfig::TabOpenMode m_openMode;
    int m_scrollbackLength;
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


TextChatConfig::TextChatConfig() :
    d(new TextChatConfigPrivate())
{
    // only ever called from TextChatConfig::instance() while mutex is locked
    // thus no own mutex use inside the constructor

    KSharedConfigPtr config = KSharedConfig::openConfig(QLatin1String("ktelepathyrc"));
    KConfigGroup behaviorConfig = config->group("Behavior");

    QString mode = behaviorConfig.readEntry("tabOpenMode", "NewWindow");
    if(mode == QLatin1String("NewWindow")) {
        d->m_openMode = TextChatConfig::NewWindow;
    } else {
        d->m_openMode = TextChatConfig::FirstWindow;
    }

    d->m_scrollbackLength = behaviorConfig.readEntry("scrollbackLength", 4);
}


#include "text-chat-config.moc"
