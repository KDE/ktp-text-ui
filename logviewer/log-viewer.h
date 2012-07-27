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

#ifndef LOGVIEWER_H
#define LOGVIEWER_H

#include <QWidget>
#include <TelepathyQt/AccountManager>
#include <TelepathyLoggerQt4/Types>

namespace Ui {
    class LogViewer;
}

namespace Tpl {
    class PendingOperation;
}

class EntityModel;
class EntityProxyModel;

class LogViewer : public QWidget
{
    Q_OBJECT

public:
    explicit LogViewer(QWidget *parent = 0);
    ~LogViewer();
private Q_SLOTS:
    void onAccountManagerReady();

    void onEntitySelected(const QModelIndex &current, const QModelIndex &previous);
    void onDateSelected();

    void updateMainView();
    void switchConversation(const QDate &date);

    void startGlobalSearch(const QString &term);
    void globalSearchFinished(Tpl::PendingOperation *);
    void clearGlobalSearch();

private:
    Ui::LogViewer *ui;
    Tp::AccountManagerPtr m_accountManager;
    EntityModel *m_entityModel;
    EntityProxyModel *m_filterModel;
};

#endif // LOGVIEWER_H
