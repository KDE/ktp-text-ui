#include "mainwindow.h"

#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>


int main(int argc, char *argv[])
{
    KAboutData aboutData("telepathy-chat-window-config",
                         0,
                         ki18n("Telepathy Chat Window Config"),
                         "0.1");

    KCmdLineArgs::init( argc, argv, &aboutData );

    KApplication app;

    MainWindow w;
    w.show();

    return app.exec();
}
