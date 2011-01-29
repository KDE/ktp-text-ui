/*
    KDE Telepathy AdiumxtraProtocolHandler - Install Adiumxtra packages through adiumxtra://-pseudo protocol
    Copyright (C) 2010 Dominik Schmidt <domme@rautelinux.org>

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

#include "chatstyleinstaller.h"

#include "chatwindowstylemanager.h"
#include "chatstyleplistfilereader.h"

#include <KDebug>
#include <KTemporaryFile>
#include <KArchiveFile>
#include <KLocale>
#include <KNotification>
#include <KApplication>


ChatStyleInstaller::ChatStyleInstaller(KArchive *archive, KTemporaryFile *tmpFile)
{
    kDebug();

    m_archive = archive;
    m_tmpFile = tmpFile;
}

ChatStyleInstaller::~ChatStyleInstaller()
{
    kDebug();
}

BundleInstaller::BundleStatus ChatStyleInstaller::validate()
{
    kDebug();

    KArchiveEntry *currentEntry = 0L;
    KArchiveDirectory* currentDir = 0L;
    const KArchiveDirectory* rootDir = m_archive->directory();
    int validResult = 0;
    const QStringList entries = rootDir->entries();
    QStringList::ConstIterator entriesIt;

    for (entriesIt = entries.begin(); entriesIt != entries.end(); ++entriesIt) {
        currentEntry = const_cast<KArchiveEntry*>(rootDir->entry(*entriesIt));
        kDebug() << "Current entry name: " << currentEntry->name();
        if (currentEntry->isDirectory()) {
            currentDir = dynamic_cast<KArchiveDirectory*>(currentEntry);
            if (currentDir) {
                if (currentDir->entry(QLatin1String("Contents"))) {
                    kDebug() << "Contents found";
                    validResult += 1;
                }
                if (currentDir->entry(QLatin1String("Contents/Info.plist"))) {
                    kDebug() << "Contents/Info.plist found";
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
        return BundleNotValid;
    }
}

QString ChatStyleInstaller::bundleName() const
{
    kDebug();

    return m_bundleName;
}

BundleInstaller::BundleStatus ChatStyleInstaller::install()
{
    kDebug();

    BundleInstaller::BundleStatus status = static_cast<BundleInstaller::BundleStatus>(
        ChatWindowStyleManager::self()->installStyle(m_archive->fileName())
    );
    kDebug()<< "status " << status;
    delete m_tmpFile;

    m_status = status;

    emit(finished(status));

    return status;
}

void ChatStyleInstaller::showRequest()
{
    kDebug();

    KNotification *notification = new KNotification("chatstyleRequest", NULL, KNotification::Persistent);
    notification->setText( i18n("Install Chatstyle %1", this->bundleName()) );
    notification->setActions( QStringList() << i18n("Install") << i18n("Cancel") );

    QObject::connect(notification, SIGNAL(action1Activated()), this, SLOT(install()));
    QObject::connect(notification, SIGNAL(action1Activated()), notification, SLOT(close()));

    QObject::connect(notification, SIGNAL(ignored()), this, SLOT(ignoreRequest()));
    QObject::connect(notification, SIGNAL(ignored()), notification, SLOT(close()));

    QObject::connect(notification, SIGNAL(action2Activated()), this, SLOT(ignoreRequest()));
    QObject::connect(notification, SIGNAL(action2Activated()), notification, SLOT(close()));

    notification->sendEvent();
}

void ChatStyleInstaller::showResult()
{
    kDebug();

    KNotification *notification;
    if(m_status == BundleInstaller::BundleInstallOk) {
        notification = new KNotification("chatstyleSuccess", NULL, KNotification::Persistent);
        notification->setText( i18n("Installed Chatstyle %1 successfully.", this->bundleName()) );
    } else {
        notification = new KNotification("chatstyleFailure", NULL, KNotification::Persistent);
        notification->setText( i18n("Installation of Chatstyle %1 failed.", this->bundleName()) );
    }

    notification->setActions( QStringList() << i18n("OK") );
    QObject::connect(notification, SIGNAL(action1Activated()), notification, SLOT(close()));
    QObject::connect(notification, SIGNAL(ignored()), notification, SLOT(close()));

    notification->sendEvent();

    emit(showedResult());
}

void ChatStyleInstaller::ignoreRequest()
{
    kDebug();

    emit(ignoredRequest());
}
