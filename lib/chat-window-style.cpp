/*
   kopetechat-window-style.cpp - A Chat Window Style.

   Copyright (c) 2005      by Michaël Larouche     <larouche@kde.org>

   Kopete    (c) 2002-2008 by the Kopete developers <kopete-devel@kde.org>

   *************************************************************************
   *                                                                       *
   * This program is free software; you can redistribute it and/or modify  *
   * it under the terms of the GNU General Public License as published by  *
   * the Free Software Foundation; either version 2 of the License, or     *
   * (at your option) any later version.                                   *
   *                                                                       *
   *************************************************************************
*/

#include "chat-window-style.h"
#include "chat-style-plist-file-reader.h"

// Qt includes
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QHash>
#include <QtCore/QStringList>
#include <QtCore/QTextCodec>
#include <QtCore/QTextStream>

// KDE includes
#include <KDebug>
#include <KLocale>
#include <KStandardDirs>

class ChatWindowStyle::Private
{
public:
    QString styleId;
    QString baseHref;
    StyleVariants variantsList;
    QString defaultVariantName;
    QString defaultFontFamily;
    int     defaultFontSize;
    bool    disableCombineConsecutive;

    QHash<int, QString> templateContents;
    QHash<QString, bool> compactVariants;
};

ChatWindowStyle::ChatWindowStyle(const QString &styleId, StyleBuildMode styleBuildMode)
    : QObject(), d(new Private)
{
    init(styleId, styleBuildMode);
}

ChatWindowStyle::ChatWindowStyle(const QString &styleId, const QString &variantPath,
                                 StyleBuildMode styleBuildMode)
    : QObject(), d(new Private)
{
    Q_UNUSED(variantPath);
    init(styleId, styleBuildMode);
}

void ChatWindowStyle::init(const QString &styleId, StyleBuildMode styleBuildMode)
{
    QStringList styleDirs = KGlobal::dirs()->findDirs("data",
        QString(QLatin1String("ktelepathy/styles/%1/Contents/Resources/")).arg(styleId)
    );

    if (styleDirs.isEmpty()) {
        kDebug() << "Failed to find style" << styleId;
        return;
    }
    d->styleId = styleId;
    if (styleDirs.count() > 1) {
        kDebug() << "found several styles with the same name. using first";
    }
    d->baseHref = styleDirs.at(0);
    kDebug() << "Using style:" << d->baseHref;
    readStyleFiles();
    if (styleBuildMode & StyleBuildNormal) {
        listVariants();
        if(d->defaultVariantName.isEmpty() && !d->variantsList.isEmpty()) {
            d->defaultVariantName = d->variantsList.keys().first();
        }
    }
}

ChatWindowStyle::~ChatWindowStyle()
{
    kDebug() ;
    delete d;
}

bool ChatWindowStyle::isValid() const
{
    kDebug();
    bool statusHtml = !content(Status).isEmpty();
    bool fileTransferIncomingHtml = !content(FileTransferIncoming).isEmpty();
    bool nextIncomingHtml = !content(IncomingNext).isEmpty();
    bool incomingHtml = !content(Incoming).isEmpty();
    bool nextOutgoingHtml = !content(OutgoingNext).isEmpty();
    bool outgoingHtml = !content(Outgoing).isEmpty();

    return (statusHtml && fileTransferIncomingHtml && nextIncomingHtml
            && incomingHtml && nextOutgoingHtml  && outgoingHtml);
}

ChatWindowStyle::StyleVariants ChatWindowStyle::getVariants()
{
    // If the variantList is empty, list available variants.
    if (d->variantsList.isEmpty()) {
        listVariants();
    }
    return d->variantsList;
}

QString ChatWindowStyle::id() const
{
    return d->styleId;
}

QString ChatWindowStyle::defaultVariantName() const
{
    return d->defaultVariantName;
}

QString ChatWindowStyle::defaultFontFamily() const
{
    return d->defaultFontFamily;
}

int ChatWindowStyle::defaultFontSize() const
{
    return d->defaultFontSize;
}

bool ChatWindowStyle::disableCombineConsecutive() const
{
    return d->disableCombineConsecutive;
}

QString ChatWindowStyle::getStyleBaseHref() const
{
    return d->baseHref;
}


bool ChatWindowStyle::hasHeader() const
{
    return ! content(Header).isEmpty();
}

QString ChatWindowStyle::getTemplateHtml() const
{
    return content(Template);
}

QString ChatWindowStyle::getHeaderHtml() const
{
    return content(Header);
}

QString ChatWindowStyle::getFooterHtml() const
{
    return content(Footer);
}

QString ChatWindowStyle::getTopicHtml() const
{
    return content(Topic);
}

QString ChatWindowStyle::getIncomingHtml() const
{
    return content(Incoming);
}

QString ChatWindowStyle::getNextIncomingHtml() const
{
    return content(IncomingNext);
}

QString ChatWindowStyle::getOutgoingHtml() const
{
    return content(Outgoing);
}

QString ChatWindowStyle::getNextOutgoingHtml() const
{
    return content(OutgoingNext);
}

QString ChatWindowStyle::getStatusHtml() const
{
    return content(Status);
}

QString ChatWindowStyle::getHistoryIncomingHtml() const
{
    return content(HistoryIncoming);
}

QString ChatWindowStyle::getHistoryNextIncomingHtml() const
{
    return content(HistoryIncomingNext);
}

QString ChatWindowStyle::getHistoryOutgoingHtml() const
{
    return content(HistoryOutgoing);
}

QString ChatWindowStyle::getHistoryNextOutgoingHtml() const
{
    return content(HistoryOutgoingNext);
}

QString ChatWindowStyle::getActionIncomingHtml() const
{
    return content(ActionIncoming);
}

QString ChatWindowStyle::getActionOutgoingHtml() const
{
    return content(ActionOutgoing);
}

QString ChatWindowStyle::getFileTransferIncomingHtml() const
{
    return content(FileTransferIncoming);
}

QString ChatWindowStyle::getVoiceClipIncomingHtml() const
{
    return content(VoiceClipIncoming);
}

QString ChatWindowStyle::getOutgoingStateSendingHtml() const
{
    return content(OutgoingStateSending);
}

QString ChatWindowStyle::getOutgoingStateSentHtml() const
{
    return content(OutgoingStateSent);
}

QString ChatWindowStyle::getOutgoingStateErrorHtml() const
{
    return content(OutgoingStateError);
}

QString ChatWindowStyle::getOutgoingStateUnknownHtml() const
{
    return content(OutgoingStateUnknown);
}

bool ChatWindowStyle::hasActionTemplate() const
{
    return (!content(ActionIncoming).isEmpty() && !content(ActionOutgoing).isEmpty());
}

void ChatWindowStyle::listVariants()
{
    QString variantDirPath = d->baseHref + QString::fromUtf8("Variants/");
    QDir variantDir(variantDirPath);

    QStringList variantList = variantDir.entryList(QStringList(QLatin1String("*.css")));
    QStringList::ConstIterator it, itEnd = variantList.constEnd();
    QLatin1String compactVersionPrefix("_compact_");
    for (it = variantList.constBegin(); it != itEnd; ++it) {
        QString variantName = *it, variantPath;
        // Retrieve only the file name.
        variantName = variantName.left(variantName.lastIndexOf(QLatin1String(".")));
        if (variantName.startsWith(compactVersionPrefix)) {
            if (variantName == compactVersionPrefix) {
                d->compactVariants.insert(QLatin1String(""), true);
            }
            continue;
        }
        QString compactVersionFilename = *it;
        QString compactVersionPath = variantDirPath + compactVersionFilename.prepend(compactVersionPrefix);
        if (QFile::exists(compactVersionPath)) {
            d->compactVariants.insert(variantName, true);
        }
        // variantPath is relative to baseHref.
        variantPath = QString(QLatin1String("Variants/%1")).arg(*it);
        d->variantsList.insert(variantName, variantPath);
    }
}

void ChatWindowStyle::setContent(InternalIdentifier id, const QString& content)
{
    d->templateContents.insert( id, content );
}

QString ChatWindowStyle::content(InternalIdentifier id) const
{
    return d->templateContents.value( id );
}

void ChatWindowStyle::inheritContent(InternalIdentifier subType, InternalIdentifier superType)
{
    if (content(subType).isEmpty()) {
        setContent(subType, content(superType));
    }
}


void ChatWindowStyle::readStyleFiles()
{
    // load style info
    QString infoPlistFile = d->baseHref + QLatin1String("../Info.plist");
    ChatStylePlistFileReader plistReader(infoPlistFile);
    d->defaultVariantName = plistReader.defaultVariant();
    d->defaultFontFamily  = plistReader.defaultFontFamily();
    d->defaultFontSize    = plistReader.defaultFontSize();
    d->disableCombineConsecutive = plistReader.disableCombineConsecutive();

    // specify the files for the identifiers
    QHash<InternalIdentifier, QLatin1String> templateFiles;
    templateFiles.insert(Template, QLatin1String("Template.html"));

    templateFiles.insert(Header, QLatin1String("Header.html"));
    templateFiles.insert(Content, QLatin1String("Content.html"));
    templateFiles.insert(Footer, QLatin1String("Footer.html"));
    templateFiles.insert(Topic, QLatin1String("Topic.html"));

    templateFiles.insert(Incoming, QLatin1String("Incoming/Content.html"));
    templateFiles.insert(IncomingNext, QLatin1String("Incoming/NextContent.html"));
    templateFiles.insert(Outgoing, QLatin1String("Outgoing/Content.html"));
    templateFiles.insert(OutgoingNext, QLatin1String("Outgoing/NextContent.html"));
    templateFiles.insert(Status, QLatin1String("Status.html"));

    templateFiles.insert(HistoryIncoming, QLatin1String("Incoming/Context.html"));
    templateFiles.insert(HistoryIncomingNext, QLatin1String("Incoming/NextContext.html"));
    templateFiles.insert(HistoryOutgoing, QLatin1String("Outgoing/Context.html"));
    templateFiles.insert(HistoryOutgoingNext, QLatin1String("Outgoing/NextContext.html"));

    templateFiles.insert(ActionIncoming, QLatin1String("Incoming/Action.html"));
    templateFiles.insert(ActionOutgoing, QLatin1String("Outgoing/Action.html"));

    templateFiles.insert(FileTransferIncoming, QLatin1String("FileTransferRequest.html"));
    templateFiles.insert(VoiceClipIncoming, QLatin1String("voiceClipRequest.html"));

    templateFiles.insert(OutgoingStateUnknown, QLatin1String("Outgoing/StateUnknown.html"));
    templateFiles.insert(OutgoingStateSending, QLatin1String("Outgoing/StateSending.html"));
    templateFiles.insert(OutgoingStateSent, QLatin1String("Outgoing/StateSent.html"));
    templateFiles.insert(OutgoingStateError, QLatin1String("Outgoing/StateError.html"));


    // load all files
    QFile fileAccess;
    Q_FOREACH(const QLatin1String &fileName, templateFiles) {
        QString path = d->baseHref + fileName;
        // Load template file
        if (QFile::exists(path)) {
            fileAccess.setFileName(path);
            fileAccess.open(QIODevice::ReadOnly);
            QTextStream headerStream(&fileAccess);
            headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
            QString data = headerStream.readAll();
            if(!data.isEmpty()) {
                //kDebug() << fileName << "was found!";
                setContent( templateFiles.key(fileName), data);
            } else {
                kDebug() << fileName << "was not found!";
            }
            //kDebug() << fileName << content(templateFiles.key(fileName));
            fileAccess.close();
        }
    }

    // basic fallbacks
    inheritContent(Topic, Header);

    inheritContent(Incoming, Content);
    inheritContent(Outgoing, Content);

    inheritContent(Outgoing, Incoming);
    inheritContent(Incoming, Outgoing);

    inheritContent(OutgoingNext, IncomingNext);
    inheritContent(IncomingNext, Incoming);
    inheritContent(OutgoingNext, Outgoing);

    inheritContent(HistoryOutgoing, HistoryIncoming);
    inheritContent(HistoryIncoming, Incoming);
    inheritContent(HistoryOutgoing, Outgoing);

    inheritContent(HistoryOutgoingNext, HistoryIncomingNext);
    inheritContent(HistoryIncomingNext, IncomingNext);
    inheritContent(HistoryOutgoingNext, OutgoingNext);

    inheritContent(Status, Content);

    // Load template file fallback
    if (content(Template).isEmpty())
    {
        QString templateFileName(KGlobal::dirs()->findResource("data", QLatin1String("ktelepathy/template.html")));

        if (! templateFileName.isEmpty() && QFile::exists(templateFileName)) {
            fileAccess.setFileName(templateFileName);
            fileAccess.open(QIODevice::ReadOnly);
            QTextStream headerStream(&fileAccess);
            headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
            setContent(Template, headerStream.readAll());
            fileAccess.close();
        }
    }

    //FIXME: do we have anything like this in telepathy?!

    // make sure file transfers are displayed correctly
    if (content(FileTransferIncoming).isEmpty() ||
            (!content(FileTransferIncoming).contains(QLatin1String("saveFileHandlerId")) &&
             !content(FileTransferIncoming).contains(QLatin1String("saveFileAsHandlerId")))) {   // Create default html
        QString message = QString(QLatin1String("%message%\n"
                                  "<div>\n"
                                  " <div style=\"width:37px; float:left;\">\n"
                                  "  <img src=\"%fileIconPath%\" style=\"width:32px; height:32px; vertical-align:middle;\" />\n"
                                  " </div>\n"
                                  " <div>\n"
                                  "  <span><b>%fileName%</b> (%fileSize%)</span><br>\n"
                                  "  <span>\n"
                                  "   <input id=\"%saveFileAsHandlerId%\" type=\"button\" value=\"%1\">\n"
                                  "   <input id=\"%cancelRequestHandlerId%\" type=\"button\" value=\"%2\">\n"
                                  "  </span>\n"
                                  " </div>\n"
                                  "</div>"))
                          .arg(i18n("Download"), i18n("Cancel"));
        QString incoming = content(Incoming);
        setContent(FileTransferIncoming, incoming.replace(QLatin1String("%message%"), message));
    }

    // make sure VoiceClip is displayed correctly
    if (content(VoiceClipIncoming).isEmpty() ||
            (!content(VoiceClipIncoming).contains(QLatin1String("playVoiceHandlerId")) &&
             !content(VoiceClipIncoming).contains(QLatin1String("saveAsVoiceHandlerId")))) {   // Create default html
        QString message = QString(QLatin1String("%message%\n"
                                  "<div>\n"
                                  " <div style=\"width:37px; float:left;\">\n"
                                  "  <img src=\"%fileIconPath%\" style=\"width:32px; height:32px; vertical-align:middle;\" />\n"
                                  " </div>\n"
                                  " <div>\n"
                                  "  <span>\n"
                                  "   <input id=\"%playVoiceHandlerId%\" type=\"button\" value=\"%1\">\n"
                                  "   <input id=\"%saveAsVoiceHandlerId%\" type=\"button\" value=\"%2\">\n"
                                  "  </span>\n"
                                  " </div>\n"
                                  "</div>"))
                          .arg(i18n("Play"), i18n("Save as"));
        setContent(VoiceClipIncoming, content(Incoming).replace(QLatin1String("%message%"), message));
    }
}

void ChatWindowStyle::reload()
{
    d->variantsList.clear();
    readStyleFiles();
    listVariants();
}

bool ChatWindowStyle::hasCompact(const QString & styleVariant) const
{
    if (d->compactVariants.contains(styleVariant)) {
        return d->compactVariants.value(styleVariant);
    }
    return false;
}

QString ChatWindowStyle::compact(const QString & styleVariant) const
{
    QString compacted = styleVariant;
    if (styleVariant.isEmpty()) {
        return QLatin1String("Variants/_compact_.css");
    } else {
        return compacted.insert(compacted.lastIndexOf(QLatin1Char('/')) + 1, QLatin1String("_compact_"));
    }
}
