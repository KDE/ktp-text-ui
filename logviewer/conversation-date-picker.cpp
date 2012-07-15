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

#include <KDateTable>

ConversationDatePicker::ConversationDatePicker(QWidget *parent) :
    KDatePicker(parent)
{
}

void ConversationDatePicker::setEntity(const Tp::AccountPtr &account, const Tpl::EntityPtr &entity)
{
    clear();
    Tpl::LogManagerPtr logManager = Tpl::LogManager::instance();
    Tpl::PendingDates *pendingDates = logManager->queryDates(account, entity, Tpl::EventTypeMaskText);
    connect(pendingDates, SIGNAL(finished(Tpl::PendingOperation*)), SLOT(onDatesFinished(Tpl::PendingOperation*)));
}

void ConversationDatePicker::clear()
{
    //this could really do with existing in KDateTable!
    //it only needs to call d->m_customPaintingModes.clear();
    //FIXME push this into KDE Frameworks 5

    Q_FOREACH(const QDate &date, m_setDates) {
        dateTable()->unsetCustomDatePainting(date);
    }
    m_setDates.clear();
}

QDate ConversationDatePicker::nextDate() const
{
    int i = m_setDates.indexOf(date());
    if ((i < m_setDates.count() - 1) && (i > -1)) {
        return m_setDates.at(i + 1);
    }

    return QDate();
}

QDate ConversationDatePicker::previousDate() const
{
    int i = m_setDates.indexOf(date());
    if (i > 0) {
        return m_setDates.at(i - 1);
    }

    return QDate();
}

void ConversationDatePicker::onDatesFinished(Tpl::PendingOperation *op)
{
    Tpl::PendingDates *pendingDates = qobject_cast<Tpl::PendingDates*>(op);
    Q_FOREACH(const QDate &date, pendingDates->dates()) {
        dateTable()->setCustomDatePainting(date, Qt::blue);
    }
    m_setDates.append(pendingDates->dates());
}
