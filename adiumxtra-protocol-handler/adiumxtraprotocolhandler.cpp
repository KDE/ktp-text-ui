/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "adiumxtraprotocolhandler.h"
#include "chatstyleinstaller.h"
#include "emoticonsetinstaller.h"
#include <chatwindowstylemanager.h>

#include <KDebug>
#include <KZip>
#include <KTar>
#include <KEmoticons>
#include <KTemporaryFile>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <KNotification>
#include <KIcon>

AdiumxtraProtocolHandler::AdiumxtraProtocolHandler()
{
    kDebug();
}

AdiumxtraProtocolHandler::~AdiumxtraProtocolHandler()
{
    kDebug();
}

BundleInstaller::BundleStatus AdiumxtraProtocolHandler::install(const QString &path)
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
            return BundleInstaller::BundleCannotOpen;
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
        QObject::connect(notification, SIGNAL(action1Activated()), this, SLOT(install()));
        QObject::connect(notification, SIGNAL(action1Activated()), notification, SLOT(close()));

        QObject::connect(notification, SIGNAL(ignored()), this, SLOT(ignoreRequest()));
        QObject::connect(notification, SIGNAL(ignored()), notification, SLOT(close()));
        notification->sendEvent();
        kDebug() << "unsupported file type" << currentBundleMimeType;
        kDebug() << tmpFile->fileName();
        return BundleInstaller::BundleNotValid;
    }

    if (!archive->open(QIODevice::ReadOnly)) {
        delete archive;
         kDebug() << "cannot open theme file";
        return BundleInstaller::BundleCannotOpen;
    }

    BundleInstaller *installer = new ChatStyleInstaller(archive, tmpFile);
    if(installer->validate() == BundleInstaller::BundleValid) {
        installer->showRequest();

        QObject::connect(installer, SIGNAL(finished(BundleInstaller::BundleStatus)), installer, SLOT(showResult()));
        QObject::connect(installer, SIGNAL(showedResult()), this, SLOT(quit()));
        QObject::connect(installer, SIGNAL(ignoredRequest()), this, SLOT(quit()));

        kDebug() << "sent messagestyle request";
    } else {
        delete installer;
        installer = new EmoticonSetInstaller(archive, tmpFile);
        if(installer->validate() == BundleInstaller::BundleValid) {
            installer->showRequest();

            QObject::connect(installer, SIGNAL(finished(BundleInstaller::BundleStatus)), installer, SLOT(showResult()));
            QObject::connect(installer, SIGNAL(showedResult()), this, SLOT(quit()));
            QObject::connect(installer, SIGNAL(ignoredRequest()), this, SLOT(quit()));

            kDebug() << "sent emoticonset request";
        } else {
            KNotification *notification = new KNotification("packagenotrecognized", NULL, KNotification::Persistent);
            notification->setText( i18n("Package type not recognized or not supported") );
            QObject::connect(notification, SIGNAL(action1Activated()), this, SLOT(install()));
            QObject::connect(notification, SIGNAL(action1Activated()), notification, SLOT(close()));

            QObject::connect(notification, SIGNAL(ignored()), this, SLOT(ignoreRequest()));
            QObject::connect(notification, SIGNAL(ignored()), notification, SLOT(close()));
            notification->setActions( QStringList() << i18n("Ok") );
            notification->sendEvent();
            kDebug() << "sent error";

            return BundleInstaller::BundleUnknownError;
        }
    }

    return BundleInstaller::BundleValid;
}
