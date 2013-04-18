/*
 * Copyright (C) 2013  Daniel Vrátil <dvratil@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "persons-filter-model.h"

#include <kpeople/persons-model.h>

#include <TelepathyLoggerQt4/LogManager>
#include <TelepathyLoggerQt4/PendingEntities>
#include <TelepathyLoggerQt4/Entity>
#include <TelepathyLoggerQt4/SearchHit>

#include <TelepathyQt/AccountManager>
#include <TelepathyQt/Contact>

#include <KDE/KDebug>

Q_DECLARE_METATYPE(Tp::AccountPtr)
Q_DECLARE_METATYPE(Tp::ContactPtr)
Q_DECLARE_METATYPE(Tpl::EntityPtr)

bool operator<(const Tp::AccountPtr &a1, const Tp::AccountPtr &a2)
{
    return a1->uniqueIdentifier() < a2->uniqueIdentifier();
}

PersonsFilterModel::PersonsFilterModel(QObject* parent):
    QSortFilterProxyModel(parent)
{
}

PersonsFilterModel::~PersonsFilterModel()
{
}

void PersonsFilterModel::setAccountManager(const Tp::AccountManagerPtr& accountManager)
{
    Tpl::LogManagerPtr logManager = Tpl::LogManager::instance();
    logManager->setAccountManagerPtr(accountManager);

    Q_FOREACH (const Tp::AccountPtr &account, accountManager->allAccounts()) {
        Tpl::PendingEntities *pending = logManager->queryEntities(account);
        connect(pending, SIGNAL(finished(Tpl::PendingOperation*)),
                this, SLOT(slotEntitiesReceived(Tpl::PendingOperation*)));
    }
}

void PersonsFilterModel::setSearchHits(const Tpl::SearchHitList& searchHits)
{
    m_searchHits = searchHits;
    invalidateFilter();
}

Tpl::SearchHitList PersonsFilterModel::searchHits() const
{
    return m_searchHits;
}

void PersonsFilterModel::clearSearchHits()
{
    m_searchHits.clear();
    invalidateFilter();
}

Qt::ItemFlags PersonsFilterModel::flags(const QModelIndex& index) const
{
    // FIXME: Make Persons selectable and show some fancy stuff
    if (index.data(PersonsModel::ResourceTypeRole).toUInt() == PersonsModel::Person) {
        return Qt::ItemIsEnabled;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant PersonsFilterModel::data(const QModelIndex &index, int role) const
{
    if (role == PersonsFilterModel::EntityRole) {
        if (index.data(PersonsModel::ResourceTypeRole).toUInt() == PersonsModel::Person) {
            return QVariant();
        }

        const Tp::AccountPtr account = index.data(PersonsModel::IMAccountRole).value<Tp::AccountPtr>();
        const QString contactUid = index.data(PersonsModel::IMRole).toString();
        const Tpl::EntityPtr entity = findEntity(account, contactUid);
        return QVariant::fromValue<Tpl::EntityPtr>(entity);
    }

    return QSortFilterProxyModel::data(index, role);
}

bool PersonsFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    if (!sourceModel()) {
        return false;
    }

    const QModelIndex sourceIndex = sourceModel()->index(sourceRow, 0, sourceParent);
    if (!sourceIndex.isValid()) {
        return false;
    }

    // Accept Person only when it has at least one contact with logs
    if (sourceIndex.data(PersonsModel::ResourceTypeRole).toUInt() == PersonsModel::Person) {
        for (int i = 0; i < sourceModel()->rowCount(sourceParent); ++i) {
            if (filterAcceptsRow(i, sourceIndex)) {
                return true;
            }
        }
        return false;
    }

    const Tp::AccountPtr account = sourceIndex.data(PersonsModel::IMAccountRole).value<Tp::AccountPtr>();
    // FIXME: Find out why PersonsModel::IMContactRole returns NULL ptr all the time
    const QString contactUid  = sourceIndex.data(PersonsModel::IMRole).toString();
    const Tpl::EntityPtr entity = findEntity(account, contactUid);
    if (entity.isNull()) {
        return false;
    }

    if (m_searchHits.isEmpty()) {
        return true;
    } else {
        Q_FOREACH( Tpl::SearchHit searchHit, m_searchHits) {
            if ((searchHit.account() == account) &&
                (searchHit.target() == entity))
            {
                return true;
            }
        }
    }

    return false;
}

Tpl::EntityPtr PersonsFilterModel::findEntity(const Tp::AccountPtr& account,
                                              const QString &contactUid) const
{
    if (account.isNull() || contactUid.isNull()) {
        return Tpl::EntityPtr();
    }

    if (!m_entities.contains(account)) {
        return Tpl::EntityPtr();
    }

    const QList<Tpl::EntityPtr> entities = m_entities.values(account);
    Q_FOREACH (const Tpl::EntityPtr &entity, entities) {
        if (entity->identifier() == contactUid) {
            return entity;
        }
    }

    return Tpl::EntityPtr();
}

void PersonsFilterModel::slotEntitiesReceived(Tpl::PendingOperation* operation)
{
    Tpl::PendingEntities *pending = qobject_cast<Tpl::PendingEntities*>(operation);
    Q_ASSERT(pending != 0);

    Q_FOREACH (const Tpl::EntityPtr entity, pending->entities()) {
        m_entities.insertMulti(pending->account(), entity);
    }

    invalidateFilter();
}

#include "persons-filter-model.moc"
