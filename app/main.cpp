#include <QtGui/QApplication>
#include "mainwindow.h"

#include <QDebug>

#include <TelepathyQt4/Channel>
#include <TelepathyQt4/ClientRegistrar>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    registerTypes();

    ClientRegistrarPtr registrar = ClientRegistrar::create();
    MainWindow* mainWindow = new MainWindow();

    AbstractClientPtr handler = AbstractClientPtr::dynamicCast(SharedPtr<MainWindow>(mainWindow));
    registrar->registerClient(handler, "KDEChatHandler");

    mainWindow->show();

    return a.exec();
    delete mainWindow;
}
