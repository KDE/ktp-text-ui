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

#include <TelepathyLoggerQt4/LogManager>
#include <TelepathyLoggerQt4/PendingDates>
#include <TelepathyLoggerQt4/Entity>
#include <TelepathyLoggerQt4/SearchHit>
#include <TelepathyQt/Account>

#include <KDE/KLocalizedString>
#include <KDE/KLocalizedDate>

Q_DECLARE_METATYPE(Tp::AccountPtr)
Q_DECLARE_METATYPE(Tpl::EntityPtr)

class DatesModel::Date
{
  public:
    QDate date;
    QList<AccountEntityPair> matches;
};

bool compareDates(const DatesModel::Date* date1, const DatesModel::Date* date2)
{
    return date1->date < date2->date;
}

DatesModel::DatesModel(QObject* parent):
    QAbstractItemModel(parent),
    m_resetInProgress(0)
{
}

DatesModel::~DatesModel()
{
    qDeleteAll(m_dates);
}

void DatesModel::addEntity(const Tp::AccountPtr& account, const Tpl::EntityPtr& entity)
{
    if (m_resetInProgress == 0) {
        beginResetModel();
    }
    ++m_resetInProgress;

    Tpl::LogManagerPtr logManager = Tpl::LogManager::instance();
    Tpl::PendingDates *pendingDates = logManager->queryDates(account, entity,
                                                             Tpl::EventTypeMaskText);
    connect(pendingDates, SIGNAL(finished(Tpl::PendingOperation*)),
            SLOT(onDatesReceived(Tpl::PendingOperation*)));
}

void DatesModel::setEntity(const Tp::AccountPtr& account, const Tpl::EntityPtr& entity)
{
    clear();
    addEntity(account, entity);
}

void DatesModel::setSearchHits(const Tpl::SearchHitList& searchHits)
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
    m_dates.clear();
    m_pairs.clear();
    // Don't reset searchHits!
    endResetModel();
}

QDate DatesModel::previousDate(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return QDate();
    }

    QModelIndex sibling;
    if (!index.parent().isValid() && index.row() > 0) {
         sibling = index.sibling(index.row() - 1, 0);
    } else if (index.parent().row() > 0) {
        sibling = index.parent().sibling(index.parent().row() - 1, 0);
    }

    if (sibling.isValid()) {
        return sibling.data(DatesModel::DateRole).toDate();
    }

    return QDate();
}

QDate DatesModel::nextDate(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return QDate();
    }

    QModelIndex sibling;
    if (!index.parent().isValid() && index.row() < m_dates.count() - 1) {
        sibling = index.sibling(index.row() + 1, 0);
    } else if (index.parent().row() < m_dates.count() - 1) {
        sibling = index.parent().sibling(index.parent().row() + 1, 0);
    }

    if (sibling.isValid()) {
        return sibling.data(DatesModel::DateRole).toDate();
    }

    return QDate();
}

QModelIndex DatesModel::indexForDate(const QDate& date) const
{
    for (int i = 0; i < m_dates.count(); ++i) {
        if (m_dates[i]->date == date) {
            return index(i, 0, QModelIndex());
        }
    }

    return QModelIndex();
}

QVariant DatesModel::data(const QModelIndex& index, int role) const
{
    Date *date;
    AccountEntityPair pair;
    bool isDate = false;

    // It's a date node
    if (index.internalPointer() == 0) {
        Q_ASSERT_X(index.row() < m_dates.count(), "DatesModel::data()", "Requested data out of range");
        date = m_dates.at(index.row());
        isDate = true;
        if (date->matches.count() == 1) {
            pair = date->matches.first();
        }

    // It's an account/entity node
    } else {
        date = static_cast<Date*>(index.internalPointer());
        Q_ASSERT_X(index.row() < date->matches.count(), "DatesModel::data()", "Requested data out of range");
        pair = date->matches.at(index.row());
    }

    switch (role) {
        case Qt::DisplayRole:
            return isDate
                ? KLocalizedDate(date->date).formatDate()
                : pair.first->displayName();
        case DateRole:
            return date->date;
        case HintRole:
            return pair.first.isNull()
                ? i18ncp("Number of existing conversations.", "%1 conversation", "%1 conversations", date->matches.count())
                : pair.first->displayName();
        case AccountRole:
            return QVariant::fromValue<Tp::AccountPtr>(pair.first);
        case EntityRole:
            return QVariant::fromValue<Tpl::EntityPtr>(pair.second);
    }

    return QVariant();
}

int DatesModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)

    return 1;
}

int DatesModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid()) {
        return m_dates.count();
    }

    if (!parent.parent().isValid()) {
        Date *date = m_dates.at(parent.row());

        // Only make the date expandable if there is more than one account
        if (date->matches.count() == 1) {
            return 0;
        }

        return date->matches.count();
    }

    return 0;
}

QModelIndex DatesModel::parent(const QModelIndex& child) const
{
    if (!child.isValid()) {
        return QModelIndex();
    }

    Date *date = static_cast<Date*>(child.internalPointer());
    if (date == 0) {
        return QModelIndex();
    }

    return createIndex(m_dates.indexOf(date), 0);
}

QModelIndex DatesModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!parent.isValid()) {
        return createIndex(row, column);
    }

    return createIndex(row, column, m_dates.at(parent.row()));
}

void DatesModel::onDatesReceived(Tpl::PendingOperation* operation)
{
    // Stop here if clear() was called meanwhile
    if (m_resetInProgress == 0) {
        operation->deleteLater();
        return;
    }

    Tpl::PendingDates *op = qobject_cast<Tpl::PendingDates*>(operation);
    Q_ASSERT(op);

    QList<QDate> newDates = op->dates();
    Q_FOREACH (const QDate &newDate, newDates) {
        bool found = false;
        Q_FOREACH (Date *modelDate, m_dates) {
            if (modelDate->date == newDate) {
                modelDate->matches << AccountEntityPair(op->account(), op->entity());
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
        m_dates << date;
    }

    op->deleteLater();

    qSort(m_dates.begin(), m_dates.end(), compareDates);

    --m_resetInProgress;
    if (m_resetInProgress == 0) {
        endResetModel();
    }
}


#include "dates-model.moc"
