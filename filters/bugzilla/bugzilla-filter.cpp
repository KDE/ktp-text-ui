/*
 *    Copyright (C) 2012  Lasath Fernando <kde@lasath.org>
 *    Copyright (C) 2013  David Edmundson <kde@davidedmundson.co.uk>
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

#include "bugzilla-filter.h"

#include <KPluginFactory>

#include <QDebug>
#include <QUrl>

class BugzillaFilter::Private
{
public:
    Private() {
        requestCounter = 0;
    }

    QRegExp bugText;
    int requestCounter;
    QStringList bugzillaHosts;
};

BugzillaFilter::BugzillaFilter(QObject *parent, const QVariantList &) :
    AbstractMessageFilter(parent), d(new Private)
{
    d->bugText = QRegExp(QLatin1String("BUG:[ ]*(\\d+)"));
    d->bugText.setCaseSensitivity(Qt::CaseInsensitive);

    d->bugzillaHosts << QLatin1String("bugzilla.mozilla.org")
                     << QLatin1String("bugzilla.kernel.org")
                     << QLatin1String("bugzilla.gnome.org")
                     << QLatin1String("bugs.kde.org")
                     << QLatin1String("issues.apache.org")
                     << QLatin1String("www.openoffice.org")
                     << QLatin1String("bugs.eclipse.org/bugs")
                     << QLatin1String("bugzilla.redhat.com/bugzilla")
                     << QLatin1String("qa.mandriva.com")
                     << QLatin1String("bugs.gentoo.org")
                     << QLatin1String("bugzilla.novell.com");
}

BugzillaFilter::~BugzillaFilter()
{
    delete d;
}

void BugzillaFilter::addBugDescription(KTp::Message &message, const QUrl &baseUrl)
{
    QString bugRequestId((QLatin1String("bug_") + QString::number(d->requestCounter)));
    d->requestCounter++;

    QUrl request(baseUrl);
    request.setPath(QStringLiteral("/jsonrpc.cgi"));

    QUrlQuery query(request);
    QString id = query.queryItemValue(QStringLiteral("id"));
    // Clear the query because we're setting the id as local one for identification
    // and QUrlQuery just adds another id query item, so let's just clear it
    query.clear();

    query.addQueryItem(QStringLiteral("method"), QStringLiteral("Bug.get"));
    query.addQueryItem(QStringLiteral("params"), QStringLiteral("[{\"ids\":[%1]}]").arg(id));
    query.addQueryItem(QStringLiteral("callback"), QStringLiteral("showBugCallback"));
    query.addQueryItem(QStringLiteral("id"), bugRequestId);

    request.setQuery(query);

    message.appendMessagePart(QString::fromLatin1("<p><a href=\"%1\" id=\"%2\"></a></p>").arg(baseUrl.toDisplayString(), bugRequestId));
    message.appendScript(QString::fromLatin1("showBug(\"%1\");").arg(request.toDisplayString()));
}

void BugzillaFilter::filterMessage(KTp::Message &message, const KTp::MessageContext &context)
{
    //if we're hidden we don't want to make network requests that can show we're online
    if (context.account()->currentPresence().type() == Tp::ConnectionPresenceTypeHidden) {
        return;
    }

    QString msg = message.mainMessagePart();
    int index = msg.indexOf(d->bugText);
    while (index >= 0) {
        QUrl baseUrl;

        //TODO make this configurable
        baseUrl.setScheme(QLatin1String("https"));
        baseUrl.setHost(QLatin1String("bugs.kde.org"));
        baseUrl.setPath(QLatin1String("/show_bug.cgi"));

        QUrlQuery query(baseUrl);
        query.addQueryItem(QLatin1String("id"), d->bugText.cap(1));

        addBugDescription(message, baseUrl);

        index = msg.indexOf(d->bugText, index + 1);
    }

    Q_FOREACH (QVariant var, message.property("Urls").toList()) {
        QUrl url = qvariant_cast<QUrl>(var);

        if (url.path().contains(QLatin1String("show_bug.cgi"))) { //a bugzilla of some sort

            //as we have to use jsonp to get round making a cross-domain http request, a malicious website
            //could pretend to be bugzilla and return arbitrary data that we cannot sanitise, filling the text-ui
            //then someone could send a link potentially executing random JS.
            //somewhat unlikely..but better safe than sorry.
            //QML rewrite will fix it, as that does not have security origin checks on XHttpRequest objects

            //Do not try and make this plugin more generic by removing this check unless you know what you are doing.

            //check hostname against a whitelist of bugzilla instances

            //TODO as we are checking the hostname we can support host/bugID formats
            //TODO make this configurable in config

            if (d->bugzillaHosts.contains(url.host())) {
                addBugDescription(message, url);
            }
        }
    }
}

QStringList BugzillaFilter::requiredScripts()
{
    return QStringList() << QLatin1String("ktelepathy/showBugzillaInfo.js");
}

K_PLUGIN_FACTORY(MessageFilterFactory, registerPlugin<BugzillaFilter>();)
// K_EXPORT_PLUGIN(MessageFilterFactory("ktptextui_message_filter_bugzilla"))

#include "bugzilla-filter.moc"
