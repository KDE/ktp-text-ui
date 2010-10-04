#ifndef BUNDLEINSTALLER_H
#define BUNDLEINSTALLER_H

#include <KArchive>
#include <KDebug>

class BundleInstaller : public QObject
{
    Q_OBJECT

    public:
        enum BundleStatus { BundleInstallOk = 0, BundleNotValid, BundleNoDirectoryValid, BundleCannotOpen, BundleUnknownError, BundleValid };
        virtual BundleStatus validate() = 0;
        virtual QString bundleName() = 0;
        virtual ~BundleInstaller(){ kDebug(); };

    Q_SIGNALS:
        void ignoredRequest();
        void finished(BundleInstaller::BundleStatus status);
        void showedResult();

    public Q_SLOTS:
        virtual void showRequest() = 0;
        virtual BundleStatus install() = 0;
        virtual void showResult() = 0;
};

#endif // BUNDLEINSTALLER_H
