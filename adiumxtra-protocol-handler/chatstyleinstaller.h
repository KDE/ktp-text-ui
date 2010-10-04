#ifndef CHATSTYLEINSTALLER_H
#define CHATSTYLEINSTALLER_H

#include "bundleinstaller.h"

#include <KDebug>

class KTemporaryFile;

class ChatStyleInstaller : public BundleInstaller
{
    Q_OBJECT

    public:
        ChatStyleInstaller(KArchive *archive, KTemporaryFile *tmpFile);
        BundleInstaller::BundleStatus validate();
        QString bundleName();
        ~ChatStyleInstaller() { kDebug(); };

    public Q_SLOTS:
        BundleInstaller::BundleStatus install();

    private:
        KArchive *m_archive;
        KTemporaryFile *m_tmpFile;
        QString m_bundleName;

};

#endif // CHATSTYLEINSTALLER_H
