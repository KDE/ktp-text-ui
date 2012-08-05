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

#include "format-filter.h"

#include <QRegExp>

#include <KPluginFactory>
#include <KDebug>

typedef QPair<QRegExp, QString> FormatTag;

class FormatFilter::Private {
public:
    QList<FormatTag> tags;
};

FormatFilter::FormatFilter (QObject* parent, const QVariantList&) :
    AbstractMessageFilter (parent), d(new Private())
{
    addTag("_", 'i');
    addTag("\\*", 'b');
    addTag("-", 's');
}

FormatFilter::~FormatFilter()
{
    delete d;
}

void FormatFilter::filterMessage (Message& message)
{
    Q_FOREACH(FormatTag tag, d->tags) {
        QString msg = message.mainMessagePart();

        msg = msg.replace(tag.first, tag.second);
        message.setMainMessagePart(msg);
    }
}


void FormatFilter::addTag (const char *markingCharacter, char htmlTag)
{
    QString pattern = QLatin1String("%1(\\S.*\\S)%1");
    pattern = pattern.arg(QLatin1String(markingCharacter));

    QString repl = QLatin1String("<%1>\\1</%1>");
    repl = repl.arg(htmlTag);

    QRegExp exp = QRegExp(pattern);
    exp.setMinimal(true);

    d->tags.append(FormatTag(exp, repl));

    QString singleCharPattern = QLatin1String("%1(\\S)%1");
    singleCharPattern = singleCharPattern.arg(QLatin1String(markingCharacter));

    QRegExp singleCharExp = QRegExp(singleCharPattern);
    singleCharExp.setMinimal(true);

    d->tags.append(FormatTag(singleCharExp, repl));
}

K_PLUGIN_FACTORY(MessageFilterFactory, registerPlugin<FormatFilter>();)
K_EXPORT_PLUGIN(MessageFilterFactory("ktptextui_message_filter_formatting"))