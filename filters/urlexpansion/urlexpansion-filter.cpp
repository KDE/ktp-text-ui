/*
 *    Copyright (C) 2013  Anant Kamath <kamathanant@gmail.com>
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

#include "urlexpansion-filter.h"
#include <QtCore/qjsondocument.h>

#include<QFile>

#include <KPluginFactory>
#include <KDE/KStandardDirs>
#include <KGlobal>

class UrlExpansionFilter::Private
{
public:
    Private() {
        requestCounter = 0;
    }
    int requestCounter;
    QStringList supportedServices;
};

UrlExpansionFilter::UrlExpansionFilter(QObject *parent, const QVariantList &) :
    AbstractMessageFilter(parent), d(new Private)
{
    getSupportedServices();
}

UrlExpansionFilter::~UrlExpansionFilter()
{
    delete d;
}

void UrlExpansionFilter::getSupportedServices()
{
    QFile servicesFile(KGlobal::dirs()->findResource("data", QLatin1String("ktelepathy/longurlServices.json")));
    bool b = servicesFile.open(QIODevice::ReadOnly);
    Q_ASSERT(b);
    QVariantMap response = QJsonDocument::fromJson(servicesFile.readAll()).toVariant().toMap();
    d->supportedServices = response.uniqueKeys();
}

void UrlExpansionFilter::addExpandedUrl(KTp::Message &message, const KUrl &url)
{
    d->requestCounter++;
    QString urlId = QString((QLatin1String("url") + QString::number(d->requestCounter)));
    QString callbackFunction = QString((QLatin1String("expandUrlCallbacks.") + urlId));
    KUrl request = KUrl(QLatin1String("http://api.longurl.org/v2/expand"));
    request.addQueryItem(QLatin1String("url"), url.url());
    request.addQueryItem(QLatin1String("format"), QLatin1String("json"));
    request.addQueryItem(QLatin1String("callback"), callbackFunction);
    request.addQueryItem(QLatin1String("user-agent"), QLatin1String("KTp"));

    message.appendMessagePart(QString::fromLatin1("<p id = \"%1\">Redirects to </p>").arg(urlId));
    message.appendScript(QString::fromLatin1("showShortUrl(\"%1\",\"%2\");").arg(request.prettyUrl(),urlId));
}

void UrlExpansionFilter::filterMessage(KTp::Message &message, const KTp::MessageContext &context)
{
    //if we're hidden we don't want to make network requests that can show we're online
    if (context.account()->currentPresence().type() == Tp::ConnectionPresenceTypeHidden) {
        return;
    }

    Q_FOREACH (const QVariant &var, message.property("Urls").toList()) {
        KUrl url = qvariant_cast<KUrl>(var);

        if (!url.path().isEmpty() && QString::compare(url.path(),QLatin1String("/")) && d->supportedServices.contains(url.host())) {
            addExpandedUrl(message, url);
        }
    }
}

QStringList UrlExpansionFilter::requiredScripts()
{
    return QStringList() << QLatin1String("ktelepathy/longurl.js");
}

K_PLUGIN_FACTORY(MessageFilterFactory, registerPlugin<UrlExpansionFilter>();)
K_EXPORT_PLUGIN(MessageFilterFactory("ktptextui_message_filter_urlexpansion"))

#include "urlexpansion-filter.moc"
