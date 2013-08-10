/***************************************************************************
 *   Copyright (C) 2012 by David Edmundson <kde@davidedmundson.co.uk>      *
 *   Copyright (C) 2013 by Daniel Vrátil <dvratil@redhat.com>              *
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

#ifndef CONVERSATIONDATEPICKER_H
#define CONVERSATIONDATEPICKER_H

#include <KDatePicker>

#include <KTp/Logger/log-entity.h>
#include <KTp/Logger/log-search-hit.h>

#include <TelepathyQt/Types>

namespace KTp {
class PendingLoggerOperation;
}

class ConversationDatePicker : public KDatePicker
{
    Q_OBJECT
public:
    explicit ConversationDatePicker(QWidget *parent = 0);

    void setEntity(const Tp::AccountPtr &accout, const KTp::LogEntity &entity);
    void clear();

    void setSearchHits(const QList<KTp::LogSearchHit> &searchHits);
    void clearSearchHits();

    QDate previousDate() const;
    QDate nextDate() const;
    const QList<QDate>& validDates() const;

private Q_SLOTS:
    void onDatesFinished(KTp::PendingLoggerOperation *op);

private:
    void updatePaintedDates();
    void setDatesFromSearchHits();

    Tp::AccountPtr m_account;
    KTp::LogEntity m_entity;
    QList<KTp::LogSearchHit> m_searchHits;

    QList< QDate > m_setDates;
};

#endif // CONVERSATIONDATEPICKER_H
