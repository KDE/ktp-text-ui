/*
    Copyright (C) 2012  Lasath Fernando <kde@lasath.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "filters.h"

#include <QImageReader>

#include <KProtocolInfo>
#include <KDebug>

void UrlFilter::filterMessage(Message &info) {
    QString message = info.mainMessagePart();
    //FIXME: make "Urls" into a constant
    QStringList urls = info.property("Urls").toStringList();

    // link detection
    QRegExp link(QLatin1String("\\b(?:(\\w+)://|(www\\.))([^\\s]+)"));
    int fromIndex = 0;

    while ((fromIndex = message.indexOf(link, fromIndex)) != -1) {
        QString realUrl = link.cap(0);
        QString protocol = link.cap(1);

        //if cap(1) is empty cap(2) was matched -> starts with www.
        const bool startsWithWWW = link.cap(1).isEmpty();

        kDebug() << "Found URL " << realUrl << "with protocol : " << (startsWithWWW ? QLatin1String("http") : protocol);


        // if url has a supported protocol
        if (startsWithWWW || KProtocolInfo::protocols().contains(protocol, Qt::CaseInsensitive)) {

            // text not wanted in a link ( <,> )
            QRegExp unwanted(QLatin1String("(&lt;|&gt;)"));

            if (!realUrl.contains(unwanted)) {
                // string to show to user
                QString shownUrl = realUrl;

                // check for newline and cut link when found
                if (realUrl.contains(QLatin1String(("<br/>")))) {
                    int findIndex = realUrl.indexOf(QLatin1String("<br/>"));
                    realUrl.truncate(findIndex);
                    shownUrl.truncate(findIndex);
                }

                // check prefix
                if (startsWithWWW) {
                    realUrl.prepend(QLatin1String("http://"));
                }

                // if the url is changed, show in chat what the user typed in
                QString link = QLatin1String("<a href='") + realUrl + QLatin1String("'>") + shownUrl + QLatin1String("</a>");

                message.replace(fromIndex, shownUrl.length(), link);
                // advance position otherwise I end up parsing the same link
                fromIndex += link.length();
            } else {
                fromIndex += realUrl.length();
            }

            urls.append(realUrl);
        } else {
            fromIndex += link.matchedLength();
        }
    }

    info.setProperty("Urls", urls);
    info.setMainMessagePart(message);
}
