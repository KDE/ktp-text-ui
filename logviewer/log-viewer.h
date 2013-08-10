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

#ifndef LOGVIEWER_H
#define LOGVIEWER_H

#include <KXmlGuiWindow>
#include <TelepathyQt/Types>

namespace KTp {
class PendingLoggerOperation;
}

namespace Ui {
    class LogViewer;
}

class EntityModel;
class EntityProxyModel;
class KMenu;

class LogViewer : public KXmlGuiWindow
{
    Q_OBJECT

public:
    explicit LogViewer(const Tp::AccountFactoryPtr &accountFactory, const Tp::ConnectionFactoryPtr &connectionFactory,
                       const Tp::ChannelFactoryPtr &channelFactory, const Tp::ContactFactoryPtr &contactFactory,
                       QWidget *parent = 0);
    ~LogViewer();
private Q_SLOTS:
    void onAccountManagerReady();

    void onEntitySelected(const QModelIndex &current, const QModelIndex &previous);
    void onDateSelected();

    void slotUpdateMainWindow();
    void slotSetConversationDate(const QDate &date);

    void slotShowEntityListContextMenu(const QPoint &coords);
    void slotClearGlobalSearch();
    void slotStartGlobalSearch(const QString &term);
    void onGlobalSearchFinished(KTp::PendingLoggerOperation *);

    void slotClearAccountHistory();
    void slotClearContactHistory();

    void slotImportKopeteLogs(bool force = true);

    void slotJumpToPrevConversation();
    void slotJumpToNextConversation();

    void slotNoLogsForContact();

private:
    void setupActions();

    Ui::LogViewer *ui;
    Tp::AccountManagerPtr m_accountManager;
    EntityModel *m_entityModel;
    EntityProxyModel *m_filterModel;

    KMenu *m_entityListContextMenu;

    QDate m_prevConversationDate;
    QDate m_nextConversationDate;
};

#endif // LOGVIEWER_H
