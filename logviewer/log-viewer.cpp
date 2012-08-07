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

#include "log-viewer.h"
#include "ui_log-viewer.h"

#include <TelepathyQt/AccountManager>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/ContactManager>


#include <TelepathyLoggerQt4/Init>
#include <TelepathyLoggerQt4/Entity>
#include <TelepathyLoggerQt4/LogManager>
#include <TelepathyLoggerQt4/SearchHit>
#include <TelepathyLoggerQt4/PendingSearch>

#include <QSortFilterProxyModel>
#include <QWebFrame>
#include <KLineEdit>
#include <KPixmapSequence>

#include "entity-model.h"
#include "entity-proxy-model.h"
#include "entity-model-item.h"

LogViewer::LogViewer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LogViewer)
{
    ui->setupUi(this);
    setWindowIcon(KIcon(QLatin1String("documentation")));
    Tp::registerTypes();
    Tpl::init();

    Tp::AccountFactoryPtr  accountFactory = Tp::AccountFactory::create(
                                                QDBusConnection::sessionBus(),
                                                Tp::Features() << Tp::Account::FeatureCore);

    Tp::ConnectionFactoryPtr connectionFactory = Tp::ConnectionFactory::create(
                                                QDBusConnection::sessionBus(),
                                                Tp::Features() << Tp::Connection::FeatureCore
                                                    << Tp::Connection::FeatureSelfContact
                                                    << Tp::Connection::FeatureRoster);

    Tp::ContactFactoryPtr contactFactory = Tp::ContactFactory::create(
                                                Tp::Features()  << Tp::Contact::FeatureAlias
                                                    << Tp::Contact::FeatureAvatarData
                                                    << Tp::Contact::FeatureSimplePresence
                                                    << Tp::Contact::FeatureCapabilities);

    Tp::ChannelFactoryPtr channelFactory = Tp::ChannelFactory::create(QDBusConnection::sessionBus());

    m_accountManager = Tp::AccountManager::create(accountFactory, connectionFactory, channelFactory, contactFactory);

    m_entityModel = new EntityModel(this);
    m_filterModel = new EntityProxyModel(this);
    m_filterModel->setSourceModel(m_entityModel);

    ui->entityList->setModel(m_filterModel);
    ui->entityList->setItemsExpandable(true);
    ui->entityList->setRootIsDecorated(true);
    ui->entityFilter->setProxy(m_filterModel);
    ui->entityFilter->lineEdit()->setClickMessage(i18nc("Placeholder text in line edit for filtering contacts", "Filter contacts..."));

    //TODO parse command line args and update all views as appropriate

    connect(m_accountManager->becomeReady(), SIGNAL(finished(Tp::PendingOperation*)), SLOT(onAccountManagerReady()));
    connect(ui->entityList->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), SLOT(onEntitySelected(QModelIndex,QModelIndex)));
    connect(ui->datePicker, SIGNAL(dateChanged(QDate)), SLOT(onDateSelected()));
    connect(ui->messageView, SIGNAL(conversationSwitchRequested(QDate)), SLOT(switchConversation(QDate)));
    connect(ui->globalSearch, SIGNAL(returnPressed(QString)), SLOT(startGlobalSearch(QString)));
    connect(ui->globalSearch, SIGNAL(clearButtonClicked()), SLOT(clearGlobalSearch()));
}

LogViewer::~LogViewer()
{
    delete ui;
}

void LogViewer::onAccountManagerReady()
{
    Tpl::LogManagerPtr logManager = Tpl::LogManager::instance();
    logManager->setAccountManagerPtr(m_accountManager);
    m_entityModel->setAccountManager(m_accountManager);
}

void LogViewer::onEntitySelected(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);

    /* Ignore account nodes */
    if (current.parent() == QModelIndex()) {
        return;
    }

    Tpl::EntityPtr entity = current.data(EntityModel::EntityRole).value<Tpl::EntityPtr>();
    Tp::AccountPtr account = current.data(EntityModel::AccountRole).value<Tp::AccountPtr>();

    ui->datePicker->setEntity(account, entity);

    updateMainView();
}

void LogViewer::onDateSelected()
{
    updateMainView();
}

void LogViewer::updateMainView()
{
    QModelIndex currentIndex = ui->entityList->currentIndex();

    if (!currentIndex.isValid()) {
        return;
    }

    QPair< QDate, QDate > nearestDates;

    /* If the selected date is not within valid (highlighted) dates then display empty
     * conversation (even if there is a chat log for that particular date) */
    QDate date = ui->datePicker->date();
    if (!ui->datePicker->validDates().contains(date)) {
        date = QDate();
    }

    nearestDates.first = ui->datePicker->previousDate();
    nearestDates.second = ui->datePicker->nextDate();

    Tpl::EntityPtr entity = currentIndex.data(EntityModel::EntityRole).value<Tpl::EntityPtr>();
    Tp::AccountPtr account = currentIndex.data(EntityModel::AccountRole).value<Tp::AccountPtr>();
    ui->messageView->loadLog(account, entity, date, nearestDates);
}

void LogViewer::switchConversation(const QDate &date)
{
    ui->datePicker->setDate(date);
}

void LogViewer::startGlobalSearch(const QString &term)
{
    if (term.isEmpty()) {
        ui->messageView->clearHighlightText();
        m_filterModel->clearSearchHits();
        ui->datePicker->clearSearchHits();
        return;
    }

    ui->busyAnimation->setSequence(KPixmapSequence(QLatin1String("process-working"), KIconLoader::SizeSmallMedium));
    ui->busyAnimation->setVisible(true);
    ui->globalSearch->setDisabled(true);

    ui->messageView->setHighlightText(term);

    Tpl::LogManagerPtr manager = Tpl::LogManager::instance();
    Tpl::PendingSearch *search = manager->search(term, Tpl::EventTypeMaskAny);
    connect (search, SIGNAL(finished(Tpl::PendingOperation*)),
             this, SLOT(globalSearchFinished(Tpl::PendingOperation*)));
}

void LogViewer::globalSearchFinished(Tpl::PendingOperation *operation)
{
    Tpl::PendingSearch *search = qobject_cast< Tpl::PendingSearch* >(operation);
    Q_ASSERT(search);

    m_filterModel->setSearchHits(search->hits());
    ui->datePicker->setSearchHits(search->hits());

    ui->globalSearch->setEnabled(true);
    ui->busyAnimation->setSequence(KPixmapSequence());
    ui->busyAnimation->setVisible(false);
}

void LogViewer::clearGlobalSearch()
{
    m_filterModel->clearSearchHits();
    ui->datePicker->clearSearchHits();
    ui->messageView->clearHighlightText();
}
