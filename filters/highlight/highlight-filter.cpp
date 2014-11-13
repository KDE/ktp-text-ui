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

#include "highlight-filter.h"

#include <QStringBuilder>

#include <KPluginFactory>
#include <KDebug>
#include <KUser>

#include <TelepathyQt/AccountManager>

class HighlightFilter::Private
{
public:
    KUser user;
};

HighlightFilter::HighlightFilter(QObject *parent, const QVariantList&) :
    KTp::AbstractMessageFilter(parent),
    d(new Private)
{
}

HighlightFilter::~HighlightFilter()
{
    delete d;
}

void HighlightFilter::filterMessage(KTp::Message &message,
                                    const KTp::MessageContext &context)
{
    QString msg = message.mainMessagePart();

    if (msg.contains(context.account()->nickname(), Qt::CaseInsensitive)) {
        message.setProperty("highlight", true);
    }

    if (msg.contains(d->user.loginName(), Qt::CaseInsensitive)) {
        kDebug() << "messge contains user login name" << d->user.loginName();
        message.setProperty("highlight", true);
    }
}

K_PLUGIN_FACTORY(MessageFilterFactory, registerPlugin<HighlightFilter>();)
K_EXPORT_PLUGIN(MessageFilterFactory("ktptextui_message_filter_highlight"))

#include "highlight-filter.moc"
