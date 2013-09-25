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

#include "dates-model.h"

#include <KTp/Logger/log-manager.h>
#include <KTp/Logger/log-entity.h>
#include <KTp/Logger/log-search-hit.h>
#include <KTp/Logger/pending-logger-dates.h>
#include <TelepathyQt/Account>

#include <KDE/KLocalizedString>
#include <klocalizeddate.h>
#include <KDE/KDebug>

Q_DECLARE_METATYPE(Tp::AccountPtr)

class DatesModel::Date
{
  public:
    QDate date;
    QList<AccountEntityPair> matches;
};

bool sortDatesDescending(const DatesModel::Date* date1, const DatesModel::Date* date2)
{
    return date1->date > date2->date;
}

DatesModel::DatesModel(QObject* parent):
    QAbstractItemModel(parent),
    m_resetInProgress(0)
{
}

DatesModel::~DatesModel()
{
    clear();
}

void DatesModel::addEntity(const Tp::AccountPtr &account, const KTp::LogEntity &entity)
{
    if (m_resetInProgress == 0) {
        beginResetModel();
    }
    ++m_resetInProgress;

    KTp::LogManager *logManager = KTp::LogManager::instance();
    KTp::PendingLoggerDates *pendingDates = logManager->queryDates(account, entity);
    m_pendingDates << pendingDates;
    connect(pendingDates, SIGNAL(finished(KTp::PendingLoggerOperation*)),
            SLOT(onDatesReceived(KTp::PendingLoggerOperation*)));
}

void DatesModel::setEntity(const Tp::AccountPtr &account, const KTp::LogEntity &entity)
{
    clear();
    addEntity(account, entity);
}

void DatesModel::setSearchHits(const QList<KTp::LogSearchHit> &searchHits)
{
    m_searchHits = searchHits;
}

void DatesModel::clearSearchHits()
{
    m_searchHits.clear();
}

void DatesModel::clear()
{
    beginResetModel();
    m_resetInProgress = 0;
    Q_FOREACH (KTp::PendingLoggerOperation *op, m_pendingDates) {
        disconnect(op, SIGNAL(finished(KTp::PendingLoggerOperation*)));
        op->deleteLater();
    }
    m_pendingDates.clear();
    m_groups.clear();
    Q_FOREACH (const QList<Date*> &list, m_items) {
        qDeleteAll(list);
    }
    m_items.clear();
    m_pairs.clear();
    // Don't reset searchHits!
    endResetModel();
}

QDate DatesModel::nextDate(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QDate();
    }

    // Group - it should never be selected
    if (index.internalId() == -1) {
        return QDate();
    }

    // Date
    QModelIndex idx;
    if (index.internalId() >= 0 && index.internalId() < m_groups.count()) {
        idx = index;
    } else {
        idx = index.parent();
    }

    QModelIndex sibling;
    if (idx.row() == 0) {
        QModelIndex idxParent = idx.parent();
        if (idxParent.row() == 0) {
            return QDate();
        } else {
            idxParent = idxParent.sibling(idxParent.row() - 1, 0);
            sibling = idxParent.child(rowCount(idxParent) - 1, 0);
        }
    } else {
        sibling = idx.sibling(idx.row() - 1, 0);
    }

    if (sibling.isValid()) {
        return sibling.data(DatesModel::DateRole).toDate();
    }

    return QDate();
}

QDate DatesModel::previousDate(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QDate();
    }

    // Group - it should never be selected
    if (index.internalId() == -1) {
        return QDate();
    }

    // Date
    QModelIndex idx;
    if (index.internalId() >= 0 && index.internalId() < m_groups.count()) {
        idx = index;
    } else {
        idx = index.parent();
    }

    QModelIndex sibling;
    if (idx.row() == rowCount(idx.parent()) - 1) {
        QModelIndex idxParent = idx.parent();
        if (idxParent.row() == rowCount(idxParent.parent()) - 1) {
            return QDate();
        } else {
            idxParent = idxParent.sibling(idxParent.row() + 1, 0);
            sibling = idxParent.child(0, 0);
        }
    } else {
        sibling = idx.sibling(idx.row() + 1, 0);
    }

    if (sibling.isValid()) {
        return sibling.data(DatesModel::DateRole).toDate();
    }

    return QDate();
}

QModelIndex DatesModel::indexForDate(const QDate &date) const
{
    const QDate groupDate(date.year(), date.month(), 1);
    const QModelIndex parent = index(m_groups.indexOf(groupDate), 0, QModelIndex());

    QList<Date*> dates = m_items.value(groupDate);
    for (int i = 0; i < dates.count(); ++i) {
        if (dates[i]->date == date) {
            return index(i, 0, parent);
        }
    }

    return QModelIndex();
}

QVariant DatesModel::data(const QModelIndex &index, int role) const
{
    Date *date = 0;
    AccountEntityPair pair;
    bool isDate = false;

    // It's a group
    if (index.internalId() == -1) {
        switch (role) {
            case Qt::DisplayRole:
                return m_groups.at(index.row()).toString(QLatin1String("MMMM yyyy"));
            case DateRole:
                return m_groups.at(index.row());
            case TypeRole:
                return DatesModel::GroupRow;
            default:
                return QVariant();
        }
        return QVariant();
    }

    // It's a date node
    if (index.internalId() >= 0 && index.internalId() <= m_groups.count()) {
        const QDate key = m_groups.at(index.parent().row());
        date = m_items.value(key).at(index.row());
        isDate = true;
        if (date->matches.count() == 1) {
            pair = date->matches.first();
        }
    // It's an account/entity node
    } else {
        Date *date = static_cast<Date*>(index.internalPointer());
        pair = date->matches.at(index.row());
    }

    switch (role) {
        case Qt::DisplayRole:
            return isDate
                ? KLocalizedDate(date->date).formatDate()
                : pair.first->displayName();
        case TypeRole:
            return isDate ? DatesModel::DateRow : DatesModel::ConversationRow;
        case DateRole:
            return date->date;
        case HintRole:
            return pair.first.isNull()
                ? i18ncp("Number of existing conversations.", "%1 conversation", "%1 conversations", date->matches.count())
                : pair.first->displayName();
        case AccountRole:
            return QVariant::fromValue<Tp::AccountPtr>(pair.first);
        case EntityRole:
            return QVariant::fromValue<KTp::LogEntity>(pair.second);
    }

    return QVariant();
}

int DatesModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return 1;
}

int DatesModel::rowCount(const QModelIndex &parent) const
{
    // Groups
    if (!parent.isValid()) {
        return m_groups.count();
    }

    // Dates
    if (parent.internalId() == -1) {
        const QDate key = m_groups.at(parent.row());
        const QList<Date*> dates = m_items.value(key);
        return dates.count();
    }

    // Conversations
    if (parent.internalId() >= 0) {
        const QDate key = m_groups.at(parent.parent().row());
        const QList<Date*> dates = m_items.value(key);
        const Date *date = dates.at(parent.row());
        // Only make the date expandable if there is more than one account
        if (date->matches.count() == 1) {
            return 0;
        }
        return date->matches.count();
    }

    return 0;
}

QModelIndex DatesModel::parent(const QModelIndex &child) const
{
    if (!child.isValid()) {
        return QModelIndex();
    }

    // Child is a group
    if (child.internalId() == -1) {
        return QModelIndex();
    }

    // Child is a date
    if (child.internalId() >= 0 && child.internalId() < m_items.count()) {
        return createIndex(child.internalId(), 0, -1);
    }

    // Child is a conversation
    Date *date = static_cast<Date*>(child.internalPointer());
    if (date) {
        const QDate key = QDate(date->date.year(), date->date.month(), 1);
        const QList<Date*> dates = m_items.value(key);
        return createIndex(dates.indexOf(date), 0, m_items.uniqueKeys().indexOf(key));
    }

    // Assert?
    return QModelIndex();
}

QModelIndex DatesModel::index(int row, int column, const QModelIndex &parent) const
{
    // Group
    if (!parent.isValid()) {
        return createIndex(row, column, -1);
    }

    // Date, id is index of parent group
    if (parent.internalId() == -1) {
        return createIndex(row, column, parent.row());
    }

    // Conversation, data is a pointer to parent date
    if (!parent.internalId() >= 0) {
        const QDate key = m_items.uniqueKeys().at(parent.internalId());
        const QList<Date*> dates = m_items.value(key);
        return createIndex(row, column, dates.at(parent.row()));
    }

    return QModelIndex();
}

void DatesModel::onDatesReceived(KTp::PendingLoggerOperation *operation)
{
    // Stop here if clear() was called meanwhile
    if (m_resetInProgress == 0) {
        return;
    }

    KTp::PendingLoggerDates *op = qobject_cast<KTp::PendingLoggerDates*>(operation);
    Q_ASSERT(op);
    Q_ASSERT(m_pendingDates.contains(op));

    m_pendingDates.removeOne(op);
    op->deleteLater();

    QList<QDate> newDates = op->dates();
    Q_FOREACH (const QDate &newDate, newDates) {
        const QDate groupDate(newDate.year(), newDate.month(), 1);
        if (!m_groups.contains(groupDate)) {
            m_groups << groupDate;
            // Sort dates in descending order
            qStableSort(m_groups.begin(), m_groups.end(), qGreater<QDate>());

            Date *date = new Date;
            date->date = newDate;
            date->matches << AccountEntityPair(op->account(), op->entity());
            m_items.insert(groupDate, QList<Date*>() << date);
            continue;
        }

        QList<Date*> dates = m_items.value(groupDate);
        bool found = false;
        Q_FOREACH (Date *date, dates) {
            if (date->date == newDate) {
                date->matches << AccountEntityPair(op->account(), op->entity());
                found = true;
                break;
            }
        }

        if (found) {
            continue;
        }

        Date *date = new Date;
        date->date = newDate;
        date->matches << AccountEntityPair(op->account(), op->entity());
        dates.append(date);

        qStableSort(dates.begin(), dates.end(), sortDatesDescending);
        m_items.insert(groupDate, dates);
    }

    --m_resetInProgress;
    if (m_resetInProgress == 0) {
        endResetModel();
    }

    if (m_pendingDates.isEmpty()) {
        Q_EMIT datesReceived();
    }
}

#include "dates-model.moc"
