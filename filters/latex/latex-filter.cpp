/*
 *    Copyright (C) 2012  Lasath Fernando <kde@lasath.org>
 *
 *    Copyright (c) 2004 by Duncan Mac-Vicar Prett   <duncan@kde.org>
 *    Copyright (c) 2004-2005 by Olivier Goffart  <ogoffart@kde. org>
 *    Kopete    (c) 2001-2004 by the Kopete developers  <kopete-devel@kde.org>
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

#include "latex-filter.h"

#include <QTextDocument>
#include <QImage>
#include <QStringBuilder>

#include <KPluginFactory>
#include <KDebug>
#include <KStandardDirs>
#include <KTemporaryFile>
#include <KProcess>

class LatexFilter::Private
{
public:
    QString convScript;
    QList<KTemporaryFile *> tempFiles;
};

LatexFilter::LatexFilter(QObject *parent, const QVariantList &) :
    AbstractMessageFilter(parent), d(new Private)
{
    d->convScript = KStandardDirs::findExe(QLatin1String("kopete_latexconvert.sh"));
}

void LatexFilter::filterMessage(Message &message)
{
    QString mMagick = KStandardDirs::findExe(QLatin1String("convert"));
    if(mMagick.isEmpty()) {
        // don't try to parse if convert is not installed
        kError() << "Cannot find the Magick 'convert' program.\nconvert is required to render the LaTeX formulae.\nPlease get the software from www.imagemagick.org or from your distribution's package manager.";
        return;
    }
    QString messageText = message.mainMessagePart();
    if(!messageText.contains(QLatin1String("$$"))) {
        return;
    }

    QRegExp rg(QLatin1String("\\$\\$.+\\$\\$"));
    rg.setMinimal(true);

    QMap<QString, QString> replaceMap;
    int pos = 0;
    while(pos >= 0 && pos < messageText.length()) {
        pos = rg.indexIn(messageText, pos);

        if(pos >= 0) {
            const QString match = rg.cap(0);
            pos += rg.matchedLength();

            QString formul = match;
            // first remove the $$ delimiters on start and end
            formul.remove(QLatin1String("$$"));
            // then trim the result, so we can skip totally empty/whitespace-only formulas
            formul = formul.trimmed();
            if(formul.isEmpty() || !securityCheck(formul)) {
                continue;
            }

            const QString fileName = handleLatex(formul);

            // get the image and encode it with base64
            replaceMap[match] = fileName;
        }
    }

    if(replaceMap.isEmpty()) {
        //we haven't found any LaTeX strings
        return;
    }

    int imagePxWidth, imagePxHeight;
    for(QMap<QString, QString>::ConstIterator it = replaceMap.constBegin(); it != replaceMap.constEnd(); ++it) {
        QImage theImage(*it);
        if(theImage.isNull()) {
            continue;
        }

        imagePxWidth = theImage.width();
        imagePxHeight = theImage.height();
        QString escapedLATEX = Qt::escape(it.key()).replace(QLatin1Char('\"'), QLatin1String("&quot;"));     //we need  the escape quotes because that string will be in a title="" argument, but not the \n
        message.appendMessagePart(
            QLatin1String(" <img width=\"") %
            QString::number(imagePxWidth) %
            QLatin1String("\" height=\"") %
            QString::number(imagePxHeight) %
            QLatin1String("\" align=\"middle\" src=\"") %
            (*it) + QLatin1String("\"  alt=\"") %
            escapedLATEX %
            QLatin1String("\" title=\"") %
            escapedLATEX %
            QLatin1String("\"  /> ")
        );
    }
}

QString LatexFilter::handleLatex(const QString &latexFormula)
{
    KTemporaryFile *tempFile = new KTemporaryFile();
    tempFile->setPrefix(QLatin1String("kopetelatex-"));
    tempFile->setSuffix(QLatin1String(".png"));
    tempFile->open();
    d->tempFiles.append(tempFile);
    QString fileName = tempFile->fileName();

    KProcess p;

    QString argumentRes = QString(QLatin1String("-r %1x%2")).arg(150).arg(150);
    QString argumentOut = QString(QLatin1String("-o %1")).arg(fileName);
    QString argumentInclude(QLatin1String("-x %1"));


    p << d->convScript <<  argumentRes << argumentOut /*<< argumentFormat*/ << latexFormula;

    kDebug() << "Rendering" << d->convScript << argumentRes << argumentOut << argumentInclude << latexFormula ;

    // FIXME our sucky sync filter API limitations :-)
    p.execute();
    return fileName;
}

bool LatexFilter::securityCheck(const QString &latexFormula)
{
    return !latexFormula.contains(QRegExp(QLatin1String(
        "\\\\(def|let|futurelet|newcommand|renewcomment|else|fi|write|input|include"
        "|chardef|catcode|makeatletter|noexpand|toksdef|every|errhelp|errorstopmode|scrollmode|nonstopmode|batchmode"
        "|read|csname|newhelp|relax|afterground|afterassignment|expandafter|noexpand|special|command|loop|repeat|toks"
        "|output|line|mathcode|name|item|section|mbox|DeclareRobustCommand)[^a-zA-Z]"
    )));

}

LatexFilter::~LatexFilter()
{
    qDeleteAll(d->tempFiles);
    delete d;
}

K_PLUGIN_FACTORY(MessageFilterFactory, registerPlugin<LatexFilter>();)
K_EXPORT_PLUGIN(MessageFilterFactory("ktptextui_message_filter_latex"))
