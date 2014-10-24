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

#include "imgursharer.h"

#include <QString>
#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonobject.h>
#include <KUrl>

// Taken from "share" Data Engine
// key associated with plasma-devel@kde.org
// thanks to Alan Schaaf of Imgur (alan@imgur.com)
static const QString apiKey = QLatin1String("d0757bc2e94a0d4652f28079a0be9379");

ImgurSharer::ImgurSharer(const QString& contentPath): AbstractSharer(contentPath)
{
}


KUrl ImgurSharer::url() const
{
    KUrl url(QLatin1String("https://api.imgur.com/2/upload.json"));
    url.addQueryItem(QLatin1String("key"), apiKey);
    return url;
}

QByteArray ImgurSharer::postBody(const QByteArray &imageData)
{
    // Create the request body
    m_form.addFile(QLatin1String("image"), m_contentPath, imageData);
    m_form.finish();
    return m_form.formData();
}

void ImgurSharer::parseResponse(const QByteArray& responseData)
{
    QJsonDocument parser;
    QJsonParseError error;
    QJsonObject resultMap = parser.fromJson(responseData, &error).object();
    if (error.error) {
        m_hasError = true;
        m_errorMessage = error.errorString();
    } else if ( resultMap.contains(QLatin1String("error")) ) {
        m_hasError = true;
        QJsonObject errorMap = resultMap[QLatin1String("error")].toObject();
        m_errorMessage = errorMap[QLatin1String("message")].toString();
    } else {
        QJsonObject uploadMap = resultMap[QLatin1String("upload")].toObject();
        QJsonObject linksMap = uploadMap[QLatin1String("links")].toObject();
        m_imageUrl = KUrl(linksMap[QLatin1String("original")].toString());
    }
}
