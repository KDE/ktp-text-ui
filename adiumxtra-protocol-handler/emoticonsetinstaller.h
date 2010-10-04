#ifndef EMOTICONSETINSTALLER_H
#define EMOTICONSETINSTALLER_H

#include "bundleinstaller.h"

#include <KDebug>

class KTemporaryFile;

class EmoticonSetInstaller : public BundleInstaller
{
    Q_OBJECT

    public:
        EmoticonSetInstaller(KArchive *archive, KTemporaryFile *tmpFile);
        BundleStatus validate();
        QString bundleName();
        void showRequest();
        void showResult();
        ~EmoticonSetInstaller() { kDebug(); };

    public Q_SLOTS:
        void ignoreRequest();
        BundleStatus install();

private:
    KArchive *m_archive;
    KTemporaryFile *m_tmpFile;
    QString m_bundleName;

};

#endif // EMOTICONSETINSTALLER_H
