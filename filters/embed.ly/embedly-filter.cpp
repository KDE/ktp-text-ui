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

#include "embedly-filter.h"

#include <KPluginFactory>
#include <KDebug>
#include <KUrl>

#include <QtCore/QEventLoop>
#include <QtCore/QUrl>
#include <QtCore/QVariantMap>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include <qjson/parser.h>

#define EMBEDLY_URL "http://api.embed.ly/1/oembed?key=2f5b868fc3c44c4ba3e6397d3d1606a3&url=%1&callback=showEmbedlyCallback"

QJson::Parser qjson_parser;

EmbedlyFilter::EmbedlyFilter(QObject *parent, const QVariantList &)
    : AbstractMessageFilter(parent)
{
}

EmbedlyFilter::~EmbedlyFilter()
{
}

void EmbedlyFilter::filterMessage(KTp::Message &message, const KTp::MessageContext&)
{
    QStringList processedUrls;
    Q_FOREACH (const QVariant &var, message.property("Urls").toList()) {
        KUrl url = qvariant_cast<KUrl>(var);

        QUrl apiRequestUrl = QString::fromLatin1(EMBEDLY_URL).arg(url.url());

        QString part = message.mainMessagePart();
        part.replace(QString::fromLatin1("<a href=\"%1\">%1</a>").arg(url.url()),
                     QString::fromLatin1("<a href=\"%1\" id=\"%1\"></a>").arg(url.url()));
        message.setMainMessagePart(part);

        if (!processedUrls.contains(url.url())) {
            message.appendScript(QString::fromLatin1("embedly_processUrl(\"%1\");").arg(apiRequestUrl));
            processedUrls.append(url.url());
        }
    }
}

QStringList EmbedlyFilter::requiredScripts()
{
    return QStringList() << QLatin1String("ktelepathy/showEmbedlyInfo.js");
}

K_PLUGIN_FACTORY(MessageFilterFactory, registerPlugin<EmbedlyFilter>();)
K_EXPORT_PLUGIN(MessageFilterFactory("ktptextui_message_filter_embedly"))
