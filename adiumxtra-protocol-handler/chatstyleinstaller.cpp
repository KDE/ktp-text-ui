#include "chatstyleinstaller.h"
#include <chatwindowstylemanager.h>
#include <chatstyleplistfilereader.h>
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

BundleInstaller::BundleStatus ChatStyleInstaller::validate()
{
    kDebug();

    KArchiveEntry *currentEntry = 0L;
    KArchiveDirectory* currentDir = 0L;
    const KArchiveDirectory* rootDir = m_archive->directory();
    int validResult = 0;
    const QStringList entries = rootDir->entries();
    QStringList::ConstIterator entriesIt, entriesItEnd = entries.end();
    for (entriesIt = entries.begin(); entriesIt != entries.end(); ++entriesIt) {
        currentEntry = const_cast<KArchiveEntry*>(rootDir->entry(*entriesIt));
        kDebug() << "Current entry name: " << currentEntry->name();
        if (currentEntry->isDirectory()) {
            currentDir = dynamic_cast<KArchiveDirectory*>(currentEntry);
            if (currentDir) {
                if (currentDir->entry(QString::fromUtf8("Contents"))) {
                   kDebug() << "Contents found";
                    validResult += 1;
                }
                if (currentDir->entry(QString::fromUtf8("Contents/Info.plist"))) {
                   kDebug() << "Contents/Info.plist found";
                   KArchiveFile const *info = dynamic_cast<KArchiveFile const *>(currentDir->entry(QString::fromUtf8("Contents/Info.plist")));
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

QString ChatStyleInstaller::bundleName()
{
    kDebug();

    return m_bundleName;
}

BundleInstaller::BundleStatus ChatStyleInstaller::install()
{
    kDebug();

    BundleInstaller::BundleStatus status = static_cast<BundleInstaller::BundleStatus>(ChatWindowStyleManager::self()->installStyle(m_archive->fileName()));
    kDebug()<< "status " << status;
    delete(m_tmpFile);

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
