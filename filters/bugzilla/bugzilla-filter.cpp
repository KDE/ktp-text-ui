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

#include <KPluginFactory>
#include <KDebug>
#include <KUrl>
#include <KUriFilter>
#include <KIO/Job>

class BugzillaFilter::Private
{
public:
    QRegExp bugText;
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

void BugzillaFilter::filterMessage(Message &message)
{
//     KIO does not seem to let me do this syncrhonously, so I'll come back to this when
//     we have an async plugin system.
//     KUrl request = KUrl(QLatin1String("https://bugs.kde.org/jsonrpc.cgi"));
//     request.addQueryItem(QLatin1String("method"), QLatin1String("Bug.get"));
//     request.addQueryItem(QLatin1String("params"), QLatin1String("[{\"ids\":[300592]}]"));
// 
//     KIO::TransferJob* job = KIO::get(request);
//     job->

    QString section = QLatin1String("[BUG %1] https://bugs.kde.org/show_bug.cgi?id=%1");

    QString msg = message.mainMessagePart();
    int index = msg.indexOf(d->bugText);
    while (index >= 0) {
        message.appendMessagePart(section.arg(d->bugText.cap(1)));
        index = msg.indexOf(d->bugText, index + 1);
    }
}

K_PLUGIN_FACTORY(MessageFilterFactory, registerPlugin<BugzillaFilter>();)
K_EXPORT_PLUGIN(MessageFilterFactory("ktptextui_message_filter_bugzilla"))
