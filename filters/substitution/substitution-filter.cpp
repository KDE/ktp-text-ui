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

#include "substitution-filter.h"

#include <KPluginFactory>
#include <KDebug>
#include <KUrl>
#include <KUriFilter>

class SubstitutionFilter::Private
{
public:
    QMap<QString, QString> wordList;
};

SubstitutionFilter::SubstitutionFilter(QObject *parent, const QVariantList &) :
    AbstractMessageFilter(parent), d(new Private)
{
    reloadConfig();
}

SubstitutionFilter::~SubstitutionFilter()
{
    delete d;
}

void SubstitutionFilter::reloadConfig()
{
    if (!config().groupList().contains(QLatin1String("Replacements"))) {
        KConfigGroup g = config().group("Replacements");
        g.writeEntry("(TM)", "™");
        g.writeEntry(" dnt ", "don't");
        g.sync();
    }

    d->wordList = config().group("Replacements").entryMap();
}

void SubstitutionFilter::filterMessage(Message &message)
{
    QString msg = message.mainMessagePart();
    Q_FOREACH (const QString &src, d->wordList.keys()) {
        msg.replace(src, d->wordList[src]);
    }
    message.setMainMessagePart(msg);
}

K_PLUGIN_FACTORY(MessageFilterFactory, registerPlugin<SubstitutionFilter>();)
K_EXPORT_PLUGIN(MessageFilterFactory("ktptextui_message_filter_substitution"))
