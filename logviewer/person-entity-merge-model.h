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

#ifndef PERSONENTITYMERGEMODEL_H
#define PERSONENTITYMERGEMODEL_H

#include <QtCore/QAbstractItemModel>

#include <TelepathyQt/Types>
#include <TelepathyLoggerQt4/Types>

#include <kpeople/persons-model.h>

class EntityModel;

namespace Tpl {
class PendingOperation;
}

class PersonEntityMergeModel : public QAbstractItemModel
{
    Q_OBJECT

  public:
    enum Roles {
        EntityRole = PersonsModel::LastRole + 1
    };

    explicit PersonEntityMergeModel(PersonsModel *personsModel, EntityModel *entityModel,
                                    QObject *parent);
    virtual ~PersonEntityMergeModel();

    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex& index, int role) const;
    virtual QModelIndex parent(const QModelIndex& child) const;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;

    void setSearchHits(const Tpl::SearchHitList &searchHits);
    void clearSearchHits();

  private Q_SLOTS:
    void initializeModel();

  private:
    class Item;
    class ContactItem;
    class GroupItem;

    GroupItem* groupForName(const QVariant &name);
    ContactItem* itemForPersona(const QModelIndex &personsModel_personaIndex);
    Item* itemForIndex(const QModelIndex &index) const;

    PersonsModel *m_personsModel;
    EntityModel *m_entityModel;

    ContactItem *m_rootItem;
};

#endif // PERSONENTITYMERGEMODEL_H




