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

#include <KTp/logs-importer.h>

#include <kpeople/persons-model.h>
#include <kpeople/persons-presence-model.h>
#include <kpeople/ktp-translation-proxy.h>

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
#include "logs-import-dialog.h"
#include "person-entity-merge-model.h"
#include "entity-filter-model.h"
#include "entity-view-delegate.h"
#include "dates-model.h"
#include "dates-view-delegate.h"

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

    m_personsModel = new PersonsModel(PersonsModel::FeatureIM | PersonsModel::FeatureContactUID,
                                      PersonsModel::FeatureAvatars | PersonsModel::FeatureFullName | PersonsModel::FeatureGroups,
                                      this);

    m_presenceModel = new PersonsPresenceModel(this);
    m_presenceModel->setSourceModel(m_personsModel);

    m_entityModel = new EntityModel(this);

    m_mergeModel = new PersonEntityMergeModel(m_personsModel, m_entityModel, this);

    m_filterModel = new EntityFilterModel(this);
    m_filterModel->setSourceModel(m_mergeModel);

    ui->entityList->setModel(m_filterModel);
    ui->entityList->setItemDelegate(new EntityViewDelegate(ui->entityList));
    ui->entityList->setItemsExpandable(true);
    ui->entityList->setRootIsDecorated(false);
    ui->entityList->setExpandsOnDoubleClick(false);
    ui->entityList->setIndentation(0);
    ui->entityList->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->entityFilter->setProxy(m_filterModel);
    ui->entityFilter->lineEdit()->setClickMessage(i18nc("Placeholder text in line edit for filtering contacts", "Filter contacts..."));

    m_datesModel = new DatesModel(this);
    ui->datesView->setModel(m_datesModel);
    ui->datesView->setItemDelegate(new DatesViewDelegate(ui->datesView));
    ui->datesView->setItemsExpandable(true);
    ui->datesView->setRootIsDecorated(false);
    ui->datesView->setExpandsOnDoubleClick(false);
    ui->datesView->setIndentation(0);

    connect(ui->entityList->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), SLOT(onEntitySelected(QModelIndex,QModelIndex)));
    connect(ui->datesView, SIGNAL(clicked(QModelIndex)), SLOT(slotDateClicked(QModelIndex)));
    connect(ui->datesView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), SLOT(slotUpdateMainWindow()));
    connect(ui->messageView, SIGNAL(conversationSwitchRequested(QDate)), SLOT(slotSetConversationDate(QDate)));
    connect(ui->globalSearch, SIGNAL(returnPressed(QString)), SLOT(slotStartGlobalSearch(QString)));
    connect(ui->globalSearch, SIGNAL(clearButtonClicked()), SLOT(slotClearGlobalSearch()));
    connect(ui->entityList, SIGNAL(customContextMenuRequested(QPoint)), SLOT(slotShowEntityListContextMenu(QPoint)));
}

LogViewer::~LogViewer()
{
}

void LogViewer::setupActions()
{
    KStandardAction::quit(KApplication::instance(), SLOT(quit()), actionCollection());
    KStandardAction::showMenubar(this->menuBar(), SLOT(setVisible(bool)), actionCollection());

    /*
    KAction *clearAccHistory = new KAction(i18n("Clear &account history"), this);
    clearAccHistory->setIcon(KIcon(QLatin1String("edit-clear-history")));
    connect(clearAccHistory, SIGNAL(triggered(bool)), SLOT(slotClearAccountHistory()));

    KAction *clearContactHistory = new KAction(i18n("Clear &contact history"), this);
    clearContactHistory->setIcon(KIcon(QLatin1String("edit-clear-history")));
    clearContactHistory->setEnabled(false);
    connect(clearContactHistory, SIGNAL(triggered(bool)), SLOT(slotClearContactHistory()));
    */

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

    /*
    actionCollection()->addAction(QLatin1String("clear-contact-logs"), clearContactHistory);
    */
    actionCollection()->addAction(QLatin1String("import-kopete-logs"), importKopeteLogs);
    actionCollection()->addAction(QLatin1String("jump-prev-conversation"), prevConversation);
    actionCollection()->addAction(QLatin1String("jump-next-conversation"), nextConversation);

    /* Build the popup menu for entity list */
    m_entityListContextMenu = new KMenu(ui->entityList);
    /*
    m_entityListContextMenu->addAction(clearContactHistory);
    m_entityListContextMenu->addAction(clearAccHistory);
    */
}

void LogViewer::onAccountManagerReady()
{
    m_entityModel->setAccountManager(m_accountManager);

    /* Try to run log import */
    slotImportKopeteLogs(false);
}

void LogViewer::onEntitySelected(const QModelIndex &current, const QModelIndex &previous)
{
    if (previous.data(PersonEntityMergeModel::ItemTypeRole).toUInt() == PersonEntityMergeModel::Persona &&
        current.parent() != previous)
    {
        ui->entityList->collapse(previous);
    } else if (previous.parent().data(PersonEntityMergeModel::ItemTypeRole).toUInt() == PersonEntityMergeModel::Persona) {
        ui->entityList->collapse(previous.parent());
    }

    if (current.data(PersonEntityMergeModel::ItemTypeRole).toUInt() == PersonEntityMergeModel::Group) {
        if (ui->entityList->isExpanded(current)) {
            ui->entityList->collapse(current);
        } else {
            ui->entityList->expand(current);
        }
        return;
    }

    // FIXME: Show something fancy when selecting a persona
    if (current.data(PersonEntityMergeModel::ItemTypeRole).toUInt() == PersonEntityMergeModel::Persona) {
        ui->entityList->expand(current);
        ui->messageView->clear();
        m_datesModel->clear();
        for (int i = 0; i < current.model()->rowCount(current); ++i) {
            const QModelIndex child = current.child(i, 0);
            m_datesModel->addEntity(child.data(PersonEntityMergeModel::AccountRole).value<Tp::AccountPtr>(),
                                    child.data(PersonEntityMergeModel::EntityRole).value<Tpl::EntityPtr>());
        }
        return;
    }

    Tpl::EntityPtr entity = current.data(PersonEntityMergeModel::EntityRole).value<Tpl::EntityPtr>();
    Tp::AccountPtr account = current.data(PersonEntityMergeModel::AccountRole).value<Tp::AccountPtr>();
    Q_ASSERT(!entity.isNull());
    Q_ASSERT(!account.isNull());
    /*
    if (!account.isNull() && !entity.isNull()) {
        actionCollection()->action(QLatin1String("clear-contact-logs"))->setEnabled(true);
    }
    */

    m_datesModel->setEntity(account, entity);
}

void LogViewer::slotDateClicked(const QModelIndex& index)
{
    if (ui->datesView->isExpanded(index)) {
        ui->datesView->collapse(index);
    } else {
        ui->datesView->expand(index);
    }
}

void LogViewer::slotUpdateMainWindow()
{
    const QModelIndex currentIndex = ui->entityList->currentIndex();

    if (!currentIndex.isValid()) {
        return;
    }

    /* If the selected date is not within valid (highlighted) dates then display empty
     * conversation (even if there is a chat log for that particular date) */
    QDate date;
    const QModelIndex currentDateIndex = ui->datesView->currentIndex();
    if (currentDateIndex.isValid()) {
        date = currentDateIndex.data(DatesModel::DateRole).toDate();
    }

    m_prevConversationDate = m_datesModel->previousDate(currentDateIndex);
    m_nextConversationDate = m_datesModel->nextDate(currentDateIndex);

    actionCollection()->action(QLatin1String("jump-prev-conversation"))->setEnabled(m_prevConversationDate.isValid());
    actionCollection()->action(QLatin1String("jump-next-conversation"))->setEnabled(m_nextConversationDate.isValid());

    QPair< QDate, QDate > nearestDates;
    nearestDates.first = m_prevConversationDate;
    nearestDates.second = m_nextConversationDate;

    Tpl::EntityPtr entity = currentDateIndex.data(DatesModel::EntityRole).value<Tpl::EntityPtr>();
    Tp::AccountPtr account = currentDateIndex.data(DatesModel::AccountRole).value<Tp::AccountPtr>();
    Tp::ContactPtr contact = currentIndex.data(PersonEntityMergeModel::ContactRole).value<Tp::ContactPtr>();
    if (entity.isNull() || account.isNull()) {
        ui->messageView->clear();
    } else {
        ui->messageView->loadLog(account, entity, contact, date, nearestDates);
    }
}

void LogViewer::slotSetConversationDate(const QDate &date)
{
    const QModelIndex index = m_datesModel->indexForDate(date);
    if (index.isValid()) {
        ui->datesView->setCurrentIndex(index);
        slotUpdateMainWindow();
    }
}

void LogViewer::slotStartGlobalSearch(const QString &term)
{
    if (term.isEmpty()) {
        ui->messageView->clearHighlightText();
        m_filterModel->clearSearchHits();
        m_datesModel->clearSearchHits();
        return;
    }

    ui->busyAnimation->setSequence(KPixmapSequence(QLatin1String("process-working"), KIconLoader::SizeSmallMedium));
    ui->busyAnimation->setVisible(true);
    ui->globalSearch->setDisabled(true);

    ui->messageView->setHighlightText(term);

    Tpl::LogManagerPtr manager = Tpl::LogManager::instance();
    Tpl::PendingSearch *search = manager->search(term, Tpl::EventTypeMaskAny);
    connect (search, SIGNAL(finished(Tpl::PendingOperation*)),
             this, SLOT(onGlobalSearchFinished(Tpl::PendingOperation*)));
}

void LogViewer::onGlobalSearchFinished(Tpl::PendingOperation *operation)
{
    Tpl::PendingSearch *search = qobject_cast< Tpl::PendingSearch* >(operation);
    Q_ASSERT(search);

    m_filterModel->setSearchHits(search->hits());
    m_datesModel->setSearchHits(search->hits());

    ui->globalSearch->setEnabled(true);
    ui->busyAnimation->setSequence(KPixmapSequence());
    ui->busyAnimation->setVisible(false);
}

void LogViewer::slotClearGlobalSearch()
{
    m_filterModel->clearSearchHits();
    m_datesModel->clearSearchHits();
    ui->messageView->clearHighlightText();
}

void LogViewer::slotShowEntityListContextMenu (const QPoint &coords)
{
    /*
    QModelIndex index = ui->entityList->indexAt(coords);
    if (!index.isValid()) {
        return;
    }

    m_entityListContextMenu->setProperty("index", QVariant::fromValue(index));
    m_entityListContextMenu->popup(ui->entityList->mapToGlobal(coords));
    */
}

void LogViewer::slotClearContactHistory()
{
    QModelIndex index = ui->entityList->currentIndex();
    if (!index.isValid()) {
        return;
    }

    Tp::AccountPtr account = index.data(PersonsModel::IMAccountRole).value<Tp::AccountPtr>();
    Tpl::EntityPtr entity = index.data(PersonEntityMergeModel::EntityRole).value<Tpl::EntityPtr>();
    if (account.isNull() || entity.isNull()) {
        return;
    }

    QString name = index.data(Qt::DisplayRole).toString();
    if (KMessageBox::warningYesNo(
            this, i18n("Are you sure you want to remove history of all conversations with %1?", name),
            i18n("Clear contact history"), KStandardGuiItem::del(), KStandardGuiItem::cancel(),
            QString(), KMessageBox::Dangerous) == KMessageBox::No) {
        return;
    }

    Tpl::LogManagerPtr logManager = Tpl::LogManager::instance();
    Tpl::PendingOperation *operation = logManager->clearEntityHistory(account, entity);
    connect(operation, SIGNAL(finished(Tpl::PendingOperation*)), SLOT(onLogClearingFinished(Tpl::PendingOperation*)));
}

void LogViewer::onLogClearingFinished(Tpl::PendingOperation *operation)
{
    if (!operation->errorName().isEmpty() || !operation->errorMessage().isEmpty()) {
        /* Make sure we display at least some message */
        QString msg = (operation->errorMessage().isEmpty()) ? operation->errorName() : operation->errorMessage();
        KMessageBox::sorry(this, msg, operation->errorName(), 0);
        return;
    }

    QModelIndex index = m_entityListContextMenu->property("index").value<QModelIndex>();
    if (!index.isValid()) {
        m_entityListContextMenu->setProperty("index", QVariant());
        return;
    }

    /*
    QModelIndex parent = index.parent();
    m_entityModel->removeRow(index.row(), parent);
    */
    /* If last entity was removed then remove the account node as well */
    /*
    if (parent.isValid() && !m_entityModel->hasChildren(parent)) {
        m_entityModel->removeRow(parent.row(), parent.parent());
    }
    */

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

    const QModelIndex index = m_datesModel->indexForDate(m_nextConversationDate);
    if (index.isValid()) {
        ui->datesView->setCurrentIndex(index);
        slotUpdateMainWindow();
    }
}

void LogViewer::slotJumpToPrevConversation()
{
    if (!m_prevConversationDate.isValid()) {
        return;
    }

    const QModelIndex index = m_datesModel->indexForDate(m_prevConversationDate);
    if (index.isValid()) {
        ui->datesView->setCurrentIndex(index);
        slotUpdateMainWindow();
    }
}
