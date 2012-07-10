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


#ifndef ENTITY_MODEL_ITEM_H
#define ENTITY_MODEL_ITEM_H

#include <QVariant>
#include <QList>

#include <TelepathyQt/Types>
#include <TelepathyLoggerQt4/Entity>

class EntityModelItem
{

public:
    EntityModelItem(EntityModelItem *parent = 0);
    virtual ~EntityModelItem();

    void addItem(EntityModelItem *item);

    EntityModelItem* item(int row) const;
    EntityModelItem* item(const Tp::AccountPtr &account);
    int itemCount() const;

    QVariant data(int role) const;
    void setData(const QVariant &data, int role);

    int row() const;
    EntityModelItem* parent() const;

private:
    QList< EntityModelItem* > m_items;
    EntityModelItem *m_parent;

    Tp::AccountPtr m_account;
    Tpl::EntityPtr m_entity;
    Tp::ContactPtr m_contact;
};

#endif // ENTITY_MODEL_ITEM_H
