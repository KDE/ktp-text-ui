/*
    Copyright (C) 2012  Lasath Fernando <kde@lasath.org>
    Copyright (C) 2012  David Edmundson <kde@davidedmundson.co.uk>
    Copyright (C) 2012  Rohan Garg      <rohangarg@kubuntu.org>

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

#include <KUrl>
#include <KProtocolInfo>
#include <KDebug>
#include <KTp/text-parser.h>

UrlFilter::UrlFilter(QObject *parent)
    : AbstractMessageFilter(parent)
{
}

void UrlFilter::filterMessage(Message &info) {
    QString message = info.mainMessagePart();
    //FIXME: make "Urls" into a constant
    QVariantList urls = info.property("Urls").toList();

    // link detection
    KTp::TextUrlData parsedUrl = KTp::TextParser::instance()->extractUrlData(message);

    int offset = 0;
    for (int i = 0; i < parsedUrl.fixedUrls.size(); i++) {
         KUrl url(parsedUrl.fixedUrls.at(i));
         QString originalText = message.mid(parsedUrl.urlRanges.at(i).first + offset, parsedUrl.urlRanges.at(i).second);
         QString link = QString::fromLatin1("<a href=\"%1\">%2</a>").arg(QString::fromAscii(url.toEncoded()), originalText);
         message.replace(parsedUrl.urlRanges.at(i).first + offset, parsedUrl.urlRanges.at(i).second, link);

         urls.append(url);

         //after the first replacement is made, the original position values are not valid anymore, this adjusts them
         offset += link.length() - originalText.length();
     }

    info.setProperty("Urls", urls);
    info.setMainMessagePart(message);
}


