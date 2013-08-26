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

#include <KTp/Logger/log-manager.h>
#include <KTp/Logger/log-entity.h>
#include <KTp/Logger/pending-logger-entities.h>

#include <TelepathyQt/AccountManager>
#include <TelepathyQt/Account>
#include <TelepathyQt/ContactManager>
#include <TelepathyQt/PendingOperation>
#include <TelepathyQt/PendingContacts>

#include <KTp/contact.h>

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
    KTp::LogManager *logManager = KTp::LogManager::instance();
    Q_FOREACH(const Tp::AccountPtr &account, accountManager->allAccounts()) {
        connect(logManager->queryEntities(account),
                SIGNAL(finished(KTp::PendingLoggerOperation*)),
                SLOT(onEntitiesSearchFinished(KTp::PendingLoggerOperation*)));
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

Qt::ItemFlags EntityModel::flags(const QModelIndex &index) const
{
    if (index.parent() == QModelIndex()) {
        return QAbstractItemModel::flags(index) & ~Qt::ItemIsSelectable;
    }

    return QAbstractItemModel::flags(index);
}

bool EntityModel::removeRows(int start, int count, const QModelIndex &parent)
{
    EntityModelItem *parentItem;

    if (parent.isValid()) {
        parentItem = m_rootItem->item(parent.row());
    } else {
        parentItem = m_rootItem;
    }

    beginRemoveRows(parent, start, start + count - 1);
    for (int row = 0; row < count; ++row) {
        parentItem->removeItem(start);
    }
    endRemoveRows();

    return true;
}

void EntityModel::onEntitiesSearchFinished(KTp::PendingLoggerOperation *operation)
{
    KTp::PendingLoggerEntities *pendingEntities = qobject_cast<KTp::PendingLoggerEntities*>(operation);
    const QList<KTp::LogEntity> newEntries = pendingEntities->entities();

    Q_FOREACH(const KTp::LogEntity &entity, newEntries) {
        EntityModelItem *parent = m_rootItem->item(pendingEntities->account());
        if (!parent) {
            beginInsertRows(QModelIndex(), m_rootItem->itemCount(), m_rootItem->itemCount());
            parent = new EntityModelItem(m_rootItem);
            parent->setData(QVariant::fromValue(pendingEntities->account()), EntityModel::AccountRole);
            m_rootItem->addItem(parent);
            endInsertRows();
        }

        QModelIndex parentIndex = index(parent->row(), 0, QModelIndex());
        beginInsertRows(parentIndex, parent->itemCount() , parent->itemCount());
        EntityModelItem *item = new EntityModelItem(parent);
        item->setData(QVariant::fromValue(pendingEntities->account()), EntityModel::AccountRole);
        item->setData(QVariant::fromValue(entity), EntityModel::EntityRole);
        parent->addItem(item);

        if (pendingEntities->account()->connection()) {
            Tp::PendingOperation *op =
                pendingEntities->account()->connection()->contactManager()->contactsForIdentifiers(
                                        QStringList() << entity.id());
            connect(op, SIGNAL(finished(Tp::PendingOperation*)),
                    this, SLOT(onEntityContactRetrieved(Tp::PendingOperation*)));
        }
        endInsertRows();
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
                item->setData(QVariant::fromValue(KTp::ContactPtr::qObjectCast(contact)), EntityModel::ContactRole);
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
