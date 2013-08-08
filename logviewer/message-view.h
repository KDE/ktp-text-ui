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

#include <KTp/Logger/log-entity.h>
#include <KTp/Logger/log-message.h>
#include <KTp/Logger/pending-logger-operation.h>

class QLabel;

class MessageView : public AdiumThemeView
{
    Q_OBJECT
public:
    explicit MessageView(QWidget *parent = 0);

    void loadLog(const Tp::AccountPtr &account, const KTp::LogEntity &entity,
                 const Tp::ContactPtr &contact, const QDate &date,
                 const QPair< QDate, QDate > &nearestDates);

    void setHighlightText(const QString &text);
    void clearHighlightText();

    void showInfoMessage(const QString &message);

public Q_SLOTS:
    void onLinkClicked(const QUrl &link);

private Q_SLOTS:
    void onEventsLoaded(KTp::PendingLoggerOperation* po);
    void doHighlightText();
    void processStoredEvents();

Q_SIGNALS:
    void conversationSwitchRequested(const QDate &date);

protected:
    virtual void resizeEvent(QResizeEvent *e);

private:
    KTp::LogEntity m_entity;
    Tp::AccountPtr m_account;
    Tp::ContactPtr m_contact;
    QDate m_date;
    QDate m_prev;
    QDate m_next;

    QString m_highlightedText;

    QList<KTp::LogMessage> m_events;

    QString m_accountAvatar;

    QLabel *m_infoLabel;
};

#endif // MESSAGEVIEW_H
