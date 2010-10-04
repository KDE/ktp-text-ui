#include "chatstyleinstaller.h"
#include <chatwindowstylemanager.h>
#include <chatstyleplistfilereader.h>
#include <KDebug>
#include <KTemporaryFile>
#include <KArchiveFile>


ChatStyleInstaller::ChatStyleInstaller(KArchive *archive, KTemporaryFile *tmpFile)
{
    kDebug();

    m_archive = archive;
    m_tmpFile = tmpFile;
}

bool ChatStyleInstaller::validate()
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
                   ChatStylePlistFileReader reader(info->data());
                   if(m_bundleName.isEmpty()) {
                       m_bundleName = reader.CFBundleName();
                   }
                    validResult += 1;
                }
            }
        }
    }

    if(validResult >= 2) {
        return true;
    } else {
        return false;
    }
}

QString ChatStyleInstaller::bundleName()
{
    kDebug();

    return m_bundleName;
}

bool ChatStyleInstaller::install()
{
    kDebug();

    int status = ChatWindowStyleManager::self()->installStyle(m_archive->fileName());
    kDebug()<< "status " << status;
    delete(m_tmpFile);

    emit(finished());

    if(!status) {
        return true;
    } else  {
        return false;
    }
}
