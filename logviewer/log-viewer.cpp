/***************************************************************************
 *   Copyright (C) 2012 by David Edmundson <kde@davidedmundson.co.uk>      *
 *   Copyright (C) 2012,2013 by Daniel Vrátil <dvratil@redhat.com>         *
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

#include <KTp/Logger/log-manager.h>
#include <KTp/Logger/log-entity.h>
#include <KTp/Logger/pending-logger-search.h>

#include <KTp/logs-importer.h>
#include <KTp/contact.h>

#include <QSortFilterProxyModel>
#include <QWebFrame>
#include <KLineEdit>
#include <KPixmapSequence>
#include <KMessageBox>

#include <KDebug>
#include <KStandardAction>
#include <KMenu>
#include <KApplication>
#include <KAction>
#include <KActionCollection>
#include <KMenuBar>

#include "entity-model.h"
#include "entity-proxy-model.h"
#include "entity-model-item.h"
#include "logs-import-dialog.h"

Q_DECLARE_METATYPE(QModelIndex)

LogViewer::LogViewer(const Tp::AccountFactoryPtr &accountFactory, const Tp::ConnectionFactoryPtr &connectionFactory,
                     const Tp::ChannelFactoryPtr &channelFactory, const Tp::ContactFactoryPtr &contactFactory,
                     QWidget *parent):
    KXmlGuiWindow(parent),
    ui(new Ui::LogViewer)
{
    setWindowIcon(KIcon(QLatin1String("documentation")));

    QWidget *widget = new QWidget(this);
    setCentralWidget(widget);
    ui->setupUi(widget);

    setupActions();
    setupGUI(Keys | Save | Create, QLatin1String("log-viewer.rc"));

    /* Setup AccountManager */
    m_accountManager = Tp::AccountManager::create(accountFactory, connectionFactory, channelFactory, contactFactory);
    connect(m_accountManager->becomeReady(), SIGNAL(finished(Tp::PendingOperation*)), SLOT(onAccountManagerReady()));

    m_entityModel = new EntityModel(this);
    m_filterModel = new EntityProxyModel(this);
    m_filterModel->setSourceModel(m_entityModel);

    ui->entityList->setModel(m_filterModel);
    ui->entityList->setItemsExpandable(true);
    ui->entityList->setRootIsDecorated(true);
    ui->entityList->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->entityFilter->setProxy(m_filterModel);
    ui->entityFilter->lineEdit()->setClickMessage(i18nc("Placeholder text in line edit for filtering contacts", "Filter contacts..."));

    connect(ui->entityList->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), SLOT(onEntitySelected(QModelIndex,QModelIndex)));
    connect(ui->datePicker, SIGNAL(dateChanged(QDate)), SLOT(onDateSelected()));
    connect(ui->messageView, SIGNAL(conversationSwitchRequested(QDate)), SLOT(slotSetConversationDate(QDate)));
    connect(ui->globalSearch, SIGNAL(returnPressed(QString)), SLOT(slotStartGlobalSearch(QString)));
    connect(ui->globalSearch, SIGNAL(clearButtonClicked()), SLOT(slotClearGlobalSearch()));
    connect(ui->entityList, SIGNAL(customContextMenuRequested(QPoint)), SLOT(slotShowEntityListContextMenu(QPoint)));
    connect(ui->entityList, SIGNAL(noSuchContact()), SLOT(slotNoLogsForContact()));
}

LogViewer::~LogViewer()
{
}

void LogViewer::setupActions()
{
    KStandardAction::quit(KApplication::instance(), SLOT(quit()), actionCollection());
    KStandardAction::showMenubar(this->menuBar(), SLOT(setVisible(bool)), actionCollection());

    KAction *clearAccHistory = new KAction(i18n("Clear &account history"), this);
    clearAccHistory->setIcon(KIcon(QLatin1String("edit-clear-history")));
    connect(clearAccHistory, SIGNAL(triggered(bool)), SLOT(slotClearAccountHistory()));

    KAction *clearContactHistory = new KAction(i18n("Clear &contact history"), this);
    clearContactHistory->setIcon(KIcon(QLatin1String("edit-clear-history")));
    clearContactHistory->setEnabled(false);
    connect(clearContactHistory, SIGNAL(triggered(bool)), SLOT(slotClearContactHistory()));

    KAction *importKopeteLogs = new KAction(i18n("&Import Kopete Logs"), this);
    importKopeteLogs->setIcon(KIcon(QLatin1String("document-import")));
    connect(importKopeteLogs, SIGNAL(triggered(bool)), SLOT(slotImportKopeteLogs()));

    KAction *prevConversation = new KAction(i18n("&Previous Conversation"), this);
    prevConversation->setShortcut(KShortcut(Qt::CTRL + Qt::Key_P));
    prevConversation->setIcon(KIcon(QLatin1String("go-previous")));
    prevConversation->setEnabled(false);
    connect(prevConversation, SIGNAL(triggered(bool)), SLOT(slotJumpToPrevConversation()));

    KAction *nextConversation = new KAction(i18n("&Next Conversation"), this);
    nextConversation->setShortcut(KShortcut(Qt::CTRL + Qt::Key_N));
    nextConversation->setIcon(KIcon(QLatin1String("go-next")));
    nextConversation->setEnabled(false);
    connect(nextConversation, SIGNAL(triggered(bool)), SLOT(slotJumpToNextConversation()));

    actionCollection()->addAction(QLatin1String("clear-account-logs"), clearAccHistory);
    actionCollection()->addAction(QLatin1String("clear-contact-logs"), clearContactHistory);
    actionCollection()->addAction(QLatin1String("import-kopete-logs"), importKopeteLogs);
    actionCollection()->addAction(QLatin1String("jump-prev-conversation"), prevConversation);
    actionCollection()->addAction(QLatin1String("jump-next-conversation"), nextConversation);

    /* Build the popup menu for entity list */
    m_entityListContextMenu = new KMenu(ui->entityList);
    m_entityListContextMenu->addAction(clearContactHistory);
    m_entityListContextMenu->addAction(clearAccHistory);
}

void LogViewer::onAccountManagerReady()
{
    KTp::LogManager *logManger = KTp::LogManager::instance();
    logManger->setAccountManager(m_accountManager);
    m_entityModel->setAccountManager(m_accountManager);

    /* Try to run log import */
    slotImportKopeteLogs(false);
}

void LogViewer::onEntitySelected(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);

    /* Ignore account nodes */
    if (current.parent() == QModelIndex()) {
        actionCollection()->action(QLatin1String("clear-contact-logs"))->setEnabled(false);
        return;
    }

    KTp::LogEntity entity = current.data(EntityModel::EntityRole).value<KTp::LogEntity>();
    Tp::AccountPtr account = current.data(EntityModel::AccountRole).value<Tp::AccountPtr>();

    if (!account.isNull() && entity.isValid()) {
        actionCollection()->action(QLatin1String("clear-contact-logs"))->setEnabled(true);
    }

    ui->datePicker->setEntity(account, entity);
}

void LogViewer::onDateSelected()
{
    slotUpdateMainWindow();
}

void LogViewer::slotUpdateMainWindow()
{
    QModelIndex currentIndex = ui->entityList->currentIndex();

    if (!currentIndex.isValid()) {
        return;
    }

    /* If the selected date is not within valid (highlighted) dates then display empty
     * conversation (even if there is a chat log for that particular date) */
    QDate date = ui->datePicker->date();
    if (!ui->datePicker->validDates().contains(date)) {
        date = QDate();
    }

    m_prevConversationDate = ui->datePicker->previousDate();
    m_nextConversationDate = ui->datePicker->nextDate();

    actionCollection()->action(QLatin1String("jump-prev-conversation"))->setEnabled(m_prevConversationDate.isValid());
    actionCollection()->action(QLatin1String("jump-next-conversation"))->setEnabled(m_nextConversationDate.isValid());

    QPair< QDate, QDate > nearestDates;
    nearestDates.first = m_prevConversationDate;
    nearestDates.second = m_nextConversationDate;

    KTp::LogEntity entity = currentIndex.data(EntityModel::EntityRole).value<KTp::LogEntity>();
    KTp::ContactPtr contact = currentIndex.data(EntityModel::ContactRole).value<KTp::ContactPtr>();
    Tp::AccountPtr account = currentIndex.data(EntityModel::AccountRole).value<Tp::AccountPtr>();
    ui->messageView->loadLog(account, entity, contact, date, nearestDates);
}

void LogViewer::slotSetConversationDate(const QDate &date)
{
    ui->datePicker->setDate(date);
}

void LogViewer::slotStartGlobalSearch(const QString &term)
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

    KTp::LogManager *manager = KTp::LogManager::instance();
    KTp::PendingLoggerSearch *search = manager->search(term);
    connect(search, SIGNAL(finished(KTp::PendingLoggerOperation*)),
            this, SLOT(onGlobalSearchFinished(KTp::PendingLoggerOperation*)));
}

void LogViewer::onGlobalSearchFinished(KTp::PendingLoggerOperation *operation)
{
    KTp::PendingLoggerSearch *search = qobject_cast<KTp::PendingLoggerSearch*>(operation);
    Q_ASSERT(search);

    m_filterModel->setSearchHits(search->searchHits());
    ui->datePicker->setSearchHits(search->searchHits());

    ui->globalSearch->setEnabled(true);
    ui->busyAnimation->setSequence(KPixmapSequence());
    ui->busyAnimation->setVisible(false);
}

void LogViewer::slotClearGlobalSearch()
{
    m_filterModel->clearSearchHits();
    ui->datePicker->clearSearchHits();
    ui->messageView->clearHighlightText();
}

void LogViewer::slotShowEntityListContextMenu (const QPoint &coords)
{
    QModelIndex index = ui->entityList->indexAt(coords);
    if (!index.isValid()) {
        return;
    }
    index = m_filterModel->mapToSource(index);

    /* Whether the node is an account or a contact */
    actionCollection()->action(QLatin1String("clear-contact-logs"))->setEnabled((index.parent() != QModelIndex()));

    m_entityListContextMenu->setProperty("index", QVariant::fromValue(index));
    m_entityListContextMenu->popup(ui->entityList->mapToGlobal(coords));
}

void LogViewer::slotClearAccountHistory()
{
    QModelIndex index = ui->entityList->currentIndex();

    /* Usually a contact node is selected, so traverse up to it's parent
     * account node */
    if (index.parent().isValid()) {
        index = index.parent();
    }

    if (!index.isValid()) {
        return;
    }

    Tp::AccountPtr account = index.data(EntityModel::AccountRole).value<Tp::AccountPtr>();
    if (account.isNull()) {
        return;
    }

    if (KMessageBox::warningYesNo(
            this, i18n("Are you sure you want to remove all logs from account %1?", account->displayName()),
            i18n("Clear account history"), KStandardGuiItem::del(), KStandardGuiItem::cancel(),
            QString(), KMessageBox::Dangerous) == KMessageBox::No) {
        return;
    }

    KTp::LogManager::instance()->clearAccountLogs(account);

    QModelIndex parent = index.parent();
    m_entityModel->removeRow(index.row(), parent);

    // If last entity was removed then remove the account node as well
    if (parent.isValid() && !m_entityModel->hasChildren(parent)) {
        m_entityModel->removeRow(parent.row(), parent.parent());
    }

    m_entityListContextMenu->setProperty("index", QVariant());
}

void LogViewer::slotClearContactHistory()
{
    QModelIndex index = ui->entityList->currentIndex();
    if (!index.isValid()) {
        return;
    }

    Tp::AccountPtr account = index.data(EntityModel::AccountRole).value<Tp::AccountPtr>();
    KTp::LogEntity entity = index.data(EntityModel::EntityRole).value<KTp::LogEntity>();
    if (account.isNull() || !entity.isValid()) {
        return;
    }

    QString name = index.data(Qt::DisplayRole).toString();
    if (KMessageBox::warningYesNo(
            this, i18n("Are you sure you want to remove history of all conversations with %1?", name),
            i18n("Clear contact history"), KStandardGuiItem::del(), KStandardGuiItem::cancel(),
            QString(), KMessageBox::Dangerous) == KMessageBox::No) {
        return;
    }

    KTp::LogManager::instance()->clearContactLogs(account, entity);

    QModelIndex parent = index.parent();
    m_entityModel->removeRow(index.row(), parent);

    // If last entity was removed then remove the account node as well
    if (parent.isValid() && !m_entityModel->hasChildren(parent)) {
        m_entityModel->removeRow(parent.row(), parent.parent());
    }

    m_entityListContextMenu->setProperty("index", QVariant());
}

void LogViewer::slotImportKopeteLogs(bool force)
{
    KSharedConfigPtr config = KSharedConfig::openConfig(QLatin1String("ktelepathyrc"));
    KConfigGroup logsConfig = config->group("logs");

    bool importDone = logsConfig.readEntry(QLatin1String("InitialKopeteImportDone"), QVariant(false)).toBool();
    if (!force && importDone) {
        kDebug() << "Skipping initial Kopete logs import, already done.";
        return;
    }

    QList<Tp::AccountPtr> accounts = m_accountManager->allAccounts();
    QList<Tp::AccountPtr> matchingAccounts;
    KTp::LogsImporter importer;

    Q_FOREACH (const Tp::AccountPtr &account, accounts) {
        if (importer.hasKopeteLogs(account)) {
            matchingAccounts << account;
        }
    }

    kDebug() << "Initial Kopete logs import: found" << matchingAccounts.count() << "accounts to import";

    if (!matchingAccounts.isEmpty()) {
        LogsImportDialog *dialog = new LogsImportDialog(this);
        dialog->importLogs(matchingAccounts);
    }

    logsConfig.writeEntry(QLatin1String("InitialKopeteImportDone"), true);
}

void LogViewer::slotJumpToNextConversation()
{
    if (!m_nextConversationDate.isValid()) {
        return;
    }

    ui->datePicker->setDate(m_nextConversationDate);
}

void LogViewer::slotJumpToPrevConversation()
{
    if (!m_prevConversationDate.isValid()) {
        return;
    }

    ui->datePicker->setDate(m_prevConversationDate);
}

void LogViewer::slotNoLogsForContact()
{
    ui->messageView->showInfoMessage(i18n("There are no logs for this contact"));
}
