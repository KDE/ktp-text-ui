#include <QtGui/QApplication>
#include "mainwindow.h"

#include <QDebug>

#include <TelepathyQt4/Channel>
#include <TelepathyQt4/ClientRegistrar>
#include "realclienthandler.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    registerTypes();

    //I'd like to move this to mainwindow.cpp but this seemed to break it. I really don't understand why.

    ClientRegistrarPtr registrar = ClientRegistrar::create();
    RealClientHandler* clientHandler = new RealClientHandler();

    AbstractClientPtr handler = AbstractClientPtr::dynamicCast(
                                    SharedPtr<RealClientHandler>(clientHandler));
    registrar->registerClient(handler, "KDEChatHandler");

    MainWindow w(clientHandler);
    w.show();

    return a.exec();
    delete clientHandler;
}
