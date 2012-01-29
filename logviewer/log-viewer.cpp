#include "log-viewer.h"
#include "ui_log-viewer.h"

#include <TelepathyQt/AccountManager>
#include <TelepathyQt/PendingReady>


#include <TelepathyLoggerQt4/Init>
#include <TelepathyLoggerQt4/Entity>
#include <TelepathyLoggerQt4/LogManager>

#include <glib-object.h>
#include <QGlib/Init>

#include "entity-model.h"

LogViewer::LogViewer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LogViewer)
{
    ui->setupUi(this);
    Tp::registerTypes();
    g_type_init();
    QGlib::init(); //are these 4 really needed?
    Tpl::init();

    m_accountManager = Tp::AccountManager::create();

    m_entityModel = new EntityModel(this);

    ui->entityList->setModel(m_entityModel);

    //TODO parse command line args and update all views as appropriate

    connect(m_accountManager->becomeReady(), SIGNAL(finished(Tp::PendingOperation*)), SLOT(onAccountManagerReady()));
    connect(ui->entityList, SIGNAL(activated(QModelIndex)), SLOT(onEntitySelected(QModelIndex)));
    connect(ui->datePicker, SIGNAL(dateChanged(QDate)), SLOT(onDateSelected()));
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

void LogViewer::onEntitySelected(const QModelIndex &index)
{
    //TODO, update calendar needs to get pendingDates

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

    Tpl::EntityPtr entity = currentIndex.data(EntityModel::EntityRole).value<Tpl::EntityPtr>();
    Tp::AccountPtr account = currentIndex.data(EntityModel::AccountRole).value<Tp::AccountPtr>();
    ui->messageView->loadLog(account, entity, ui->datePicker->date());
}
