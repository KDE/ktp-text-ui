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

#ifndef IMGURSHARER_H
#define IMGURSHARER_H

#include "abstractsharer.h"

class ImgurSharer : public AbstractSharer
{
public:
    ImgurSharer(const QString& contentPath);

    QUrl url() const;
    QByteArray postBody(const QByteArray &imageData);
    virtual void parseResponse(const QByteArray& responseData);
};

#endif // IMGURSHARER_H
