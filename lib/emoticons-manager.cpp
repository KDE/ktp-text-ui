/*
 * Emoticons Manager
 *
 * Copyright (C) 2015 David Rosca <nowrep@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "emoticons-manager.h"

#include <KEmoticons>
#include <KConfigGroup>
#include <KSharedConfig>

class EmoticonsManagerPrivate
{
public:
    EmoticonsManagerPrivate()
    {
        config = KSharedConfig::openConfig(QStringLiteral("ktp-text-uirc"));
    }

    KEmoticons emoticons;
    KSharedConfig::Ptr config;
    QHash<QString, KEmoticonsTheme> themeHash;
};

Q_GLOBAL_STATIC(EmoticonsManagerPrivate, sPrivate)

// static
KEmoticonsTheme EmoticonsManager::themeForAccount(const Tp::AccountPtr &account)
{
    const QString id = account->uniqueIdentifier();

    if (!sPrivate->themeHash.contains(id)) {
        KConfigGroup group = sPrivate->config->group("Filter-Emoticons");
        QString themeName = group.readEntry<QString>(id, QString());

        if (themeName.isEmpty()) {
            themeName = sPrivate->emoticons.currentThemeName();
        }

        sPrivate->themeHash.insert(id, sPrivate->emoticons.theme(themeName));
    }

    return sPrivate->themeHash.value(id);
}
