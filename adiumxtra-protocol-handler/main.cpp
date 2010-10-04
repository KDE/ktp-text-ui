#include "adiumxtraprotocolhandler.h"

#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KDebug>

int main(int argc, char *argv[])
{
    kDebug();

    KAboutData aboutData("adiumxtra-protocol-handler",
                         0,
                         ki18n("AdiumXtra Protocol Handler"),
                         "0.1");
    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineOptions options;
    options.add("!+install-chatstyles", ki18n("Install Adium packages"));
    KCmdLineArgs::addCmdLineOptions( options );


    AdiumxtraProtocolHandler app;

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    for(int i = 0; i < args->count(); i++)
    {
        kDebug() << "install: " << args->arg(i);
        app.install(args->arg(i));

    }
    args->clear();

    return app.exec();
}