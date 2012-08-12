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

#ifndef SUBSTITUTION_PREFS_H
#define SUBSTITUTION_PREFS_H

#include <QMap>
#include <QStringList>
#include <QAbstractItemModel>

class SubstitutionPrefs :
    public QAbstractTableModel
{

public:
    SubstitutionPrefs();
    virtual ~SubstitutionPrefs();

    QString replacementFor(const QString &word) const;
    QStringList wordsToReplace() const;

    typedef QMap<QString, QString> List;
    List defaultList();

    void load();
    void save();
    void defaults();

    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    void addReplacement(const QString &word, const QString &replacement);
    void removeWord(const QString &word);

protected:
    enum Column {
        WORD_COLUMN = 0,
        REPLACEMENT_COLUMN
    };

private:
    class Private;
    Private *d;
};

#endif // SUBSTITUTION_PREFS_H
