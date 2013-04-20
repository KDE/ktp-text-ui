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

#include <TelepathyLoggerQt4/LogManager>
#include <TelepathyLoggerQt4/PendingEntities>
#include <TelepathyLoggerQt4/PendingOperation>
#include <TelepathyLoggerQt4/Entity>

#include <TelepathyQt/AccountManager>
#include <TelepathyQt/Account>
#include <TelepathyQt/ContactManager>
#include <TelepathyQt/PendingOperation>
#include <TelepathyQt/PendingContacts>

class EntityModelItem
{
  public:
    Tp::AccountPtr account;
    Tpl::EntityPtr entity;
    Tp::ContactPtr contact;
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
    Tpl::LogManagerPtr logManager = Tpl::LogManager::instance();
    Q_FOREACH (const Tp::AccountPtr &account, accountManager->allAccounts()) {
        connect(logManager->queryEntities(account),
                SIGNAL(finished(Tpl::PendingOperation*)),
                SLOT(onEntitiesSearchFinished(Tpl::PendingOperation*)));
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

    EntityModelItem *childItem = m_items.at(row);
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

    EntityModelItem *item = m_items.at(index.row());

    switch (role) {
        case Qt::DisplayRole:
            return item->contact ? item->contact->alias() : item->entity->alias();
        case EntityModel::AccountRole:
            return QVariant::fromValue(item->account);
        case EntityModel::ContactRole:
            return QVariant::fromValue(item->contact);
        case EntityModel::EntityRole:
            return QVariant::fromValue(item->entity);
        case EntityModel::IdRole:
            return item->entity->identifier();
    }

    return QVariant();
}

bool EntityModel::removeRows(int start, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, start, start + count - 1);
    for (int row = 0; row < count; ++row) {
        m_items.removeAt(start);
    }
    endRemoveRows();

    return true;
}

void EntityModel::onEntitiesSearchFinished(Tpl::PendingOperation *operation)
{
    Tpl::PendingEntities *pendingEntities = qobject_cast<Tpl::PendingEntities*>(operation);

    Tpl::EntityPtrList newEntries = pendingEntities->entities();

    Q_FOREACH (const Tpl::EntityPtr &entity, newEntries) {
        beginInsertRows(QModelIndex(), m_items.count(), m_items.count());
        EntityModelItem *item = new EntityModelItem();
        item->account = pendingEntities->account();
        item->entity = entity;
        m_items << item;
        endInsertRows();

        if (pendingEntities->account()->connection()) {
            Tp::PendingOperation *op =
                pendingEntities->account()->connection()->contactManager()->contactsForIdentifiers(
                                        QStringList() << entity->identifier());
            connect(op, SIGNAL(finished(Tp::PendingOperation*)),
                    this, SLOT(onEntityContactRetrieved(Tp::PendingOperation*)));
        }
    }
}

void EntityModel::onEntityContactRetrieved(Tp::PendingOperation *operation)
{
    Tp::PendingContacts *pendingContacts = qobject_cast<Tp::PendingContacts*>(operation);

    Q_FOREACH (const Tp::ContactPtr &contact, pendingContacts->contacts()) {
        for (int i = 0; i < m_items.count(); ++i) {
            EntityModelItem *item = m_items.at(i);
            if (item->entity->identifier() == contact->id()) {
                item->contact = contact;
            }
        }
    }
}
