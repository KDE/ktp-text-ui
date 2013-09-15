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

#include <KTp/Logger/log-manager.h>
#include <KTp/Logger/log-entity.h>
#include <KTp/Logger/pending-logger-entities.h>

#include <TelepathyQt/AccountManager>
#include <TelepathyQt/Account>
#include <TelepathyQt/ContactManager>
#include <TelepathyQt/PendingOperation>
#include <TelepathyQt/PendingContacts>

#include <KDebug>

class EntityModelItem
{
  public:
    Tp::AccountPtr account;
    KTp::LogEntity entity;
    QString displayName;
};

EntityModel::EntityModel(QObject *parent) :
    QAbstractItemModel(parent)
{
}

EntityModel::~EntityModel()
{
    qDeleteAll(m_items);
}

void EntityModel::setAccountManager(const Tp::AccountManagerPtr &accountManager)
{
    KTp::LogManager *logManager = KTp::LogManager::instance();
    logManager->setAccountManager(accountManager);
    m_accountManager = accountManager;
    Q_FOREACH(const Tp::AccountPtr &account, accountManager->allAccounts()) {
        KTp::PendingLoggerEntities *op = logManager->queryEntities(account);
        m_pendingOperations << op;
        connect(op, SIGNAL(finished(KTp::PendingLoggerOperation*)),
                this, SLOT(onEntitiesSearchFinished(KTp::PendingLoggerOperation*)));
    }
}

int EntityModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_items.count();
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

    const QStringList keys = m_items.uniqueKeys();
    EntityModelItem *childItem = m_items.value(keys.at(row));
    if (childItem) {
        return createIndex(row, column, childItem);
    }

    return QModelIndex();
}

QModelIndex EntityModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child);

    return QModelIndex();
}

QVariant EntityModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    EntityModelItem *item = static_cast<EntityModelItem*>(index.internalPointer());

    switch (role) {
        case Qt::DisplayRole:
            return !item->displayName.isEmpty() ? item->displayName : item->entity.alias();
        case EntityModel::AccountRole:
            return QVariant::fromValue(item->account);
        case EntityModel::EntityRole:
            return QVariant::fromValue(item->entity);
        case EntityModel::IdRole:
            return item->entity.id();
    }

    return QVariant();
}

bool EntityModel::removeRows(int start, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, start, start + count - 1);
    const QStringList keys = m_items.uniqueKeys();
    for (int row = 0; row < count; ++row) {
        const QString key = keys.at(start + row);
        m_items.remove(key);
    }
    endRemoveRows();
    return true;
}

void EntityModel::onEntitiesSearchFinished(KTp::PendingLoggerOperation *operation)
{
    KTp::PendingLoggerEntities *pendingEntities = qobject_cast<KTp::PendingLoggerEntities*>(operation);
    const QList<KTp::LogEntity> newEntries = pendingEntities->entities();

    QStringList ids;
    Q_FOREACH (const KTp::LogEntity &entity, newEntries) {
        beginInsertRows(QModelIndex(), m_items.count(), m_items.count());
        EntityModelItem *item = new EntityModelItem();
        item->account = pendingEntities->account();
        item->entity = entity;
        m_items.insert(entity.id(), item);
        ids << entity.id();
        endInsertRows();
    }

    Q_ASSERT(m_pendingOperations.contains(operation));
    m_pendingOperations.removeAll(operation);
    if (m_pendingOperations.isEmpty()) {
        Q_EMIT modelInitialized();
    }
}
