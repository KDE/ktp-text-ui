/*
 * Copyright (C) 2014  Ahmed I. Khalil <ahmedibrahimkhali@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef SHAREPROVIDER_H
#define SHAREPROVIDER_H

#include <QObject>
#include <QMap>

#include "imagesharer_export.h"

class ShareProviderPrivate;
class KJob;

namespace KIO {
class Job;
}

class IMAGESHARER_EXPORT ShareProvider : public QObject
{
    Q_OBJECT

public:
    // Available Image Share Services
    enum ShareService {
        Imgur,
        SimplestImageHosting,
        ImageBin
    };

    ShareProvider(ShareService shareServiceType, QObject *parent = 0);
    virtual ~ShareProvider();

    static QMap<QString, ShareService> availableShareServices();

    ShareService shareServiceType() const;
    void setShareServiceType(ShareService shareServiceType);

    void publish(const QString &filePath);

Q_SIGNALS:
    void finishedSuccess(ShareProvider *provider, const QString &url);
    void finishedError(ShareProvider *provider, const QString &msg);

protected Q_SLOTS:
    void onMimetypeJobFinished(KJob *job);
    void onFileOpened(KIO::Job *job);
    void onFinishedReadingFile(KIO::Job *job, const QByteArray &data);
    void onTransferJobDataReceived(KIO::Job *job, QByteArray data);
    void onTransferJobResultReceived(KJob *job);

private:
    ShareProviderPrivate * const d;
};

#endif // SHAREPROVIDER_H
