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

#include <QStringBuilder>
#include <QRegExp>
#include <QImageReader>
#include <QUrl>

#include <KPluginFactory>
#include <KLocalizedString>

class ImagesFilter::Private {
public:
    QList<QByteArray> formats;
};

ImagesFilter::ImagesFilter (QObject* parent, const QVariantList&) :
    KTp::AbstractMessageFilter (parent), d(new Private)
{
    d->formats = QImageReader::supportedImageFormats();
}

ImagesFilter::~ImagesFilter()
{
    delete d;
}

void ImagesFilter::filterMessage(KTp::Message &message, const KTp::MessageContext &context)
{
    Q_UNUSED(context)
    Q_FOREACH (const QVariant &var, message.property("Urls").toList()) {
        const QUrl url = qvariant_cast<QUrl>(var);
        QString fileName = url.adjusted(QUrl::StripTrailingSlash).fileName().toLower();

        //get everything after the . The +1 means we don't include the . character
        QString extension = fileName.mid(fileName.lastIndexOf(QLatin1Char('.'))+1);

        if (!fileName.isNull() && d->formats.contains(extension.toUtf8())) {
            QString href = QString::fromLatin1(url.toEncoded());
            message.appendMessagePart(
                QLatin1Literal("<br/><a href=\"") % href % QLatin1Literal("\">") %
                    QLatin1Literal("<img src=\"") %
                    href %
                    QLatin1Literal("\" style=\"max-width:100%;margin-top:3px\"/>") %
                QLatin1Literal("</a>")
            );
        }
    }
}

K_PLUGIN_FACTORY(MessageFilterFactory, registerPlugin<ImagesFilter>();)

#include "images-filter.moc"
