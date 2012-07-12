/***************************************************************************
 *   Copyright (C) 2012 by David Edmundson <kde@davidedmundson.co.uk>      *
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


#include "entity-model.h"
#include "entity-model-item.h"

#include <TelepathyLoggerQt4/LogManager>
#include <TelepathyLoggerQt4/PendingEntities>
#include <TelepathyLoggerQt4/PendingOperation>
#include <TelepathyLoggerQt4/Entity>

#include <TelepathyQt/AccountManager>
#include <TelepathyQt/Account>
#include <TelepathyQt/ContactManager>
#include <TelepathyQt/PendingOperation>
#include <TelepathyQt/PendingContacts>

#include <QPixmap>

EntityModel::EntityModel(QObject *parent) :
    QAbstractItemModel(parent)
{
    m_rootItem = new EntityModelItem();
}

EntityModel::~EntityModel()
{
    delete m_rootItem;
}

void EntityModel::setAccountManager(const Tp::AccountManagerPtr &accountManager)
{
    Tpl::LogManagerPtr logManager = Tpl::LogManager::instance();
    Q_FOREACH(const Tp::AccountPtr &account, accountManager->allAccounts()) {
        connect(logManager->queryEntities(account),
                SIGNAL(finished(Tpl::PendingOperation*)),
                SLOT(onEntitiesSearchFinished(Tpl::PendingOperation*)));
    }
}

int EntityModel::rowCount(const QModelIndex &parent) const
{
    if (parent == QModelIndex()) {
        return m_rootItem->itemCount();
    }

    return static_cast<EntityModelItem*>(parent.internalPointer())->itemCount();
}

int EntityModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return 1;
}

QModelIndex EntityModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    EntityModelItem *parentItem, *childItem;
    if (parent == QModelIndex()) {
        parentItem = m_rootItem;
    } else {
        parentItem = static_cast<EntityModelItem*>(parent.internalPointer());
    }

    childItem = parentItem->item(row);
    if (childItem) {
        return createIndex(row, column, childItem);
    } else {
        return QModelIndex();
    }
}

QModelIndex EntityModel::parent(const QModelIndex &child) const
{
    if (child == QModelIndex()) {
        return QModelIndex();
    }

    EntityModelItem* item = static_cast< EntityModelItem* >(child.internalPointer());
    EntityModelItem *parent = item->parent();

    if (parent == m_rootItem) {
        return QModelIndex();
    }

    return createIndex(parent->row(), 0, parent);
}

QVariant EntityModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    EntityModelItem *item;
    if (index.parent() == QModelIndex()) {
        item = m_rootItem->item(index.row());
    } else {
        item = m_rootItem->item(index.parent().row())->item(index.row());
    }

    if (!item) {
        return QVariant();
    }

    return item->data(role);
}

void EntityModel::onEntitiesSearchFinished(Tpl::PendingOperation *operation)
{
    Tpl::PendingEntities *pendingEntities = qobject_cast<Tpl::PendingEntities*>(operation);

    Tpl::EntityPtrList newEntries = pendingEntities->entities();

    if (newEntries.size() > 0) {
        Q_FOREACH(const Tpl::EntityPtr &entity, newEntries) {
            EntityModelItem *parent = m_rootItem->item(pendingEntities->account());
            if (!parent) {
                beginInsertRows(QModelIndex(), m_rootItem->itemCount(), m_rootItem->itemCount());
                parent = new EntityModelItem(m_rootItem);
                parent->setData(QVariant::fromValue(pendingEntities->account()), EntityModel::AccountRole);
                m_rootItem->addItem(parent);
                endInsertRows();
            }


            QModelIndex parentIndex = index(parent->row(), 0, QModelIndex());
            beginInsertRows(parentIndex, m_rootItem->item(parentIndex.row())->row(), m_rootItem->item(parentIndex.row())->row());
            EntityModelItem *item = new EntityModelItem(parent);
            item->setData(QVariant::fromValue(pendingEntities->account()), EntityModel::AccountRole);
            item->setData(QVariant::fromValue(entity), EntityModel::EntityRole);
            parent->addItem(item);

            if (pendingEntities->account()->connection()) {
                Tp::PendingOperation *op =
                    pendingEntities->account()->connection()->contactManager()->contactsForIdentifiers(
                                            QStringList() << entity->identifier());
                connect(op, SIGNAL(finished(Tp::PendingOperation*)),
                        this, SLOT(onEntityContactRetrieved(Tp::PendingOperation*)));
            }
            endInsertRows();
        }

    }
}

void EntityModel::onEntityContactRetrieved(Tp::PendingOperation *operation)
{
    Tp::PendingContacts *pendingContacts = qobject_cast<Tp::PendingContacts*>(operation);

    Q_FOREACH(const Tp::ContactPtr &contact, pendingContacts->contacts()) {

        int accRow = 0; int itemRow = 0;
        EntityModelItem *parent, *item;

        do {
            parent = m_rootItem->item(accRow);
            item = parent->item(itemRow);

            if (item->data(EntityModel::IdRole).toString() == contact->id()) {
                item->setData(QVariant::fromValue(contact), EntityModel::ContactRole);
                break;
            }

            itemRow++;
            if (itemRow >= parent->itemCount()) {
                accRow++;
                itemRow = 0;
            }
        } while (accRow < m_rootItem->itemCount());
    }
}
