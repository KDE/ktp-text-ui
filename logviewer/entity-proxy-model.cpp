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


#include "entity-proxy-model.h"
#include "entity-model.h"

#include <TelepathyQt/Types>

EntityProxyModel::EntityProxyModel(QObject *parent):
    QSortFilterProxyModel(parent)
{
}

EntityProxyModel::~EntityProxyModel()
{
}

bool EntityProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    /* Always display account node */
    if (source_parent == QModelIndex()) {
        return true;
    }

    QModelIndex index = source_parent.child(source_row, 0);
    QString term = filterRegExp().pattern();

    Tp::ContactPtr contact = index.data(EntityModel::ContactRole).value< Tp::ContactPtr >();
    Tpl::EntityPtr entity = index.data(EntityModel::EntityRole).value< Tpl::EntityPtr >();

    /* Check if contact's account name matches */
    if (entity->alias().contains(term, Qt::CaseInsensitive)) {
        return true;
    }

    /* If there's information about contact's real name try to match it too */
    if (!contact.isNull()) {
        if (contact->alias().contains(term, Qt::CaseInsensitive)) {
            return true;
        }
    }

    return false;
}
