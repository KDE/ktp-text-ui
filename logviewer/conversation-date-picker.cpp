/***************************************************************************
 *   Copyright (C) 2012 by David Edmundson <kde@davidedmundson.co.uk>      *
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

#include "conversation-date-picker.h"

#include <TelepathyLoggerQt4/LogManager>
#include <TelepathyLoggerQt4/PendingDates>
#include <TelepathyLoggerQt4/PendingOperation>
#include <TelepathyLoggerQt4/Entity>
#include <TelepathyLoggerQt4/SearchHit>

#include <KDateTable>
#include <TelepathyQt/Account>

ConversationDatePicker::ConversationDatePicker(QWidget *parent) :
    KDatePicker(parent)
{
}

void ConversationDatePicker::setEntity(const Tp::AccountPtr &account, const Tpl::EntityPtr &entity)
{
    clear();

    m_account = account;
    m_entity = entity;

    if (!m_searchHits.isEmpty()) {
        setDatesFromSearchHits();
        updatePaintedDates();
    } else {
        Tpl::LogManagerPtr logManager = Tpl::LogManager::instance();
        Tpl::PendingDates *pendingDates = logManager->queryDates(account, entity, Tpl::EventTypeMaskText);
        connect(pendingDates, SIGNAL(finished(Tpl::PendingOperation*)), SLOT(onDatesFinished(Tpl::PendingOperation*)));
    }
}

void ConversationDatePicker::clear()
{
    //this could really do with existing in KDateTable!
    //it only needs to call d->m_customPaintingModes.clear();
    //FIXME push this into KDE Frameworks 5

    Q_FOREACH(const QDate &date, m_setDates) {
        dateTable()->unsetCustomDatePainting(date);
    }
}

void ConversationDatePicker::setSearchHits(const Tpl::SearchHitList &searchHits)
{
    m_searchHits = searchHits;

    setDatesFromSearchHits();
    updatePaintedDates();
}


void ConversationDatePicker::clearSearchHits()
{
    m_searchHits.clear();
    updatePaintedDates();
}


QDate ConversationDatePicker::nextDate() const
{
    QList<QDate>::ConstIterator iter = qUpperBound(m_setDates, date());
    if (iter != m_setDates.constEnd()) {
        return *iter;
    }

     return QDate();
}

QDate ConversationDatePicker::previousDate() const
{
    QList<QDate>::ConstIterator iter = qLowerBound(m_setDates, date());
    if (iter != m_setDates.constBegin()) {
        return *(iter - 1);
    }

    return QDate();
}

const QList<QDate>& ConversationDatePicker::validDates() const
{
    return m_setDates;
}

void ConversationDatePicker::onDatesFinished(Tpl::PendingOperation *op)
{
    Tpl::PendingDates *pendingDates = qobject_cast<Tpl::PendingDates*>(op);
    m_setDates = pendingDates->dates();

    qSort(m_setDates);
    updatePaintedDates();
}

void ConversationDatePicker::updatePaintedDates()
{
    clear();

    Q_FOREACH(const QDate &date, m_setDates) {
        dateTable()->setCustomDatePainting(date, Qt::blue);
    }
}

void ConversationDatePicker::setDatesFromSearchHits()
{
    m_setDates.clear();

    if (m_account.isNull() || m_entity.isNull()) {
        return;
    }

    Q_FOREACH (const Tpl::SearchHit &searchHit, m_searchHits) {
        if ((searchHit.account()->uniqueIdentifier() == m_account->uniqueIdentifier()) &&
            (searchHit.target()->identifier() == m_entity->identifier())) {
                m_setDates << searchHit.date();
        }
    }

    qSort(m_setDates);
}
