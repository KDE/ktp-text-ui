/*
    KDE Telepathy AdiumxtraProtocolHandler - Install Adiumxtra packages through adiumxtra://-pseudo protocol
    Copyright (C) 2010 Dominik Schmidt <domme@rautelinux.org>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "adiumxtra-protocol-handler.h"
#include "chat-style-installer.h"
#include "emoticon-set-installer.h"

#include <KDebug>
#include <KZip>
#include <KTar>
#include <KEmoticons>
#include <KTemporaryFile>
#include <KIO/Job>
#include <KIO/NetAccess>
#include <KNotification>
#include <KMimeType>
#include <KAboutData>
#include <KComponentData>
#include <KUrl>

// FIXME: Part of a hack to let adiumxtra-protocol-handler use the main ktelepathy.notifyrc because
// the string freeze does not permit adding a new notifyrc only for adiumxtra-protocol-handler.
// Remove this after 0.7 is released.
static QString ktelepathyComponentName() {
    return QStringLiteral("ktelepathy");
}

AdiumxtraProtocolHandler::AdiumxtraProtocolHandler()
    : QObject()
{
    kDebug();
}

AdiumxtraProtocolHandler::~AdiumxtraProtocolHandler()
{
    kDebug();
}

void AdiumxtraProtocolHandler::install()
{
    kDebug();

    if (m_url.isEmpty()) {
        Q_EMIT finished();
        return; // BundleInstaller:: xxxxx
    }

    KUrl url(m_url);
    if(url.protocol() == QLatin1String("adiumxtra")) {
        url.setProtocol(QLatin1String("http"));
    }

    KTemporaryFile *tmpFile = new KTemporaryFile();
    if (tmpFile->open()) {
        KIO::Job* getJob = KIO::file_copy(url.prettyUrl(), KUrl(tmpFile->fileName()), -1,
                                          KIO::Overwrite | KIO::HideProgressInfo);
        if (!KIO::NetAccess::synchronousRun(getJob, 0)) {
            kDebug() << "Download failed";
            Q_EMIT finished();
            return; // BundleInstaller::BundleCannotOpen;
        }
        getJob->deleteLater();
    }

    KArchive *archive = 0L;

    QString currentBundleMimeType = KMimeType::findByPath(tmpFile->fileName(), 0, false)->name();
    if (currentBundleMimeType == QLatin1String("application/zip")) {
        archive = new KZip(tmpFile->fileName());
    } else if (currentBundleMimeType == QLatin1String("application/x-compressed-tar") ||
               currentBundleMimeType == QLatin1String("application/x-bzip-compressed-tar") ||
               currentBundleMimeType == QLatin1String("application/x-gzip") ||
               currentBundleMimeType == QLatin1String("application/x-bzip")) {
        archive = new KTar(tmpFile->fileName());
    } else {
        KNotification *notification = new KNotification(QLatin1String("packagenotrecognized"), NULL, KNotification::Persistent);
        notification->setText( i18n("Package type not recognized or not supported") );
        notification->setActions( QStringList() << i18n("OK") );
        QObject::connect(notification, SIGNAL(action1Activated()), notification, SLOT(close()));
        QObject::connect(notification, SIGNAL(ignored()), notification, SLOT(close()));
        notification->setComponentName(ktelepathyComponentName());
        notification->sendEvent();
        kDebug() << "Unsupported file type" << currentBundleMimeType;
        kDebug() << tmpFile->fileName();
        Q_EMIT finished();
        return;// BundleInstaller::BundleNotValid;
    }

    if (!archive->open(QIODevice::ReadOnly)) {
        delete archive;
        kDebug() << "Cannot open theme file";
        Q_EMIT finished();
        return;// BundleInstaller::BundleCannotOpen;
    }

    ChatStyleInstaller *chatStyleInstaller = new ChatStyleInstaller(archive, tmpFile);
    if (chatStyleInstaller->validate() == BundleInstaller::BundleValid) {
        chatStyleInstaller->showRequest();
        kDebug() << "Sent messagestyle request";

        QObject::connect(chatStyleInstaller, SIGNAL(finished(BundleInstaller::BundleStatus)),
                         chatStyleInstaller, SLOT(showResult()));
        QObject::connect(chatStyleInstaller, SIGNAL(showedResult()), this, SIGNAL(finished()));
        QObject::connect(chatStyleInstaller, SIGNAL(showedResult()),
                         chatStyleInstaller, SLOT(deleteLater()));
        QObject::connect(chatStyleInstaller, SIGNAL(ignoredRequest()), this, SIGNAL(finished()));
        QObject::connect(chatStyleInstaller, SIGNAL(ignoredRequest()),
                         chatStyleInstaller, SLOT(deleteLater()));

        return;// BundleInstaller::BundleValid;
    }
    delete chatStyleInstaller;

    EmoticonSetInstaller *emoticonSetInstaller = new EmoticonSetInstaller(archive, tmpFile);
    if(emoticonSetInstaller->validate() == BundleInstaller::BundleValid) {
        emoticonSetInstaller->showRequest();
        kDebug() << "Sent emoticonset request";

        QObject::connect(emoticonSetInstaller, SIGNAL(finished(BundleInstaller::BundleStatus)),
                         emoticonSetInstaller, SLOT(showResult()));
        QObject::connect(emoticonSetInstaller, SIGNAL(showedResult()), this, SIGNAL(finished()));
        QObject::connect(emoticonSetInstaller, SIGNAL(showedResult()),
                         emoticonSetInstaller, SLOT(deleteLater()));
        QObject::connect(emoticonSetInstaller, SIGNAL(ignoredRequest()), this, SIGNAL(finished()));
        QObject::connect(emoticonSetInstaller, SIGNAL(ignoredRequest()),
                         emoticonSetInstaller, SLOT(deleteLater()));

        return;// BundleInstaller::BundleValid;
    }
    delete emoticonSetInstaller;

    KNotification *notification = new KNotification(QLatin1String("packagenotrecognized"),
                                                    NULL,
                                                    KNotification::Persistent);
    notification->setText( i18n("Package type not recognized or not supported") );
    QObject::connect(notification, SIGNAL(action1Activated()), notification, SLOT(close()));
    QObject::connect(notification, SIGNAL(ignored()), notification, SLOT(close()));
    notification->setActions( QStringList() << i18n("OK") );
    notification->setComponentName(ktelepathyComponentName());
    notification->sendEvent();
    kDebug() << "Sent error";

    Q_EMIT finished();
    return;// BundleInstaller::BundleUnknownError;
}

void AdiumxtraProtocolHandler::setUrl(const QString& url)
{
    m_url = url;
}
