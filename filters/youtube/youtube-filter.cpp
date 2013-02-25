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

#include "youtube-filter.h"

#include <KPluginFactory>
#include <KDebug>
#include <KUrl>
#include <KUriFilter>

YoutubeFilter::YoutubeFilter(QObject *parent, const QVariantList &) :
    AbstractMessageFilter(parent)
{
}

YoutubeFilter::~YoutubeFilter()
{
}

void YoutubeFilter::filterMessage(KTp::Message &message, const KTp::MessageContext&)
{
    static const QString html = QLatin1String(
        "<br />\n"
        "<iframe class=\"youtube-player\" "
            "type=\"text/html\""
            "style=\"max-width:100%;max-height:100%\""
            "src=\"http://www.youtube.com/embed/%1\" "
            "frameborder=\"0\">"
        "</iframe>"
    );

    static const QRegExp validId(QLatin1String("[a-zA-Z0-9_-]+"));

    Q_FOREACH (QVariant var, message.property("Urls").toList()) {
        KUrl url = qvariant_cast<KUrl>(var);
        if (url.host() == QLatin1String("www.youtube.com") ||
                url.host() == QLatin1String("youtube.com")) {
            kDebug() << "found youtube url :" << url.url();

            QString v = url.queryItemValue(QLatin1String("v"));
            kDebug() << "v =" << v;

            if (v.contains(validId)){
                message.appendMessagePart(html.arg(url.queryItemValue(QLatin1String("v"))));
            }
        }
    }
}

K_PLUGIN_FACTORY(MessageFilterFactory, registerPlugin<YoutubeFilter>();)
K_EXPORT_PLUGIN(MessageFilterFactory("ktptextui_message_filter_youtube"))
