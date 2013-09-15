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


#include "entity-filter-model.h"
#include "entity-model.h"
#include "person-entity-merge-model.h"

#include <TelepathyQt/Types>

#include <KDE/KDebug>

EntityFilterModel::EntityFilterModel(QObject *parent):
    QSortFilterProxyModel(parent)
{
}

EntityFilterModel::~EntityFilterModel()
{
}

bool EntityFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    /* Fast path for groups */
    if (source_parent == QModelIndex()) {
        return true;
    }

    const QModelIndex index = source_parent.child(source_row, 0);

    const PersonEntityMergeModel::ItemType itemType =
        static_cast<PersonEntityMergeModel::ItemType>(index.data(PersonEntityMergeModel::ItemTypeRole).toUInt());
    if (itemType == PersonEntityMergeModel::Entity) {
        const Tp::AccountPtr account = index.data(PersonEntityMergeModel::AccountRole).value< Tp::AccountPtr >();
        const KTp::LogEntity entity = index.data(PersonEntityMergeModel::EntityRole).value< KTp::LogEntity >();
        Q_ASSERT(entity.isValid());

        bool matches_filter = false;

        if (!m_searchHits.isEmpty() && !account.isNull() && entity.isValid()) {
            Q_FOREACH(const KTp::LogSearchHit &searchHit, m_searchHits) {
                const Tp::AccountPtr searchHitAccount = searchHit.account();
                const KTp::LogEntity searchHitTarget = searchHit.entity();

                /* Don't display search hits with empty account or target */
                if (searchHitAccount.isNull() || !searchHitTarget.isValid()) {
                    continue;
                }

                if ((searchHitAccount->uniqueIdentifier() == account->uniqueIdentifier()) &&
                    (searchHitTarget.id() == entity.id()))
                {
                    matches_filter = true;
                    break;
                }
            }
        } else {
            matches_filter = true;
        }

        const QString term = filterRegExp().pattern();
        if (term.isEmpty()) {
            return matches_filter;
        }

        const KTp::ContactPtr contact = index.data(PersonEntityMergeModel::ContactRole).value< KTp::ContactPtr >();

        /* Check if contact's account name matches */
        if (entity.alias().contains(term, Qt::CaseInsensitive) && matches_filter) {
            kDebug() << entity.alias() << "matches" << term;
            return matches_filter;
        }

        /* If there's information about contact's real name try to match it too */
        if (!contact.isNull()) {
            if (contact->alias().contains(term, Qt::CaseInsensitive) && matches_filter) {
                kDebug() << contact->alias() << "matches" << term;
                return matches_filter;
            }
        }

        return false;
    } else if (itemType == PersonEntityMergeModel::Persona) {
        if (sourceModel()->rowCount(index) == 0) {
            return true;
        }

        for (int i = 0; i < sourceModel()->rowCount(index); ++i) {
            if (filterAcceptsRow(i, index)) {
                return true;
            }
        }
        return false;
    }

    Q_ASSERT(false);
    return false;
}

void EntityFilterModel::setSearchHits(const QList<KTp::LogSearchHit> &searchHits)
{
    m_searchHits = searchHits;

    invalidate();
}


void EntityFilterModel::clearSearchHits()
{
    m_searchHits.clear();

    invalidate();
}
