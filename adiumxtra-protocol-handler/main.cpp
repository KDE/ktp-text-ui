#include "chatstyleinstaller.h"
#include "emoticonsetinstaller.h"
#include <chatwindowstylemanager.h>


#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KDebug>
#include <KZip>
#include <KTar>
#include <KEmoticons>
#include <KTemporaryFile>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <KNotification>
#include <KIcon>

bool install(const QString& path);

KApplication *app;

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


    app = new KApplication();

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    for(int i = 0; i < args->count(); i++)
    {
        kDebug() << "install: " << args->arg(i);
        install(args->arg(i));

    }
    args->clear();

    return app->exec();
}


bool install(const QString &path)
{
    kDebug();

    KUrl url(path);
    if(url.protocol() == "adiumxtra") {
        url.setProtocol("http");
    }

    KTemporaryFile *tmpFile = new KTemporaryFile();
    if (tmpFile->open()) {
        KIO::Job* getJob = KIO::file_copy(url.prettyUrl(), KUrl(tmpFile->fileName()), -1, KIO::Overwrite);
        if (!KIO::NetAccess::synchronousRun(getJob, 0)) {
            kDebug() << "download failed";
            return false;
        }
    }

    KArchive *archive = 0L;

    QString currentBundleMimeType = KMimeType::findByPath(tmpFile->fileName(), 0, false)->name();
    if (currentBundleMimeType == "application/zip") {
        archive = new KZip(tmpFile->fileName());
    } else if (currentBundleMimeType == "application/x-compressed-tar" || currentBundleMimeType == "application/x-bzip-compressed-tar" || currentBundleMimeType == "application/x-gzip" || currentBundleMimeType == "application/x-bzip") {
        archive = new KTar(tmpFile->fileName());
    } else {
        KNotification *notification = new KNotification("packagenotrecognized", NULL, KNotification::Persistent);
        notification->setText( i18n("Package type not recognized or not supported") );
        notification->setActions( QStringList() << i18n("Ok") );
        notification->sendEvent();
        kDebug() << "unsupported file type" << currentBundleMimeType;
        kDebug() << tmpFile->fileName();
        return false;
    }

    if (!archive->open(QIODevice::ReadOnly)) {
        delete archive;
         kDebug() << "cannot open theme file";
        return false;
    }

    BundleInstaller *installer = new ChatStyleInstaller(archive, tmpFile);
    if(installer->validate()) {
        KNotification *notification = new KNotification("installchatstyle", NULL, KNotification::Persistent);
        notification->setText( i18n("Install Chatstyle %1", installer->bundleName()) );
        notification->setActions( QStringList() << i18n("Install") << i18n("Cancel") );

        QObject::connect(notification, SIGNAL(action1Activated()), installer, SLOT(install()));
        QObject::connect(notification, SIGNAL(ignored()), app, SLOT(quit()));
        QObject::connect(notification, SIGNAL(action2Activated()), app, SLOT(quit()));

        QObject::connect(installer, SIGNAL(finished()), app, SLOT(quit()));


        notification->sendEvent();
        kDebug() << "sent messagestyle request";
    } else {
        delete installer;
        installer = new EmoticonSetInstaller(archive, tmpFile);
        if(installer->validate()) {
            KNotification *notification = new KNotification("installemoticonset", NULL, KNotification::Persistent);
            notification->setText( i18n("Install Emoticonset %1", installer->bundleName()) );
            notification->setActions( QStringList() << i18n("Install") << i18n("Cancel") );

            QObject::connect(notification, SIGNAL(action1Activated()), installer, SLOT(install()));
            QObject::connect(notification, SIGNAL(ignored()), app, SLOT(quit()));
            QObject::connect(notification, SIGNAL(action2Activated()), app, SLOT(quit()));

            QObject::connect(installer, SIGNAL(finished()), app, SLOT(quit()));


            notification->sendEvent();
            kDebug() << "sent emoticonset request";
        } else {
            KNotification *notification = new KNotification("packagenotrecognized", NULL, KNotification::Persistent);
            notification->setText( i18n("Package type not recognized or not supported") );
            notification->setActions( QStringList() << i18n("Ok") );
            notification->sendEvent();
            kDebug() << "sent error";
        }
    }
}
