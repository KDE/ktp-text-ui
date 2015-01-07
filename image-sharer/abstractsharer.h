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

#ifndef ABSTRACTSHARER_H
#define ABSTRACTSHARER_H

#include <QByteArray>
#include <QString>
#include <QMap>
#include <QUrl>

#include "mpform.h"

/**
 * This class is going to be used inside the ServiceProvider class.
 * It is used to supply the share service connection information to the ServiceProvider.
 * By the information, it means the URL, post body and the request headers.
 */

class AbstractSharer
{
public:
    AbstractSharer(const QString &contentPath);
    virtual ~AbstractSharer();

    /**
     * @return A KUrl that points to the uploading endpoint of the Share Service
     */
    virtual QUrl url() const = 0;

    /**
     * @param imageData The image bytes that needs to be put inside the post body.
     * @return The post body that the ServiceProvider will send to the URL
     * */
    virtual QByteArray postBody(const QByteArray &imageData) = 0;

    /**
     * When the request is sent and the response is received this method is called.
     * You will need to parse the response here and determine if the request was a success
     * or a failure.
     * @param responseData The response received from the server.
     * */
    virtual void parseResponse(const QByteArray &responseData) = 0;

    /**
     * @return true if the parsed response has an error
     */
    virtual bool hasError() const;

    /**
     * @return a string that contains the error message. If the server response has an error
     * this method should return it.
     * */
    virtual QString errorMessage() const;

    /**
     * @return A URL that contains the image url from the service if the response is success.
     */
    virtual QUrl imageUrl() const;

    /**
     * You should override this if you want to customize the headers sent to the server.
     * The default is Content-Type: multipart/form-data .
     */
    virtual QMap<QString, QString> headers() const;

protected:
    QString m_contentPath;
    QString m_errorMessage;
    MPForm m_form;
    QUrl m_imageUrl;
    bool m_hasError;
};

#endif // ABSTRACTSHARER_H
