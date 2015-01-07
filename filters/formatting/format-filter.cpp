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

typedef QPair<QRegExp, QString> FormatTag;

class FormatFilter::Private {
public:
    QList<FormatTag> tags;
    QString mainPattern;
    QString allTagsPattern;

    void addTag (const QString &markingCharacter, const QString &htmlTag);
    QString filterString(QString string);
};

FormatFilter::FormatFilter (QObject* parent, const QVariantList&) :
    KTp::AbstractMessageFilter (parent),
    d(new Private())
{
    // Matches a string
    // 1. The beginning of the string or a white character [ (^|\\s) ]
    // 2. Depending on the regexp the tag to be replaced or a pattern including
    //    all the tags in tagsMap [ %1 ]
    // 3. One non-whitespace character, or by any string that starts and ends
    //    with a non-whitespace character [ (\\S|\\S.*\\S) ]
    // 4. Same as 2. [ %1 ]
    // 5. A white character or the end of the string [ (\\s|$) ]
    d->mainPattern = QLatin1String("(^|\\s)%1(\\S|\\S.*\\S)%1(\\s|$)");

    QMap<QString, QString> tagsMap;

    tagsMap[QLatin1String("_")] = QLatin1Char('u');
    tagsMap[QLatin1String("*")] = QLatin1Char('b');
    tagsMap[QLatin1String("-")] = QLatin1Char('s');
    tagsMap[QLatin1String("/")] = QLatin1Char('i');

    d->allTagsPattern = QLatin1Char('(');

    QMapIterator<QString, QString> i(tagsMap);
    while (i.hasNext()) {
        i.next();

        d->allTagsPattern += QRegExp::escape(i.key());
        if (i.hasNext())
            d->allTagsPattern += QLatin1Char('|');

        d->addTag(i.key(), i.value());
    }

    d->allTagsPattern += QLatin1Char(')');
}

FormatFilter::~FormatFilter()
{
    delete d;
}

void FormatFilter::filterMessage (KTp::Message& message, const KTp::MessageContext &context)
{
    Q_UNUSED(context)
    message.setMainMessagePart(d->filterString(message.mainMessagePart()));
}

QString FormatFilter::Private::filterString(QString string)
{
    QRegExp rx(mainPattern.arg(allTagsPattern));
    rx.setMinimal(true);

    int pos = 0;
    while ((pos = string.indexOf(rx, pos)) != -1) {
        string = string.replace(rx.cap(3), filterString(rx.cap(3)));
        pos += rx.matchedLength();
    }

    Q_FOREACH(const FormatTag &tag, tags) {
        string = string.replace(tag.first, tag.second);
    }

    return string;
}

void FormatFilter::Private::addTag (const QString &markingCharacter, const QString &htmlTag)
{
    QString repl = QLatin1String("\\1<%1>%2\\2%2</%1>\\3");
    repl = repl.arg(htmlTag).arg(markingCharacter);

    QRegExp exp = QRegExp(mainPattern.arg(QRegExp::escape(markingCharacter)));
    exp.setMinimal(true);

    tags.append(FormatTag(exp, repl));
}

K_PLUGIN_FACTORY(MessageFilterFactory, registerPlugin<FormatFilter>();)

#include "format-filter.moc"
