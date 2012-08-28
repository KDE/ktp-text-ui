/*
 *    Copyright (C) 2012  Lasath Fernando <kde@lasath.org>
 *    Copyright (C) 2012  Daniele E. Domenichelli <daniele.domenichelli@gmail.com>
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

    void addTag (const char *markingCharacter, char htmlTag);
    QString filterString(QString string);
};

FormatFilter::FormatFilter (QObject* parent, const QVariantList&) :
    AbstractMessageFilter (parent), d(new Private())
{
    d->addTag("_", 'u');
    d->addTag("\\*", 'b');
    d->addTag("-", 's');
    d->addTag("/", 'i');
}

FormatFilter::~FormatFilter()
{
    delete d;
}

void FormatFilter::filterMessage (Message& message)
{
    message.setMainMessagePart(d->filterString(message.mainMessagePart()));

}

QString FormatFilter::Private::filterString(QString string)
{
    Q_FOREACH(FormatTag tag, tags) {
        if (string.indexOf(tag.first) >= 0) {
            string = string.replace(tag.first.cap(2), filterString(tag.first.cap(2)));
            string = string.replace(tag.first, tag.second);
        }
    }

    return string;
}

void FormatFilter::Private::addTag (const char *markingCharacter, char htmlTag)
{
    QString pattern = QLatin1String("(^|\\s)%1(\\S|\\S.*\\S)%1(\\s|$)");
    pattern = pattern.arg(QLatin1String(markingCharacter));

    QString repl = QLatin1String("\\1<%1>%2\\2%2</%1>\\3");
    repl = repl.arg(htmlTag).arg(QLatin1String(markingCharacter));

    QRegExp exp = QRegExp(pattern);
    exp.setMinimal(true);

    tags.append(FormatTag(exp, repl));
}

K_PLUGIN_FACTORY(MessageFilterFactory, registerPlugin<FormatFilter>();)
K_EXPORT_PLUGIN(MessageFilterFactory("ktptextui_message_filter_formatting"))
