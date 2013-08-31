/***************************************************************************
 *   Copyright (C) 2012,2013 by Dan Vratil <dan@progdan.cz>                *
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


#ifndef ENTITY_PROXY_MODEL_H
#define ENTITY_PROXY_MODEL_H

#include <QSortFilterProxyModel>

#include <TelepathyQt/Types>

#include <KTp/Logger/log-entity.h>
#include <KTp/Logger/log-search-hit.h>

typedef QPair< Tp::AccountPtr, KTp::LogEntity > AccountEntityPair;

class EntityFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit EntityFilterModel(QObject *parent = 0);
    virtual ~EntityFilterModel();

    void setSearchHits(const QList<KTp::LogSearchHit> &searchHits);
    void clearSearchHits();

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

private:
    QList<KTp::LogSearchHit> m_searchHits;

};

#endif // ENTITY_PROXY_MODEL_H
