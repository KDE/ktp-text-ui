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
    Tp::AccountPtr account = source_parent.data(EntityModel::AccountRole).value< Tp::AccountPtr >();
    KTp::LogEntity entity = index.data(EntityModel::EntityRole).value< KTp::LogEntity >();

    bool matches_filter = false;

    /*
    if (!m_searchHits.isEmpty() && !account.isNull() && !entity.isNull()) {
        Q_FOREACH(const Tpl::SearchHit &searchHit, m_searchHits) {
            Tp::AccountPtr searchHitAccount = searchHit.account();
            Tpl::EntityPtr searchHitTarget = searchHit.target();

            // Don't display search hits with empty account or target
            if (searchHitAccount.isNull() || searchHitTarget.isNull()) {
                continue;
            }

            if ((searchHitAccount->uniqueIdentifier() == account->uniqueIdentifier()) &&
                (searchHitTarget->identifier() == entity->identifier())) {
                matches_filter = true;
            }
        }
    } else {
        matches_filter = true;
    }
    */
    matches_filter = true;

    QString term = filterRegExp().pattern();
    if (term.isEmpty()) {
        return matches_filter;
    }

    KTp::ContactPtr contact = index.data(EntityModel::ContactRole).value< KTp::ContactPtr >();

    /* Check if contact's account name matches */
    if (entity.alias().contains(term, Qt::CaseInsensitive) && matches_filter) {
        return true;
    }

    /* If there's information about contact's real name try to match it too */
    if (!contact.isNull()) {
        if (contact->alias().contains(term, Qt::CaseInsensitive) && matches_filter) {
            return true;
        }
    }

    return false;
}

/*
void EntityProxyModel::setSearchHits(const Tpl::SearchHitList &searchHits)
{
    m_searchHits = searchHits;

    invalidate();
}


void EntityProxyModel::clearSearchHits()
{
    m_searchHits.clear();

    invalidate();
}
*/
