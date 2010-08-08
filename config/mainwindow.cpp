#include "mainwindow.h"
#include "ui_chatwindowconfig.h"
#include "chatwindowstylemanager.h"

#include "telepathychatinfo.h"
#include "telepathychatmessageinfo.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChatWindowConfig)
{
    ui->setupUi(this);

    ChatWindowStyleManager* manager = ChatWindowStyleManager::self();
    manager->loadStyles();
    connect(manager,SIGNAL(loadStylesFinished()),SLOT(debugStyleList()));

    //FIXME move all the demo chat code into a different file, as it will be quite long and in the way.

    //set up a pretend config chat.
    TelepathyChatInfo info;

    info.setChatName("A demo chat");
    info.setTimeOpened(QDateTime::currentDateTime());
    info.setDestinationName("BobMarley@yahoo.com");
    info.setSourceName("Jabber");
    info.setDestinationDisplayName("Bob Marley");

    ui->chatView->initialise(info);

    connect(ui->chatView,SIGNAL(loadFinished(bool)),SLOT(sendDemoMessages()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::debugStyleList()
{
    qDebug() << ChatWindowStyleManager::self()->getAvailableStyles();
}

void MainWindow::changeEvent(QEvent *e)
{/*
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }*/
}

void MainWindow::sendDemoMessages()
{
    //add a fake message
    //in my head Bob Marley is quite a chatty friendly guy...

    TelepathyChatMessageInfo message(TelepathyChatMessageInfo::RemoteToLocal);
    message.setMessage("Hello, how are things?");
    message.setSenderDisplayName("BobMarley@yahoo.com");
    message.setSenderScreenName("Bob Marley");
    message.setService("Jabber");
    message.setTime(QDateTime::currentDateTime());
    ui->chatView->addMessage(message);
}
