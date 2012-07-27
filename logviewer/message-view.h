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

#ifndef MESSAGEVIEW_H
#define MESSAGEVIEW_H

#include "adium-theme-view.h"

#include <QDate>

#include <TelepathyLoggerQt4/Entity>
#include <TelepathyLoggerQt4/PendingOperation>


class MessageView : public AdiumThemeView
{
    Q_OBJECT
public:
    explicit MessageView(QWidget *parent = 0);

    void loadLog(const Tp::AccountPtr &account, const Tpl::EntityPtr &entity,
                 const QDate &date, const QPair< QDate, QDate > &nearestDates);

    void setHighlightText(const QString &text);
    void clearHighlightText();

public Q_SLOTS:
    void onLinkClicked(const QUrl &link);

private Q_SLOTS:
   void onLoadFinished();
   void onEventsLoaded(Tpl::PendingOperation* po);
   void doHighlightText();

Q_SIGNALS:
    void conversationSwitchRequested(const QDate &date);

private:
    Tpl::EntityPtr m_entity;
    Tp::AccountPtr m_account;
    QDate m_date;
    QDate m_prev;
    QDate m_next;

    QString m_highlightedText;

};

#endif // MESSAGEVIEW_H
