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

#include "chat-style-installer.h"

#include "chat-window-style-manager.h"
#include "chat-style-plist-file-reader.h"

#include <QTemporaryFile>
#include <QTimer>
#include <QDebug>

#include <KLocalizedString>
#include <KArchiveFile>
#include <KNotification>

// FIXME: Part of a hack to let adiumxtra-protocol-handler use the main ktelepathy.notifyrc because
// the string freeze does not permit adding a new notifyrc only for adiumxtra-protocol-handler.
// Remove this after 0.7 is released.
static QString ktelepathyComponentName() {
    return QStringLiteral("ktelepathy");
}

ChatStyleInstaller::ChatStyleInstaller(KArchive *archive, QTemporaryFile *tmpFile)
{
    m_archive = archive;
    m_tmpFile = tmpFile;
}

ChatStyleInstaller::~ChatStyleInstaller()
{
}

BundleInstaller::BundleStatus ChatStyleInstaller::validate()
{
    KArchiveEntry *currentEntry = 0L;
    KArchiveDirectory* currentDir = 0L;
    const KArchiveDirectory* rootDir = m_archive->directory();
    int validResult = 0;
    const QStringList entries = rootDir->entries();
    QStringList::ConstIterator entriesIt;

    for (entriesIt = entries.begin(); entriesIt != entries.end(); ++entriesIt) {
        currentEntry = const_cast<KArchiveEntry*>(rootDir->entry(*entriesIt));
        if (currentEntry->isDirectory()) {
            currentDir = dynamic_cast<KArchiveDirectory*>(currentEntry);
            if (currentDir) {
                if (currentDir->entry(QLatin1String("Contents")) &&
                    currentDir->entry(QLatin1String("Contents"))->isDirectory()) {
                    validResult += 1;
                }
                if (currentDir->entry(QLatin1String("Contents/Info.plist")) &&
                    currentDir->entry(QLatin1String("Contents/Info.plist"))->isFile()) {
                    KArchiveFile const *info = dynamic_cast<KArchiveFile const *>(
                        currentDir->entry(QLatin1String("Contents/Info.plist"))
                    );
                    QByteArray data = info->data();
                    ChatStylePlistFileReader reader(data);
                    if(m_bundleName.isEmpty()) {
                        m_bundleName = reader.CFBundleName();
                    }
                    validResult += 1;
                }
            }
        }
    }

    if(validResult >= 2) {
        return BundleValid;
    } else {
        qWarning() << "Bundle is not valid";
        return BundleNotValid;
    }
}

QString ChatStyleInstaller::bundleName() const
{
    return m_bundleName;
}

BundleInstaller::BundleStatus ChatStyleInstaller::install()
{
    BundleInstaller::BundleStatus status = static_cast<BundleInstaller::BundleStatus>(
        ChatWindowStyleManager::self()->installStyle(m_archive->fileName())
    );
    delete m_tmpFile;

    m_status = status;

    Q_EMIT finished(status);

    return status;
}

void ChatStyleInstaller::showRequest()
{
    KNotification *notification = new KNotification(QLatin1String("chatstyleRequest"), NULL, KNotification::Persistent);
    notification->setText( i18n("Install Chatstyle %1", this->bundleName()) );
    notification->setActions( QStringList() << i18n("Install") << i18n("Cancel") );

    QObject::connect(notification, SIGNAL(action1Activated()), this, SLOT(install()));
    QObject::connect(notification, SIGNAL(action1Activated()), notification, SLOT(close()));

    QObject::connect(notification, SIGNAL(ignored()), this, SLOT(ignoreRequest()));
    QObject::connect(notification, SIGNAL(ignored()), notification, SLOT(close()));

    QObject::connect(notification, SIGNAL(action2Activated()), this, SLOT(ignoreRequest()));
    QObject::connect(notification, SIGNAL(action2Activated()), notification, SLOT(close()));

    notification->setComponentName(ktelepathyComponentName());
    notification->sendEvent();
}

void ChatStyleInstaller::showResult()
{
    KNotification *notification;
    if(m_status == BundleInstaller::BundleInstallOk) {
        qDebug() << "Installed Chatstyle" << this->bundleName() << "successfully";
        notification = new KNotification(QLatin1String("chatstyleSuccess"));
        notification->setText( i18n("Installed Chatstyle %1 successfully.", this->bundleName()) );
    } else {
        qWarning() << "Installation of Chatstyle" << this->bundleName() << "failed";
        notification = new KNotification(QLatin1String("chatstyleFailure"));
        notification->setText( i18n("Installation of Chatstyle %1 failed.", this->bundleName()) );
    }
    notification->setComponentName(ktelepathyComponentName());
    notification->sendEvent();

    Q_EMIT showedResult();
}

void ChatStyleInstaller::ignoreRequest()
{
    Q_EMIT ignoredRequest();
}
