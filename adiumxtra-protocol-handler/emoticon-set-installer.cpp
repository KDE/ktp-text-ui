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

#include "emoticon-set-installer.h"

#include "chat-style-plist-file-reader.h"

#include <QTemporaryFile>

#include <KArchiveFile>
#include <KEmoticons>
#include <KArchiveDirectory>
#include <KNotification>
#include <KLocalizedString>

// FIXME: Part of a hack to let adiumxtra-protocol-handler use the main ktelepathy.notifyrc because
// the string freeze does not permit adding a new notifyrc only for adiumxtra-protocol-handler.
// Remove this after 0.7 is released.
static QString ktelepathyComponentName() {
    return QStringLiteral("ktelepathy");
}

EmoticonSetInstaller::EmoticonSetInstaller(KArchive *archive, QTemporaryFile *tmpFile)
{
    m_archive = archive;
    m_tmpFile = tmpFile;
}

EmoticonSetInstaller::~EmoticonSetInstaller()
{
}

BundleInstaller::BundleStatus EmoticonSetInstaller::validate()
{
    KArchiveEntry *currentEntry = 0L;
    KArchiveDirectory* currentDir = 0L;
    if(m_archive == 0) exit(1);
    m_archive->fileName();
    m_archive->directory();
    const KArchiveDirectory* rootDir = m_archive->directory();
    const QStringList entries = rootDir->entries();
    // Will be reused later.
    QStringList::ConstIterator entriesIt;
    for (entriesIt = entries.begin(); entriesIt != entries.end(); ++entriesIt) {
        currentEntry = const_cast<KArchiveEntry*>(rootDir->entry(*entriesIt));
        if (currentEntry->isDirectory()) {
            currentDir = dynamic_cast<KArchiveDirectory*>(currentEntry);
            if (currentDir) {
                if (currentDir->entry(QString::fromUtf8("Emoticons.plist"))) {
                   QString currentItem = currentEntry->name();
                   if(m_bundleName.isEmpty() && currentItem.endsWith(QLatin1String(".AdiumEmoticonset"))) {
                       m_bundleName = currentItem.remove(QLatin1String(".AdiumEmoticonset"));
                   }
                   return BundleValid;
                }
            }
        }
    }

    return BundleNotValid;
}

QString EmoticonSetInstaller::bundleName() const
{
    return m_bundleName;
}

BundleInstaller::BundleStatus EmoticonSetInstaller::install()
{
    KEmoticons emoticons;
    emoticons.installTheme(m_tmpFile->fileName());

    // we trust in KEmoticons as it gives us no status information
    // installTheme only returns a list of installed themes if we compare the list before and after
    // the style could have been updated and the list would not have changed 
    Q_EMIT finished(BundleInstallOk);
    return BundleInstallOk;
}

void EmoticonSetInstaller::showRequest()
{
    KNotification *notification = new KNotification(QLatin1String("emoticonsRequest"), NULL, KNotification::Persistent);
    notification->setText( i18n("Install Emoticonset %1", this->bundleName()) );
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

void EmoticonSetInstaller::showResult()
{
    KNotification *notification = new KNotification(QLatin1String("emoticonsSuccess"));
    notification->setText( i18n("Installed Emoticonset %1 successfully.", this->bundleName()) );
    notification->setComponentName(ktelepathyComponentName());
    notification->sendEvent();

    Q_EMIT showedResult();
}

void EmoticonSetInstaller::ignoreRequest()
{
    Q_EMIT ignoredRequest();
}

#include "emoticon-set-installer.moc"
