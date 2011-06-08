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

#ifndef BUNDLEINSTALLER_H
#define BUNDLEINSTALLER_H

#include <KArchive>

class BundleInstaller : public QObject
{
    Q_OBJECT

public:
    virtual ~BundleInstaller();

    enum BundleStatus { BundleInstallOk = 0, BundleNotValid, BundleNoDirectoryValid,
                        BundleCannotOpen, BundleUnknownError, BundleValid };
    virtual BundleStatus validate() = 0;
    virtual QString bundleName() const = 0;

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
