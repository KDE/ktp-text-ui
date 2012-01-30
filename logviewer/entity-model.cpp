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

#include <QPixmap>


EntityModel::EntityModel(QObject *parent) :
    QAbstractListModel(parent)
{
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
        return m_entities.size();
    } else {
        return 0;
    }
}

QVariant EntityModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    Tpl::EntityPtr entity = m_entities[index.row()].entity;

    switch (role) {
    case Qt::DisplayRole:
        return entity->alias();
    case Qt::DecorationRole:
        return QPixmap(entity->avatarToken());
    case EntityModel::IdRole:
        return entity->identifier();
    case EntityModel::TypeRole:
        return entity->entityType();
    case EntityModel::EntityRole:
        return QVariant::fromValue(entity);
    case EntityModel::AccountRole:
        return QVariant::fromValue(m_entities[index.row()].account);
    }
    return QVariant();
}

void EntityModel::onEntitiesSearchFinished(Tpl::PendingOperation *operation)
{
    Tpl::PendingEntities *pendingEntities = qobject_cast<Tpl::PendingEntities*>(operation);

    Tpl::EntityPtrList newEntries = pendingEntities->entities();

    if (newEntries.size() > 0) {
        beginInsertRows(QModelIndex(), m_entities.size(), m_entities.size() + newEntries.size()-1);
        Q_FOREACH(const Tpl::EntityPtr entity, newEntries) {
            EntityModelItem item;
            item.account = pendingEntities->account();
            item.entity = entity;

            m_entities.append(item);
        }

        endInsertRows();
    }
}

