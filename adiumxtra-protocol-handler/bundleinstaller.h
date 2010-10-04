#ifndef BUNDLEINSTALLER_H
#define BUNDLEINSTALLER_H

#include "bundleinstaller.cpp"

#include <KArchive>
#include <KDebug>

class BundleInstaller : public QObject
{
    Q_OBJECT

    public:
        virtual bool validate() = 0;
        virtual QString bundleName() = 0;
        virtual ~BundleInstaller(){ kDebug(); };

    Q_SIGNALS:
        void finished();

    public Q_SLOTS:
        virtual bool install() = 0;
};

#endif // BUNDLEINSTALLER_H
