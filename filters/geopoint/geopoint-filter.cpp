/*
 *    Copyright (C) 2016 Alexandr Akulich <akulichalexander@gmail.com>
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

#include "geopoint-filter.h"

#include <KPluginFactory>

#include <QRegularExpression>
#include <QRegularExpressionMatch>

GeopointFilter::GeopointFilter(QObject *parent, const QVariantList &args) :
    AbstractMessageFilter(parent)
{
    Q_UNUSED(args)
}

void GeopointFilter::filterMessage(KTp::Message &message, const KTp::MessageContext&)
{
    static const QString linkTemplate = QStringLiteral("<a href=\"%1\">%1</a>");
    static const QString mapTemplate = QStringLiteral(
                "<iframe width=\"420\" height=\"350\" scrolling=\"no\" marginheight=\"0\" marginwidth=\"0\" src=\""
                "https://www.openlinkmap.org/small.php?lat=%1&lon=%2&zoom=%3\" "
                "style=\"border: 1px solid black\" "
                "frameborder=\"0\"></iframe>"
    );

    static const QRegularExpression geoTextExpression(QStringLiteral(
                                                          "geo:"
                                                          "(?<latitude>[\\+\\-]?\\d{1,3}(\\.[0-9]+)?),"
                                                          "(?<longitude>[\\+\\-]?\\d{1,3}(\\.[0-9]+)?)"
                                                          "(\\?z=(?<zoom>\\d+))?"));
    QRegularExpressionMatch match;
    QString messageText = message.mainMessagePart();
    int index = 0;

    while ((index = messageText.indexOf(geoTextExpression, index, &match)) >= 0) {
        const double latitude = match.capturedRef(QStringLiteral("latitude")).toDouble();
        const double longitude = match.capturedRef(QStringLiteral("longitude")).toDouble();

        const QStringRef zoomStr = match.capturedRef(QStringLiteral("zoom"));

        int zoom = 12;
        if (!zoomStr.isNull()) {
            zoom = zoomStr.toInt();
        }

        message.appendMessagePart(QStringLiteral("<br/>") + mapTemplate.arg(latitude).arg(longitude).arg(zoom));
        messageText.replace(index, match.capturedLength(), linkTemplate.arg(match.captured()));

        index = match.capturedEnd(QStringLiteral("longitude"));
        // Shift the index by the length of added text: we added the second copy of "geo:..." text,
        // plus the html code itself and minus the length of argument placeholders (%1 and %1).
        index += match.capturedLength() + linkTemplate.length() - 4;
    }

    message.setMainMessagePart(messageText);
}

K_PLUGIN_FACTORY(MessageFilterFactory, registerPlugin<GeopointFilter>();)

#include "geopoint-filter.moc"
