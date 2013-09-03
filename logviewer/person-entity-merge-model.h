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

#include <KTp/Models/contacts-model.h>

class EntityModel;

class PersonEntityMergeModel : public QAbstractItemModel
{
    Q_OBJECT

  public:
    enum Roles {
        EntityRole = KTp::CustomRole + 1,
        ContactRole,
        AccountRole,
        ItemTypeRole
    };

    enum ItemType {
        Group,
        Persona,
        Entity
    };

    explicit PersonEntityMergeModel(KTp::ContactsModel *contactsModel, EntityModel *entityModel,
                                    QObject *parent);
    virtual ~PersonEntityMergeModel();

    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

  private Q_SLOTS:
    void contactsModelRowsInserted(const QModelIndex &parent, int start, int end);
    void contactsModelRowsRemoved(const QModelIndex &parent, int start, int end);
    void contactsModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void entityModelRowsInserted(const QModelIndex &parent, int start, int end);
    void entityModelRowsRemoved(const QModelIndex &parent, int start, int end);
    void entityModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

  private:
    class Item;
    class ContactItem;
    class GroupItem;

    GroupItem* groupForName(const QString &name);
    ContactItem* itemForPerson(const QModelIndex &personsModel_personaIndex);
    Item* itemForMergeModelIndex(const QModelIndex &index) const;
    Item* itemForEntityModelIndex(const QModelIndex &index) const;
    Item* itemForContactsModelIndex(const QModelIndex &index) const;
    QModelIndex indexForItem(Item *item) const;
    void findPersonForId(const QString &entityId, Item **parentItem,
                         QModelIndex &personaIndex, QModelIndex &contactIndex);

    KTp::ContactsModel *m_contactsModel;
    EntityModel *m_entityModel;

    ContactItem *m_rootItem;

    QHash<QString, Item*> m_contactLookup;

};

#endif // PERSONENTITYMERGEMODEL_H




