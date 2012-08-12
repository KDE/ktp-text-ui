/*
 *    Copyright (C) 2012  Lasath Fernando <kde@lasath.org>
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 2.1 of the License, or (at your option) any later version.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "substitution-prefs.h"

#include <KSharedConfig>
#include <KGlobal>

class SubstitutionPrefs::Private {
public:
     SubstitutionPrefs::List wordList;
};

SubstitutionPrefs::SubstitutionPrefs()
    : d(new Private)
{
    load();
}

SubstitutionPrefs::~SubstitutionPrefs()
{
    delete d;
}

void SubstitutionPrefs::load()
{
    KSharedConfig::Ptr config = KGlobal::config();
    d->wordList = config->entryMap(QLatin1String("replacements"));

    if (d->wordList.isEmpty()) {
        d->wordList = defaultList();
    }
}

void SubstitutionPrefs::save()
{
}

void SubstitutionPrefs::defaults()
{
    d->wordList = defaultList();
}

SubstitutionPrefs::List SubstitutionPrefs::defaultList()
{
    List def;

    def[QLatin1String("(TM) ")] = QLatin1String("™ ");
    def[QLatin1String(" uber ")] = QLatin1String(" über ");

    //probably not particularly useful without i18n
    def[QLatin1String(" dnt ")] = QLatin1String(" don't ");
    def[QLatin1String(" cnt ")] = QLatin1String(" can't ");

    return def;
}

QString SubstitutionPrefs::replacementFor(const QString &word)
{
    return d->wordList[word];
}

QStringList SubstitutionPrefs::wordsToReplace()
{
    return d->wordList.keys();
}

int SubstitutionPrefs::columnCount(const QModelIndex &parent) const
{
    return 2;
}

int SubstitutionPrefs::rowCount(const QModelIndex &parent) const
{
    return d->wordList.size();
}

QVariant SubstitutionPrefs::data(const QModelIndex &index, int role) const
{
    return QVariant(QLatin1String("boo"));
}
