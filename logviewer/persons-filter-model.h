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

#ifndef PERSONSFILTERMODEL_H
#define PERSONSFILTERMODEL_H

#include <QtGui/QSortFilterProxyModel>

#include <TelepathyQt/Types>
#include <TelepathyLoggerQt4/Types>
#include <kpeople/persons-model.h>

namespace Tpl {
class PendingOperation;
}

class PersonsFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

  public:
    enum Roles {
        EntityRole = PersonsModel::LastRole + 1
    };

    explicit PersonsFilterModel(QObject *parent);
    virtual ~PersonsFilterModel();

    void setAccountManager(const Tp::AccountManagerPtr &accountManager);

    void setSearchHits(const Tpl::SearchHitList &searchHits);
    Tpl::SearchHitList searchHits() const;
    void clearSearchHits();

    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
    virtual QVariant data(const QModelIndex& index, int role) const;

  protected:
    virtual bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const;

  private Q_SLOTS:
    void slotEntitiesReceived(Tpl::PendingOperation *operation);

  private:
    Tpl::EntityPtr findEntity(const Tp::AccountPtr &account, const QString &contactUid) const;

    QMap<Tp::AccountPtr, Tpl::EntityPtr> m_entities;
    Tpl::SearchHitList m_searchHits;
};

#endif // PERSONSFILTERMODEL_H

