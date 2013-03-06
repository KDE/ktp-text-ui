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

#include <qjson/parser.h>

#include <KPluginFactory>
#include <KDebug>
#include <KUrl>
#include <KIO/Job>
#include <QTextDocument>

class BugzillaFilter::Private
{
public:
    Private() {
        filterId = 0;
    }

    QRegExp bugText;
    int filterId;
    QStringList bugzillaHosts;
};

BugzillaFilter::BugzillaFilter(QObject *parent, const QVariantList &) :
    AbstractMessageFilter(parent), d(new Private)
{
    d->bugText = QRegExp(QLatin1String("BUG:[ ]*(\\d+)"));

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

void BugzillaFilter::addBugDescription(KTp::Message &message, const KUrl &baseUrl) {
    QString bugRequestId((QLatin1String("bug_") + QString::number(d->filterId)));
    d->filterId++;

    KUrl request;
    request.setHost(baseUrl.host());
    request.setProtocol(baseUrl.protocol());
    request.setDirectory(baseUrl.directory());
    request.setFileName(QLatin1String("jsonrpc.cgi"));

    request.addQueryItem(QLatin1String("method"), QLatin1String("Bug.get"));
    request.addQueryItem(QLatin1String("params"),
                         QString(QLatin1String("[{\"ids\":[%1]}]")).
                         arg(baseUrl.queryItemValue(QLatin1String("id"))));

    request.addQueryItem(QLatin1String("callback"), QLatin1String("showBugCallback"));
    request.addQueryItem(QLatin1String("id"), bugRequestId);

    message.appendMessagePart(QString::fromLatin1("<p><a href=\"%1\" id=\"%2\"></a></p>").arg(baseUrl.prettyUrl(), bugRequestId));
    message.appendScript(QString::fromLatin1("showBug(\"%1\");").arg(request.prettyUrl()));
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
        KUrl baseUrl;

        //TODO make this configurable
        baseUrl.setProtocol(QLatin1String("https"));
        baseUrl.setHost(QLatin1String("bugs.kde.org"));
        baseUrl.setFileName(QLatin1String("show_bug.cgi"));
        baseUrl.addQueryItem(QLatin1String("id"), d->bugText.cap(1));

        addBugDescription(message, baseUrl);

        index = msg.indexOf(d->bugText, index + 1);
    }

    Q_FOREACH (QVariant var, message.property("Urls").toList()) {
        KUrl url = qvariant_cast<KUrl>(var);

        if (url.fileName() == QLatin1String("show_bug.cgi")) { //a bugzilla of some sort

                        //add a check on the hostname against a whitelist.

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
K_EXPORT_PLUGIN(MessageFilterFactory("ktptextui_message_filter_bugzilla"))
