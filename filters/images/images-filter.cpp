/*
 *    Copyright (C) 2012  Lasath Fernando <kde@lasath.org>
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 2.1 of the License, or (at your option) any later version.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "images-filter.h"

#include <QRegExp>

#include <KPluginFactory>
#include <KDebug>
#include <KUrl>
#include <QImageReader>

class ImagesFilter::Private {
public:
    QRegExp imageRegex;
};

ImagesFilter::ImagesFilter (QObject* parent, const QVariantList&) :
    AbstractMessageFilter (parent), d(new Private)
{
    QString imagePattern = QLatin1String("\\.(?:");
    Q_FOREACH (const QByteArray &format, QImageReader::supportedImageFormats()) {
        imagePattern += QString::fromAscii(format) + QLatin1String("|");
    }
    imagePattern.chop(1);
    imagePattern += QLatin1String(")$");

    d->imageRegex = QRegExp(imagePattern);
}

void ImagesFilter::filterMessage (Message& message)
{
    kDebug() << message.property("Urls").toList().size();
    Q_FOREACH (QVariant var, message.property("Urls").toList()) {
        KUrl url = qvariant_cast<KUrl>(var);
        QString fileName = url.fileName();
//         qDebug() << fileName;

        if (!fileName.isNull() && fileName.contains(d->imageRegex)) {
            message.appendMessagePart(
                QLatin1String("<img src='") +
                QString::fromAscii(url.toEncoded()) +
                QLatin1String("' alt='Link is of an Image'")
            );
        }
    }
}

K_PLUGIN_FACTORY(MessageFilterFactory, registerPlugin<ImagesFilter>();)
K_EXPORT_PLUGIN(MessageFilterFactory("ktptextui_message_filter_images"))