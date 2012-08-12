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
#include <KConfigGroup>
#include <KDebug>
#include <QBuffer>

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

KConfigGroup SubstitutionPrefs::config()
{
    return KSharedConfig::openConfig(QLatin1String("ktelepathyrc"))->group("Replacements");
}

void SubstitutionPrefs::load()
{
    beginResetModel();
    d->wordList = config().entryMap();

    if (d->wordList.isEmpty()) {
        d->wordList = defaultList();
    }
    endResetModel();

    kDebug() << "loaded" << d->wordList;
}

void SubstitutionPrefs::save()
{
    kDebug() << "writing" << d->wordList;

    KConfigGroup cfg = config();
    cfg.deleteGroup();

    Q_FOREACH (const QString &word, d->wordList.keys()) {
        cfg.writeEntry(word, d->wordList[word]);
    }
    cfg.sync();
}

void SubstitutionPrefs::defaults()
{
    beginResetModel();
        d->wordList = defaultList();
    endResetModel();
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

QString SubstitutionPrefs::replacementFor(const QString &word) const
{
    return d->wordList[word];
}

QStringList SubstitutionPrefs::wordsToReplace() const
{
    return d->wordList.keys();
}

QVariant SubstitutionPrefs::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        switch (section) {
            case WORD_COLUMN :
                return QLatin1String("Word");
            case REPLACEMENT_COLUMN :
                return QLatin1String("Replacement");
        }
    }
    return QVariant();
}

int SubstitutionPrefs::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 2;
}

int SubstitutionPrefs::rowCount(const QModelIndex &parent) const
{
    return d->wordList.size();
}

QVariant SubstitutionPrefs::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case WORD_COLUMN :
                return wordsToReplace().at(index.row());
            case REPLACEMENT_COLUMN :
                return replacementFor(wordsToReplace().at(index.row()));
            default:
                Q_ASSERT(false);
        }
    }

    return QVariant();
}

void SubstitutionPrefs::addReplacement(const QString &word, const QString &replacement)
{
    //since it's a hashmap, I have no idea where it'll put it
    beginResetModel();
    d->wordList[word] = replacement;
    endResetModel();
}

void SubstitutionPrefs::removeWord(const QString &word)
{
    beginResetModel();
    d->wordList.remove(word);
    endResetModel();
}
