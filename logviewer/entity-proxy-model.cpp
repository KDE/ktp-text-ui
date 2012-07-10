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

#include <QDebug>

EntityProxyModel::EntityProxyModel(QObject *parent):
    QSortFilterProxyModel(parent)
{
}

EntityProxyModel::~EntityProxyModel()
{
}

bool EntityProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (source_parent == QModelIndex()) {
        return true;
    }

    QModelIndex index = source_parent.child(source_row, 0);
    QString term = filterRegExp().pattern();
    return index.data(Qt::DisplayRole).toString().contains(term, Qt::CaseInsensitive);
}
