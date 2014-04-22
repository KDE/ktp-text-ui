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

#include "simplestimagehostingsharer.h"

#include <QDebug>
#include <QRegExp>

SimplestImageHostingSharer::SimplestImageHostingSharer(const QString& contentPath): AbstractSharer(contentPath)
{
}

KUrl SimplestImageHostingSharer::url() const
{
    KUrl url("http://api.simplest-image-hosting.net/upload:image,default");
    return url;
}

void SimplestImageHostingSharer::parseResponse(const QByteArray& responseData)
{
    QString responseString = QLatin1String(responseData);
    QRegExp exp(QLatin1String("800\n(http://.+)\n"));
    if ( exp.indexIn(responseString) != -1) {
        m_imageUrl = responseString.replace(QLatin1String("800"), QLatin1String("")).replace(QLatin1String("\n"), QLatin1String(""));
    } else {
        m_hasError = true;
        m_errorMessage = responseString;
    }
}

QByteArray SimplestImageHostingSharer::postBody(const QByteArray& imageData)
{
    m_form.addFile(QLatin1String("fileName"), m_contentPath, imageData);
    m_form.finish();

    return m_form.formData();
}
