/*
 * Copyright (C) 2013  Daniel Vrátil <dvratil@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef DATESMODEL_H
#define DATESMODEL_H

#include <QtCore/QAbstractItemModel>

#include <TelepathyQt/Types>
#include <TelepathyLoggerQt4/Types>

namespace Tpl {
class PendingOperation;
}

class DatesModel : public QAbstractItemModel
{
    Q_OBJECT

  public:
    enum Roles {
        TypeRole = Qt::UserRole + 1,
        DateRole,
        HintRole,
        AccountRole,
        EntityRole
    };

    enum RowTypes {
        GroupRow,
        DateRow,
        ConversationRow
    };

    explicit DatesModel(QObject* parent = 0);
    virtual ~DatesModel();

    void addEntity(const Tp::AccountPtr &account, const Tpl::EntityPtr &entity);
    void setEntity(const Tp::AccountPtr &account, const Tpl::EntityPtr &entity);

    void setSearchHits(const Tpl::SearchHitList &searchHits);
    void clearSearchHits();

    void clear();

    QDate previousDate(const QModelIndex &index) const;
    QDate nextDate(const QModelIndex &index) const;
    QModelIndex indexForDate(const QDate &date) const;

    virtual QVariant data(const QModelIndex& index, int role) const;
    virtual int columnCount(const QModelIndex& parent) const;
    virtual int rowCount(const QModelIndex& parent) const;
    virtual QModelIndex parent(const QModelIndex& child) const;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent) const;

  private Q_SLOTS:
    void onDatesReceived(Tpl::PendingOperation *operation);

  private:
    typedef QPair<Tp::AccountPtr, Tpl::EntityPtr> AccountEntityPair;
    QList<AccountEntityPair> m_pairs;
    Tpl::SearchHitList m_searchHits;
    int m_resetInProgress;

    class Date;
    QMultiMap<QDate, Date*> m_items;

    friend bool compareDates(const Date *date1, const Date *date2);
};

#endif // DATESMODEL_H
