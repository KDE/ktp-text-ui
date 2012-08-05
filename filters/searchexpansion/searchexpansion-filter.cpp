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

#include "searchexpansion-filter.h"

#include <KPluginFactory>
#include <KDebug>
#include <KUrl>
#include <KUriFilter>

class SearchexpansionFilter::Private
{
public:
};

SearchexpansionFilter::SearchexpansionFilter(QObject *parent, const QVariantList &) :
    AbstractMessageFilter(parent), d(new Private)
{
}

SearchexpansionFilter::~SearchexpansionFilter()
{
    delete d;
}

void SearchexpansionFilter::filterMessage(Message &message)
{
    KUriFilterData data(message.mainMessagePart());
    if(KUriFilter::self()->filterSearchUri(data, KUriFilter::WebShortcutFilter)) {

//         kDebug() << "Succesfully filtered" << data.typedString() << "to" << data.uri();
        message.setMainMessagePart(data.uri().url());
    }
}

K_PLUGIN_FACTORY(MessageFilterFactory, registerPlugin<SearchexpansionFilter>();)
K_EXPORT_PLUGIN(MessageFilterFactory("ktptextui_message_filter_searchexpansion"))
