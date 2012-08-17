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
#include <KUrl>
#include <KUriFilter>
#include <KUser>

#include <TelepathyQt/AccountManager>

class HighlightFilter::Private
{
public:
    Tp::AccountManagerPtr accountManager;
    KUser user;
};

HighlightFilter::HighlightFilter(QObject *parent, const QVariantList &) :
    AbstractMessageFilter(parent), d(new Private)
{
    d->accountManager = Tp::AccountManager::create(QDBusConnection::sessionBus());
    d->accountManager->becomeReady();
}

HighlightFilter::~HighlightFilter()
{
    delete d;
}

void HighlightFilter::filterMessage(Message &message)
{
    QString msg = message.mainMessagePart();

    Q_FOREACH (Tp::AccountPtr ptr, d->accountManager->allAccounts()) {
        if (msg.contains(ptr->nickname(), Qt::CaseInsensitive)) {
            kDebug() << "message contains user alias :" << ptr->nickname();
            message.setProperty("highlight", true);
        }

        if (msg.contains(d->user.loginName(), Qt::CaseInsensitive)) {
            kDebug() << "messge contains user login name" << d->user.loginName();
            message.setProperty("highlight", true);
        }
    }

    if (message.property("highlight").toBool()){
        msg = QLatin1Literal("<div style='color:red;'>") % msg %
                QLatin1Literal("</div>");
        message.setMainMessagePart(msg);
    }
}

K_PLUGIN_FACTORY(MessageFilterFactory, registerPlugin<HighlightFilter>();)
K_EXPORT_PLUGIN(MessageFilterFactory("ktptextui_message_filter_highlight"))
