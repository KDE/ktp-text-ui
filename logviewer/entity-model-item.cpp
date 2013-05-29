/***************************************************************************
 *   Copyright (C) 2012 by Dan Vratil <dan@progdan.cz>                     *
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

#include "entity-model-item.h"
#include "entity-model.h"

#include <TelepathyQt/Contact>
#include <TelepathyLoggerQt4/Entity>

#include <QDebug>

EntityModelItem::EntityModelItem(EntityModelItem *parent):
    m_parent(parent)
{
}

EntityModelItem::~EntityModelItem()
{
    qDeleteAll(m_items);
}

void EntityModelItem::addItem(EntityModelItem *item)
{
    m_items << item;
}

void EntityModelItem::removeItem(int index)
{
    delete m_items.takeAt(index);
}

EntityModelItem* EntityModelItem::item(int row) const
{
    if (row < m_items.count()) {
        return m_items.at(row);
    }

    return 0;
}

EntityModelItem *EntityModelItem::item(const Tp::AccountPtr &account)
{
    Q_FOREACH(EntityModelItem *item, m_items) {
        if (item->data(EntityModel::AccountRole).value< Tp::AccountPtr >() == account) {
            return item;
        }
    }

    return 0;
}


int EntityModelItem::itemCount() const
{
    return m_items.count();
}

EntityModelItem *EntityModelItem::parent() const
{
    return m_parent;
}

int EntityModelItem::row() const
{
    if (m_parent == 0) {
        return 0;
    }

    for (int i = 0; i < m_parent->itemCount(); i++) {
        if (m_parent->item(i) == this) {
            return i;
        }
    }

    return 0;
}

QVariant EntityModelItem::data(int role) const
{
    switch (role) {
        case Qt::DisplayRole:
            return m_contact ? m_contact->alias() : (m_entity ? m_entity->alias() : m_account->displayName());
        case EntityModel::AccountRole:
            return QVariant::fromValue(m_account);
        case EntityModel::ContactRole:
            return QVariant::fromValue(m_contact);
        case EntityModel::EntityRole:
            return QVariant::fromValue(m_entity);
        case EntityModel::IdRole:
            return m_entity->identifier();
    }

    return QVariant();
}

void EntityModelItem::setData(const QVariant &data, int role)
{
    switch (role) {
        case EntityModel::AccountRole:
            m_account = data.value< Tp::AccountPtr >();
            break;
        case EntityModel::ContactRole:
            m_contact = data.value< KTp::ContactPtr >();
            break;
        case EntityModel::EntityRole:
            m_entity = data.value< Tpl::EntityPtr >();
            break;
    }
}
