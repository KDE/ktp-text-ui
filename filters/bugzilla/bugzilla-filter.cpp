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
#include <KUriFilter>
#include <KIO/Job>

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
    d->sectionTemplate = QLatin1String("[BUG <a href='https://%1/show_bug.cgi?id=%2'>%2</a>] %3");
}

BugzillaFilter::~BugzillaFilter()
{
    delete d;
}

void BugzillaFilter::appendSection(Message &msg, const QString &host, const QString &bugId) {
    KUrl request;
    request.setHost(host);
    request.setProtocol(QLatin1String("https"));
    request.setFileName(QLatin1String("jsonrpc.cgi"));

    request.addQueryItem(QLatin1String("method"), QLatin1String("Bug.get"));
    request.addQueryItem(QLatin1String("params"), QString(QLatin1String("[{\"ids\":[%1]}]")).arg(bugId));

    kDebug() << request;
    KIO::StoredTransferJob *job = KIO::storedGet(request);
    job->exec();

    //BAM!
    QString description = QJson::Parser().parse(job->data()).toMap()[QLatin1String("result")].toMap()[QLatin1String("bugs")].toList()[0].toMap()[QLatin1String("summary")].toString();

    kDebug() << description;
    msg.appendMessagePart(d->sectionTemplate.arg(host).arg(bugId).arg(description));
}

void BugzillaFilter::filterMessage(Message &message)
{
    QString msg = message.mainMessagePart();
    int index = msg.indexOf(d->bugText);
    while (index >= 0) {
        appendSection(message, QLatin1String("bugs.kde.org"), d->bugText.cap(1));
        index = msg.indexOf(d->bugText, index + 1);
    }

    Q_FOREACH (QVariant var, message.property("Urls").toList()) {
        KUrl url = qvariant_cast<KUrl>(var);

        if (url.fileName() == QLatin1String("show_bug.cgi")) { //a bugzilla of some sort
            kDebug() << "link is to a bugzilla:" << url.host();

            //by using the host from the link, it will work with *any* bugzilla installation
            appendSection(message, url.host(), url.queryItemValue(QLatin1String("id")));
        }
    }
}

K_PLUGIN_FACTORY(MessageFilterFactory, registerPlugin<BugzillaFilter>();)
K_EXPORT_PLUGIN(MessageFilterFactory("ktptextui_message_filter_bugzilla"))
