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

#include "imagebinsharer.h"

#include <QDebug>

ImageBinSharer::ImageBinSharer(const QString& contentPath) : AbstractSharer(contentPath)
{
}

KUrl ImageBinSharer::url() const
{
    return KUrl("http://imagebin.ca/upload.php");
}

void ImageBinSharer::parseResponse(const QByteArray& responseData)
{
    //Sample Response String that contains the url
    // "status:1I0zzt2xu949
    // url:http://ibin.co/1I0zzt2xu949

    QString responseString = QString(QLatin1String(responseData));
    QString urlPrefix = QLatin1String("url:");
    int urlPrefixIndex = responseString.indexOf(urlPrefix);
    if (urlPrefixIndex != -1) {
        QString imageUrl = responseString.mid(urlPrefixIndex + urlPrefix.length()).trimmed();
        m_imageUrl = KUrl(imageUrl);
    } else {
        m_hasError = true;
        m_errorMessage = responseString.replace(QLatin1String("status:error:"), QLatin1String(""));
    }
}

QByteArray ImageBinSharer::postBody(const QByteArray& imageData)
{
    m_form.addFile(QLatin1String("file"), m_contentPath, imageData);
    m_form.finish();
    return m_form.formData();
}
