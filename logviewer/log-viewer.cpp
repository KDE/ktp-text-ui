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
#include "debug.h"

#include <TelepathyQt/AccountManager>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/ContactManager>

#include <KTp/Logger/log-manager.h>
#include <KTp/Logger/log-entity.h>
#include <KTp/Logger/pending-logger-search.h>

#include <KTp/logs-importer.h>
#include <KTp/contact.h>

#include <KTp/Models/contacts-model.h>

#include <QWebFrame>
#include <KLineEdit>
#include <KPixmapSequence>
#include <KMessageBox>
#include <KStandardAction>
#include <KMenu>
#include <KApplication>
#include <KAction>
#include <KActionCollection>
#include <KMenuBar>
#include <KSettings/Dialog>
#include <KLocalizedString>
#include <KIconLoader>

#include <KCModuleProxy>

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
    setWindowIcon(QIcon::fromTheme(QStringLiteral("documentation")));

    QWidget *widget = new QWidget(this);
    setCentralWidget(widget);
    ui->setupUi(widget);

    setupActions();
    setupGUI(Keys | Save | Create, QLatin1String("log-viewer.rc"));

    /* Setup AccountManager */
    m_accountManager = Tp::AccountManager::create(accountFactory, connectionFactory, channelFactory, contactFactory);
    connect(m_accountManager->becomeReady(), SIGNAL(finished(Tp::PendingOperation*)), SLOT(onAccountManagerReady()));

    m_contactsModel = new KTp::ContactsModel(this);
    m_entityModel = new EntityModel(this);

    m_mergeModel = new PersonEntityMergeModel(m_contactsModel, m_entityModel, this);
    connect(m_mergeModel, SIGNAL(modelInitialized()),
            this, SLOT(slotMergeModelInitialized()));

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
    ui->entityFilter->lineEdit()->setPlaceholderText(i18nc("Placeholder text in line edit for filtering contacts", "Filter contacts..."));

    m_datesModel = new DatesModel(this);
    ui->datesView->setModel(m_datesModel);
    ui->datesView->setItemDelegate(new DatesViewDelegate(ui->datesView));
    ui->datesView->setItemsExpandable(true);
    ui->datesView->setRootIsDecorated(false);
    ui->datesView->setExpandsOnDoubleClick(false);
    ui->datesView->setIndentation(0);
    connect(m_datesModel, SIGNAL(datesReceived()),
            this, SLOT(onDatesReceived()));

    connect(ui->entityList, SIGNAL(clicked(QModelIndex)), SLOT(onEntityListClicked(QModelIndex)));
    connect(ui->datesView, SIGNAL(clicked(QModelIndex)), SLOT(slotDateClicked(QModelIndex)));
    connect(ui->datesView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), SLOT(slotUpdateMainWindow()));
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

    KAction *configure = new KAction(i18n("&Configure LogViewer"), this);
    configure->setIcon(QIcon::fromTheme(QStringLiteral("configure")));
    connect(configure, SIGNAL(triggered(bool)), SLOT(slotConfigure()));

    KAction *clearAccHistory = new KAction(i18n("Clear &account history"), this);
    clearAccHistory->setIcon(QIcon::fromTheme(QStringLiteral("edit-clear-history")));
    connect(clearAccHistory, SIGNAL(triggered(bool)), SLOT(slotClearAccountHistory()));

    KAction *clearContactHistory = new KAction(i18n("Clear &contact history"), this);
    clearContactHistory->setIcon(QIcon::fromTheme(QStringLiteral("edit-clear-history")));
    clearContactHistory->setEnabled(false);
    connect(clearContactHistory, SIGNAL(triggered(bool)), SLOT(slotClearContactHistory()));

    KAction *importKopeteLogs = new KAction(i18n("&Import Kopete Logs"), this);
    importKopeteLogs->setIcon(QIcon::fromTheme(QStringLiteral("document-import")));
    connect(importKopeteLogs, SIGNAL(triggered(bool)), SLOT(slotImportKopeteLogs()));

    KAction *prevConversation = new KAction(i18n("&Previous Conversation"), this);
    prevConversation->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
    prevConversation->setIcon(QIcon::fromTheme(QStringLiteral("go-previous")));
    prevConversation->setEnabled(false);
    connect(prevConversation, SIGNAL(triggered(bool)), SLOT(slotJumpToPrevConversation()));

    KAction *nextConversation = new KAction(i18n("&Next Conversation"), this);
    nextConversation->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_N));
    nextConversation->setIcon(QIcon::fromTheme(QStringLiteral("go-next")));
    nextConversation->setEnabled(false);
    connect(nextConversation, SIGNAL(triggered(bool)), SLOT(slotJumpToNextConversation()));

    actionCollection()->addAction(QLatin1String("clear-account-logs"), clearAccHistory);
    actionCollection()->addAction(QLatin1String("clear-contact-logs"), clearContactHistory);
    actionCollection()->addAction(QLatin1String("import-kopete-logs"), importKopeteLogs);
    actionCollection()->addAction(QLatin1String("jump-prev-conversation"), prevConversation);
    actionCollection()->addAction(QLatin1String("jump-next-conversation"), nextConversation);
    actionCollection()->addAction(QLatin1String("configure"), configure);

    /* Build the popup menu for entity list */
    m_entityListContextMenu = new KMenu(ui->entityList);
    m_entityListContextMenu->addAction(clearContactHistory);
    m_entityListContextMenu->addAction(clearAccHistory);
}

void LogViewer::onAccountManagerReady()
{
    KTp::LogManager *logManger = KTp::LogManager::instance();
    logManger->setAccountManager(m_accountManager);

    m_contactsModel->setAccountManager(m_accountManager);
    m_entityModel->setAccountManager(m_accountManager);

    /* Try to run log import */
    slotImportKopeteLogs(false);
}

void LogViewer::slotMergeModelInitialized()
{
    // Make sure we cal onEntityListClicked now - this ensures that if EntityView
    // has selected a person or contact (because user has passed it via arguments
    // on command line) it will correctly list all dates for all subcontacts
    if (!ui->entityList->selectionModel()->selectedIndexes().isEmpty()) {
        onEntityListClicked(ui->entityList->selectionModel()->selectedIndexes().first());
    }
}

void LogViewer::onEntityListClicked(const QModelIndex &index)
{
    const PersonEntityMergeModel::ItemType itemType =
        static_cast<PersonEntityMergeModel::ItemType>(index.data(PersonEntityMergeModel::ItemTypeRole).toUInt());

    if (itemType == PersonEntityMergeModel::Group) {
        ui->entityList->setExpanded(index, !ui->entityList->isExpanded(index));
        // Enable clear-account-logs only when grouping by accounts (i.e. in non-kpeople mode)
        const bool enabled = !index.data(PersonEntityMergeModel::AccountRole).value<Tp::AccountPtr>().isNull();
        actionCollection()->action(QLatin1String("clear-account-logs"))->setEnabled(enabled);
        actionCollection()->action(QLatin1String("clear-contact-logs"))->setEnabled(false);
    } else if (itemType == PersonEntityMergeModel::Persona) {
        if (m_expandedPersona.isValid()) {
            ui->entityList->collapse(m_expandedPersona);
        }
        ui->entityList->expand(index);
        m_expandedPersona = index;
        ui->messageView->clear();
        // FIXME: Show something fancy when selecting a persona
        m_datesModel->clear();
        for (int i = 0; i < index.model()->rowCount(index); ++i) {
            const QModelIndex child = index.child(i, 0);
            const KTp::LogEntity entity = child.data(PersonEntityMergeModel::EntityRole).value<KTp::LogEntity>();
            const Tp::AccountPtr account = child.data(PersonEntityMergeModel::AccountRole).value<Tp::AccountPtr>();
            Q_ASSERT(entity.isValid());
            Q_ASSERT(!account.isNull());
            m_datesModel->addEntity(account, entity);
        }
        actionCollection()->action(QLatin1String("clear-contact-logs"))->setEnabled(true);
        actionCollection()->action(QLatin1String("clear-account-logs"))->setEnabled(false);
        return;
    } else if (itemType == PersonEntityMergeModel::Entity) {
        const KTp::LogEntity entity = index.data(PersonEntityMergeModel::EntityRole).value<KTp::LogEntity>();
        const Tp::AccountPtr account = index.data(PersonEntityMergeModel::AccountRole).value<Tp::AccountPtr>();
        Q_ASSERT(entity.isValid());
        Q_ASSERT(!account.isNull());
        m_datesModel->setEntity(account, entity);
        actionCollection()->action(QLatin1String("clear-contact-logs"))->setEnabled(true);
        actionCollection()->action(QLatin1String("clear-account-logs"))->setEnabled(true);
    }
}

void LogViewer::onDatesReceived()
{
    const QModelIndex groupIndex = m_datesModel->index(0, 0, QModelIndex());
    if (groupIndex.isValid()) {
         // expand group
        slotDateClicked(groupIndex);

        // call slotUpdateMainWindow
        const QModelIndex dateIndex = m_datesModel->index(0, 0, groupIndex);
        ui->datesView->selectionModel()->setCurrentIndex(dateIndex, QItemSelectionModel::Select);
    }
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

    KTp::LogEntity entity = currentDateIndex.data(DatesModel::EntityRole).value<KTp::LogEntity>();
    Tp::AccountPtr account = currentDateIndex.data(DatesModel::AccountRole).value<Tp::AccountPtr>();
    KTp::ContactPtr contact = currentIndex.data(PersonEntityMergeModel::ContactRole).value<KTp::ContactPtr>();
    if (!entity.isValid() || account.isNull()) {
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
        // slotUpdateMainWindow() is called through currentChanged() signal
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
    m_datesModel->setSearchHits(search->searchHits());

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
    QModelIndex index = ui->entityList->indexAt(coords);
    if (!index.isValid()) {
        return;
    }

    PersonEntityMergeModel::ItemType type =
        static_cast<PersonEntityMergeModel::ItemType>(index.data(PersonEntityMergeModel::ItemTypeRole).toInt());
    if (type == PersonEntityMergeModel::Group) {
        const bool enabled = !index.data(PersonEntityMergeModel::AccountRole).value<Tp::AccountPtr>().isNull();
        actionCollection()->action(QLatin1String("clear-account-logs"))->setEnabled(enabled);
    } else {
        const bool enabled = ((type != PersonEntityMergeModel::Persona) &&
                              (static_cast<PersonEntityMergeModel::ItemType>(
                                    index.parent().data(PersonEntityMergeModel::ItemTypeRole).toInt()) != PersonEntityMergeModel::Persona));
        actionCollection()->action(QLatin1String("clear-account-logs"))->setEnabled(enabled);
        actionCollection()->action(QLatin1String("clear-contact-logs"))->setEnabled(true);
    }

    m_entityListContextMenu->setProperty("index", QVariant::fromValue(index));
    m_entityListContextMenu->popup(ui->entityList->mapToGlobal(coords));
}

void LogViewer::slotClearAccountHistory()
{
    QModelIndex index = m_filterModel->mapToSource(ui->entityList->currentIndex());

    /* Usually a contact node is selected, so traverse up to it's parent
     * account node */
    if (index.parent().isValid()) {
        index = index.parent();
    }

    if (!index.isValid()) {
        return;
    }

    const Tp::AccountPtr account = index.data(PersonEntityMergeModel::AccountRole).value<Tp::AccountPtr>();
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
    m_mergeModel->removeRow(index.row(), parent);

    m_entityListContextMenu->setProperty("index", QVariant());
}

void LogViewer::slotClearContactHistory()
{
    const QModelIndex index = m_filterModel->mapToSource(ui->entityList->currentIndex());
    if (!index.isValid()) {
        return;
    }

    if (static_cast<PersonEntityMergeModel::ItemType>(index.data(PersonEntityMergeModel::ItemTypeRole).toInt()) == PersonEntityMergeModel::Persona) {
        QString name = index.data(Qt::DisplayRole).toString();
        if (KMessageBox::warningYesNo(
                this, i18nc("%1 is contactdisplay name", "Are you sure you want to remove history of all conversations with %1?", name),
                i18n("Clear contact history"), KStandardGuiItem::del(), KStandardGuiItem::cancel(),
                QString(), KMessageBox::Dangerous) == KMessageBox::No) {
            return;
        }

        for (int i = 0; i < m_entityModel->rowCount(index); i++) {
            const QModelIndex child = m_entityModel->index(i, 0, index);
            const Tp::AccountPtr account = child.data(PersonEntityMergeModel::AccountRole).value<Tp::AccountPtr>();
            const KTp::LogEntity entity = child.data(PersonEntityMergeModel::EntityRole).value<KTp::LogEntity>();

            KTp::LogManager::instance()->clearContactLogs(account, entity);
        }
    } else {
        const Tp::AccountPtr account = index.data(PersonEntityMergeModel::AccountRole).value<Tp::AccountPtr>();
        const KTp::LogEntity entity = index.data(PersonEntityMergeModel::EntityRole).value<KTp::LogEntity>();
        if (account.isNull() || !entity.isValid()) {
            return;
        }

        QString name = index.data(Qt::DisplayRole).toString();
        if (KMessageBox::warningYesNo(
                this, i18nc("%1 is contact display name, %2 is contact UID", "Are you sure you want to remove history of all conversations with %1 (%2)?", name, entity.id()),
                i18n("Clear contact history"), KStandardGuiItem::del(), KStandardGuiItem::cancel(),
                QString(), KMessageBox::Dangerous) == KMessageBox::No) {
            return;
        }

        KTp::LogManager::instance()->clearContactLogs(account, entity);
    }

    m_mergeModel->removeRow(index.row(), index.parent());

    m_entityListContextMenu->setProperty("index", QVariant());
}

void LogViewer::slotImportKopeteLogs(bool force)
{
    KSharedConfigPtr config = KSharedConfig::openConfig(QLatin1String("ktelepathyrc"));
    KConfigGroup logsConfig = config->group("logs");

    bool importDone = logsConfig.readEntry(QLatin1String("InitialKopeteImportDone"), QVariant(false)).toBool();
    if (!force && importDone) {
        qCDebug(KTP_LOGVIEWER) << "Skipping initial Kopete logs import, already done.";
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

    qCDebug(KTP_LOGVIEWER) << "Initial Kopete logs import: found" << matchingAccounts.count() << "accounts to import";

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
    }
}

void LogViewer::slotConfigure()
{
    KSettings::Dialog *dialog = new KSettings::Dialog(this);

    KPageWidgetItem *configPage = dialog->addModule(QLatin1String("kcm_ktp_chat_appearance"));
    KCModuleProxy *proxy = qobject_cast<KCModuleProxy*>(configPage->widget());
    Q_ASSERT(proxy);
    connect(proxy->realModule(), SIGNAL(reloadTheme()),
            ui->messageView, SLOT(reloadTheme()));

    configPage = dialog->addModule(QLatin1String("kcm_ktp_logviewer_behavior"));
    proxy = qobject_cast<KCModuleProxy*>(configPage->widget());
    Q_ASSERT(proxy);
    connect(proxy->realModule(), SIGNAL(reloadMessages()),
            ui->messageView, SLOT(reloadTheme()));

    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void LogViewer::slotNoLogsForContact()
{
    ui->messageView->showInfoMessage(i18n("There are no logs for this contact"));
}
