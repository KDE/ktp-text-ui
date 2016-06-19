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

#include <KZip>
#include <KTar>
#include <KEmoticons>
#include <KIO/Job>
#include <KNotification>
#include <KLocalizedString>

#include <QTemporaryFile>
#include <QUrl>
#include <QDebug>
#include <QMimeDatabase>
#include <QMimeType>

// FIXME: Part of a hack to let adiumxtra-protocol-handler use the main ktelepathy.notifyrc because
// the string freeze does not permit adding a new notifyrc only for adiumxtra-protocol-handler.
// Remove this after 0.7 is released.
static QString ktelepathyComponentName() {
    return QStringLiteral("ktelepathy");
}

AdiumxtraProtocolHandler::AdiumxtraProtocolHandler()
    : QObject()
{
}

AdiumxtraProtocolHandler::~AdiumxtraProtocolHandler()
{
}

void AdiumxtraProtocolHandler::install()
{
    if (m_url.isEmpty()) {
        Q_EMIT finished();
        return; // BundleInstaller:: xxxxx
    }

    QUrl url = QUrl::fromUserInput(m_url);
    if(url.scheme() == QLatin1String("adiumxtra")) {
        url.setScheme(QStringLiteral("http"));
    }

    QTemporaryFile *tmpFile = new QTemporaryFile();
    if (tmpFile->open()) {
        KIO::Job* getJob = KIO::file_copy(url, QUrl::fromLocalFile(tmpFile->fileName()), -1,
                                          KIO::Overwrite | KIO::HideProgressInfo);
        if (getJob->exec()) {
            qWarning() << "Download failed";
            Q_EMIT finished();
            return; // BundleInstaller::BundleCannotOpen;
        }
        getJob->deleteLater();
    }

    KArchive *archive = 0L;

    QMimeDatabase db;

    QString currentBundleMimeType =  db.mimeTypeForFile(tmpFile->fileName()).name();
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
        qWarning() << "Unsupported file type" << currentBundleMimeType;
        Q_EMIT finished();
        return;// BundleInstaller::BundleNotValid;
    }

    if (!archive->open(QIODevice::ReadOnly)) {
        delete archive;
        qWarning() << "Cannot open theme file";
        Q_EMIT finished();
        return;// BundleInstaller::BundleCannotOpen;
    }

    ChatStyleInstaller *chatStyleInstaller = new ChatStyleInstaller(archive, tmpFile);
    if (chatStyleInstaller->validate() == BundleInstaller::BundleValid) {
        chatStyleInstaller->showRequest();

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

    Q_EMIT finished();
    return;// BundleInstaller::BundleUnknownError;
}

void AdiumxtraProtocolHandler::setUrl(const QString& url)
{
    m_url = url;
}
