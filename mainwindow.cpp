#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include "realclienthandler.h"
#include <TelepathyQt4/AbstractClient>
#include <QStatusBar>

#include "chatwindow.h"


MainWindow::MainWindow(RealClientHandler *clientHandler, QWidget *parent) :
        QWidget(parent),
        ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_clientHandler = clientHandler;

    connect(m_clientHandler, SIGNAL(newConnection(ChatConnection*)), SLOT(handleNewConnection(ChatConnection*)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type())
    {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::handleNewConnection(ChatConnection *connection)
{
    ChatWindow* newWindow = new ChatWindow(connection, this);
    ui->ktabwidget->addTab(newWindow, "test"); //FIXME get name from connection contact
}

