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
    QString sectionTemplate;
    int filterId;
};

BugzillaFilter::BugzillaFilter(QObject *parent, const QVariantList &) :
    AbstractMessageFilter(parent), d(new Private)
{
    d->bugText = QRegExp(QLatin1String("BUG:[ ]*(\\d+)"));
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
            addBugDescription(message, url);
        }
    }
}

QStringList BugzillaFilter::requiredScripts()
{
    return QStringList() << QLatin1String("ktelepathy/showBugzillaInfo.js");
}

K_PLUGIN_FACTORY(MessageFilterFactory, registerPlugin<BugzillaFilter>();)
K_EXPORT_PLUGIN(MessageFilterFactory("ktptextui_message_filter_bugzilla"))
