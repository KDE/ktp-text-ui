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
    QRegExp bugText;
    QString sectionTemplate;
};

BugzillaFilter::BugzillaFilter(QObject *parent, const QVariantList &) :
    AbstractMessageFilter(parent), d(new Private)
{
    d->bugText = QRegExp(QLatin1String("BUG:[ ]*(\\d+)"));
    d->sectionTemplate = QLatin1String("<br/>[BUG <a href='%1'>%2</a>] %3 - %4");
}

BugzillaFilter::~BugzillaFilter()
{
    delete d;
}

void BugzillaFilter::addBugDescription(KTp::Message &msg, const KUrl &baseUrl) {
    KUrl request;
    request.setHost(baseUrl.host());
    request.setProtocol(baseUrl.protocol());
    request.setDirectory(baseUrl.directory());
    request.setFileName(QLatin1String("jsonrpc.cgi"));

    request.addQueryItem(QLatin1String("method"), QLatin1String("Bug.get"));
    request.addQueryItem(QLatin1String("params"),
                         QString(QLatin1String("[{\"ids\":[%1]}]")).
                         arg(baseUrl.queryItemValue(QLatin1String("id"))));

    kDebug() << request;
    KIO::StoredTransferJob *job = KIO::storedGet(request);
    job->exec();


    QVariantMap response = QJson::Parser().parse(job->data()).toMap();
    if (response.contains(QLatin1String("result"))) {
        QVariantMap result = response.value(QLatin1String("result")).toMap();
        if (result.contains(QLatin1String("bugs"))) {
            QVariantList bugs = result.value(QLatin1String("bugs")).toList();
            if (!bugs.isEmpty()) {
                QVariantMap bug = bugs.first().toMap();
                if (bug.contains(QLatin1String("summary"))) {
                    QString summary = bug.value(QLatin1String("summary")).toString();
                    QString status = bug.value(QLatin1String("status")).toString();

                    if (status == QLatin1String("RESOLVED")) {
                        status += QString::fromLatin1(" (%1)").arg(bug.value(QLatin1String("resolution")).toString());
                    }

                    msg.appendMessagePart(d->sectionTemplate
                        .arg(baseUrl.url())
                        .arg(Qt::escape(baseUrl.queryItemValue(QLatin1String("id"))))
                        .arg(Qt::escape(summary))
                        .arg(status));
                }
            }
        }
    }

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
            kDebug() << "link is to a bugzilla:" << url.host();

            addBugDescription(message, url);
        }
    }
}

K_PLUGIN_FACTORY(MessageFilterFactory, registerPlugin<BugzillaFilter>();)
K_EXPORT_PLUGIN(MessageFilterFactory("ktptextui_message_filter_bugzilla"))
