#include "emoticonsetinstaller.h"
#include <chatwindowstylemanager.h>
#include <chatstyleplistfilereader.h>
#include <KDebug>
#include <KTemporaryFile>
#include <KArchiveFile>
#include <KEmoticons>
#include <KArchiveDirectory>


EmoticonSetInstaller::EmoticonSetInstaller(KArchive *archive, KTemporaryFile *tmpFile)
{
    kDebug();

    m_archive = archive;
    m_tmpFile = tmpFile;
}

BundleInstaller::BundleStatus EmoticonSetInstaller::validate()
{
    kDebug();

    KArchiveEntry *currentEntry = 0L;
    KArchiveDirectory* currentDir = 0L;
    if(m_archive == 0) exit(1);
    m_archive->fileName();
    m_archive->directory();
    const KArchiveDirectory* rootDir = m_archive->directory();
    const QStringList entries = rootDir->entries();
    // Will be reused later.
    QStringList::ConstIterator entriesIt, entriesItEnd = entries.end();
    for (entriesIt = entries.begin(); entriesIt != entries.end(); ++entriesIt) {
        currentEntry = const_cast<KArchiveEntry*>(rootDir->entry(*entriesIt));
        kDebug() << "Current entry name: " << currentEntry->name();
        if (currentEntry->isDirectory()) {
            currentDir = dynamic_cast<KArchiveDirectory*>(currentEntry);
            if (currentDir) {
                if (currentDir->entry(QString::fromUtf8("Emoticons.plist"))) {
                   kDebug() << "Emoticons.plist found";
                   QString currentItem = currentEntry->name();
                   if(m_bundleName.isEmpty() && currentItem.endsWith(".AdiumEmoticonset")) {
                       m_bundleName = currentItem.remove(".AdiumEmoticonset");
                   }
                   return BundleValid;
                }
            }
        }
    }

    return BundleNotValid;
}

QString EmoticonSetInstaller::bundleName()
{
    kDebug();

    return m_bundleName;
}

BundleInstaller::BundleStatus EmoticonSetInstaller::install()
{
    kDebug();

    KEmoticons emoticons;
    emoticons.installTheme(m_tmpFile->fileName());

    emit(finished());
    return BundleInstallOk;
}
