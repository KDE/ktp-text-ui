#include "mainwindow.h"
#include "ui_chatwindowconfig.h"

#include "telepathychatinfo.h"
#include "telepathychatmessageinfo.h"

MainWindow::MainWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChatWindowConfig)
{
    ui->setupUi(this);

    //FIXME move all the demo chat code into a different file, as it will be quite long and in the way.

    //set up a pretend config chat.
    TelepathyChatInfo info;

    info.setChatName("A demo chat");
    info.setTimeOpened(QDateTime::currentDateTime());
    info.setDestinationName("BobMarley@yahoo.com");
    info.setSourceName("Jabber");
    info.setDestinationDisplayName("Bob Marley");

    ui->chatView->initialise(info);


    //add a fake message
    //in my head Bob Marley is quite a chatty friendly guy...

    TelepathyChatMessageInfo message;
    message.setMessage("Hello, how are things?");
    message.setMessageDirection("rtl");
    message.setSenderDisplayName("BobMarley@yahoo.com");
    message.setSenderScreenName("Bob Marley");
    message.setService("Jabber");
    message.setTime(QDateTime::currentDateTime());


    ui->chatView->addMessage(message);

}

MainWindow::~MainWindow()
{
    delete ui;
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
