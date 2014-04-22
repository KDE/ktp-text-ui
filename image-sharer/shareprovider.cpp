/*
 *
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

#include "shareprovider.h"
#include "abstractsharer.h"

#include "imgursharer.h"
#include "simplestimagehostingsharer.h"
#include "imagebinsharer.h"

#include <QDebug>
#include <KIO/Job>
#include <KIO/FileJob>
#include <KIO/TransferJob>
#include <KIO/MimetypeJob>
#include <KLocalizedString>

class ShareProviderPrivate
{
public:
    ShareProviderPrivate() : m_sharer(0) {

    }
    ~ShareProviderPrivate() {
        delete m_sharer;
    }
    AbstractSharer* getSharer();

    ShareProvider::ShareService m_shareServiceType;
    QByteArray m_data;
    QString m_filePath;
    AbstractSharer *m_sharer;

};

AbstractSharer* ShareProviderPrivate::getSharer()
{
    if (m_sharer) {
        return m_sharer;
    }

    switch (this->m_shareServiceType) {
    case ShareProvider::Imgur:
        m_sharer = new ImgurSharer(m_filePath);
        break;
    case ShareProvider::SimplestImageHosting:
        m_sharer = new SimplestImageHostingSharer(m_filePath);
        break;
    case ShareProvider::ImageBin:
        m_sharer = new ImageBinSharer(m_filePath);
        break;
    }
    return m_sharer;
}


ShareProvider::ShareProvider(ShareService shareServiceType, QObject *parent)
    : QObject(parent),
      d(new ShareProviderPrivate)
{
    d->m_shareServiceType = shareServiceType;
}

ShareProvider::~ShareProvider()
{
    delete d;
}

QMap< QString, ShareProvider::ShareService > ShareProvider::availableShareServices()
{
    QMap<QString, ShareProvider::ShareService> availableServices;
    availableServices.insert(QLatin1String("Imgur"), Imgur);
    availableServices.insert(QLatin1String("Simplest Image Hosting"), SimplestImageHosting);
    availableServices.insert(QLatin1String("Imagebin"), ImageBin);
    return availableServices;
}


void ShareProvider::setShareServiceType(ShareService shareServiceType)
{
    delete d->m_sharer;
    // d->m_sharer is going to be reinitialized with the next call of getSharer
    d->m_sharer = 0;

    d->m_shareServiceType = shareServiceType;
}

ShareProvider::ShareService ShareProvider::shareServiceType() const
{
    return d->m_shareServiceType;
}

void ShareProvider::publish(const QString& filePath)
{
    d->m_filePath = filePath;
    KUrl fileUrl(filePath);

    KIO::MimetypeJob *mimetypeJob = KIO::mimetype(fileUrl, KIO::HideProgressInfo);
    connect(mimetypeJob, SIGNAL(finished(KJob*)), this, SLOT(onMimetypeJobFinished(KJob*)));
}

void ShareProvider::onMimetypeJobFinished(KJob* job)
{
    KIO::MimetypeJob *mjob = qobject_cast<KIO::MimetypeJob *>(job);
    if (!mjob) {
        return;
    }

    if (mjob->error()) {
        Q_EMIT finishedError(this, mjob->errorString());
        return;
    }

    QString mimeType = mjob->mimetype();
    if (mimeType.isEmpty()) {
        Q_EMIT finishedError(this, i18n("Could not detect the file's mimetype"));
        return;
    }

    if (!mimeType.startsWith(QLatin1String("image/"))) {
        Q_EMIT finishedError(this, i18n("File Type is not an image"));
        return;
    }

    KIO::FileJob *fjob = KIO::open(mjob->url(), QIODevice::ReadOnly);
    connect(fjob, SIGNAL(open(KIO::Job*)), this, SLOT(onFileOpened(KIO::Job*)));

    mjob->deleteLater();
}

void ShareProvider::onFileOpened(KIO::Job* job)
{
    // finished opening the file, now try to read it's content
    KIO::FileJob *fjob = static_cast<KIO::FileJob*>(job);
    fjob->read(fjob->size());
    connect(fjob, SIGNAL(data(KIO::Job*,QByteArray)),
            this, SLOT(onFinishedReadingFile(KIO::Job*,QByteArray)));
}

void ShareProvider::onFinishedReadingFile(KIO::Job* job, const QByteArray& data)
{
    job->disconnect(this);
    qobject_cast<KIO::FileJob *>(job)->close();

    if (data.length() == 0) {
        Q_EMIT finishedError(this, i18n("It was not possible to read the selected file"));
        return;
    }

    d->m_data.clear();

    AbstractSharer *sharer = d->getSharer();
    if (sharer) {
        KUrl sharerUrl = sharer->url();
        if (!sharerUrl.isValid()) {
            Q_EMIT finishedError(this, i18n("Service Url is not valid"));
            return;
        }
        KIO::TransferJob *tJob = KIO::http_post(sharer->url(), sharer->postBody(data), KIO::HideProgressInfo);
        tJob->setMetaData(sharer->headers());
        connect(tJob, SIGNAL(data(KIO::Job*,QByteArray)),
                this, SLOT(onTransferJobDataReceived(KIO::Job*,QByteArray)));
        connect(tJob, SIGNAL(result(KJob*)), this, SLOT(onTransferJobResultReceived(KJob*)));

    }
}

void ShareProvider::onTransferJobDataReceived(KIO::Job* job, QByteArray data)
{
    Q_UNUSED(job);
    d->m_data.append(data);
}

void ShareProvider::onTransferJobResultReceived(KJob* job)
{
    if (d->m_data.size() == 0) {
        Q_EMIT finishedError(this, i18n("Service was not available"));
        return;
    }

    KIO::TransferJob *tfJob = qobject_cast<KIO::TransferJob*>(job);
    if (tfJob) {
        QString mimeType = tfJob->mimetype();
        AbstractSharer *sharer = d->getSharer();

        if (sharer) {
            sharer->parseResponse(d->m_data);
            if (tfJob->isErrorPage() || sharer->hasError()) {
                QString errorMessage = sharer->errorMessage();
                if (!errorMessage.isEmpty()) {
                    Q_EMIT finishedError(this, errorMessage);
                } else {
                    Q_EMIT finishedError(this, tfJob->errorString());
                }
            } else {
                Q_EMIT finishedSuccess(this, sharer->imageUrl().url());
            }
        }
    }
}


