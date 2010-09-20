/*
   kopetechatwindowstyle.cpp - A Chat Window Style.

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

#include "chatwindowstyle.h"
#include "chatstyleplistfilereader.h"

// Qt includes
#include <QFile>
#include <QDir>
#include <QHash>
#include <QStringList>
#include <QTextCodec>
#include <QTextStream>

// KDE includes
#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>

class ChatWindowStyle::Private
{
public:
    QString styleId;
    QString baseHref;
    StyleVariants variantsList;
    QString defaultVariantName;

    QString templateHtml;
    QString headerHtml;
    QString footerHtml;
    QString incomingHtml;
    QString nextIncomingHtml;
    QString outgoingHtml;
    QString nextOutgoingHtml;
    QString statusHtml;
    QString actionIncomingHtml;
    QString actionOutgoingHtml;
    QString fileTransferIncomingHtml;
    QString voiceClipIncomingHtml;
    QString outgoingStateSendingHtml;
    QString outgoingStateErrorHtml;
    QString outgoingStateSentHtml;
    QString outgoingStateUnknownHtml;

    QHash<QString, bool> compactVariants;
};

ChatWindowStyle::ChatWindowStyle(const QString &styleId, StyleBuildMode styleBuildMode)
	: QObject(), d(new Private)
{
    init(styleId, styleBuildMode);
}

ChatWindowStyle::ChatWindowStyle(const QString &styleId, const QString &variantPath, StyleBuildMode styleBuildMode)
	: QObject(), d(new Private)
{
    init(styleId, styleBuildMode);
}

void ChatWindowStyle::init(const QString &styleId, StyleBuildMode styleBuildMode)
{
    QStringList styleDirs = KGlobal::dirs()->findDirs("data", QString("ktelepathy/styles/%1/Contents/Resources/").arg(styleId));

    if (styleDirs.isEmpty()) {
	kDebug() << "Failed to find style" << styleId;
	return;
    }
    d->styleId = styleId;
    if (styleDirs.count() > 1)
        kDebug() << "found several styles with the same name. using first";
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
    bool statusHtml = !d->statusHtml.isEmpty();
    bool fileTransferIncomingHtml = !d->fileTransferIncomingHtml.isEmpty();
    bool nextIncomingHtml = !d->nextIncomingHtml.isEmpty();
    bool incomingHtml = !d->incomingHtml.isEmpty();
    bool nextOutgoingHtml = !d->nextOutgoingHtml.isEmpty();
    bool outgoingHtml = !d->outgoingHtml.isEmpty();

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

QString ChatWindowStyle::defaultVariantName()
{
    return d->defaultVariantName;
}

QString ChatWindowStyle::getStyleBaseHref() const
{
    return d->baseHref;
}


bool ChatWindowStyle::hasHeader() const
{
    return ! d->headerHtml.isEmpty();
}

QString ChatWindowStyle::getTemplateHtml() const
{
    return d->templateHtml;
}

QString ChatWindowStyle::getHeaderHtml() const
{
    return d->headerHtml;
}

QString ChatWindowStyle::getFooterHtml() const
{
    return d->footerHtml;
}

QString ChatWindowStyle::getIncomingHtml() const
{
    return d->incomingHtml;
}

QString ChatWindowStyle::getNextIncomingHtml() const
{
    return d->nextIncomingHtml;
}

QString ChatWindowStyle::getOutgoingHtml() const
{
    return d->outgoingHtml;
}

QString ChatWindowStyle::getNextOutgoingHtml() const
{
    return d->nextOutgoingHtml;
}

QString ChatWindowStyle::getStatusHtml() const
{
    return d->statusHtml;
}

QString ChatWindowStyle::getActionIncomingHtml() const
{
    return d->actionIncomingHtml;
}

QString ChatWindowStyle::getActionOutgoingHtml() const
{
    return d->actionOutgoingHtml;
}

QString ChatWindowStyle::getFileTransferIncomingHtml() const
{
    return d->fileTransferIncomingHtml;
}

QString ChatWindowStyle::getVoiceClipIncomingHtml() const
{
    return d->voiceClipIncomingHtml;
}

QString ChatWindowStyle::getOutgoingStateSendingHtml() const
{
    return d->outgoingStateSendingHtml;
}

QString ChatWindowStyle::getOutgoingStateSentHtml() const
{
    return d->outgoingStateSentHtml;
}

QString ChatWindowStyle::getOutgoingStateErrorHtml() const
{
    return d->outgoingStateErrorHtml;
}

QString ChatWindowStyle::getOutgoingStateUnknownHtml() const
{
    return d->outgoingStateUnknownHtml;
}

bool ChatWindowStyle::hasActionTemplate() const
{
    return (!d->actionIncomingHtml.isEmpty() && !d->actionOutgoingHtml.isEmpty());
}

void ChatWindowStyle::listVariants()
{
    QString variantDirPath = d->baseHref + QString::fromUtf8("Variants/");
    QDir variantDir(variantDirPath);

    QStringList variantList = variantDir.entryList(QStringList("*.css"));
    QStringList::ConstIterator it, itEnd = variantList.constEnd();
    QLatin1String compactVersionPrefix("_compact_");
    for (it = variantList.constBegin(); it != itEnd; ++it) {
        QString variantName = *it, variantPath;
        // Retrieve only the file name.
        variantName = variantName.left(variantName.lastIndexOf("."));
        if (variantName.startsWith(compactVersionPrefix)) {
            if (variantName == compactVersionPrefix) {
                d->compactVariants.insert("", true);
            }
            continue;
        }
        QString compactVersionFilename = *it;
        QString compactVersionPath = variantDirPath + compactVersionFilename.prepend(compactVersionPrefix);
        if (QFile::exists(compactVersionPath)) {
            d->compactVariants.insert(variantName, true);
        }
        // variantPath is relative to baseHref.
        variantPath = QString("Variants/%1").arg(*it);
        d->variantsList.insert(variantName, variantPath);
    }
}

void ChatWindowStyle::readStyleFiles()
{
    QString templateFile = d->baseHref + QString("Template.html");
    QString headerFile = d->baseHref + QString("Header.html");
    QString footerFile = d->baseHref + QString("Footer.html");
    QString incomingFile = d->baseHref + QString("Incoming/Content.html");
    QString nextIncomingFile = d->baseHref + QString("Incoming/NextContent.html");
    QString outgoingFile = d->baseHref + QString("Outgoing/Content.html");
    QString nextOutgoingFile = d->baseHref + QString("Outgoing/NextContent.html");
    QString statusFile = d->baseHref + QString("Status.html");
    QString actionIncomingFile = d->baseHref + QString("Incoming/Action.html");
    QString actionOutgoingFile = d->baseHref + QString("Outgoing/Action.html");
    QString fileTransferIncomingFile = d->baseHref + QString("Incoming/FileTransferRequest.html");
    QString voiceClipIncomingFile = d->baseHref + QString("Incoming/voiceClipRequest.html");
    QString outgoingStateUnknownFile = d->baseHref + QString("Outgoing/StateUnknown.html");
    QString outgoingStateSendingFile = d->baseHref + QString("Outgoing/StateSending.html");
    QString outgoingStateSentFile = d->baseHref + QString("Outgoing/StateSent.html");
    QString outgoingStateErrorFile = d->baseHref + QString("Outgoing/StateError.html");
    QString infoPlistFile = d->baseHref + QString("../Info.plist");


    QFile fileAccess;

    ChatStylePlistFileReader plistReader(infoPlistFile);
    d->defaultVariantName = plistReader.defaultVariant();

    //Load template file
    if (QFile::exists(templateFile)) {
        fileAccess.setFileName(templateFile);
        fileAccess.open(QIODevice::ReadOnly);
        QTextStream headerStream(&fileAccess);
        headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
        d->templateHtml = headerStream.readAll();
        fileAccess.close();
    }

    // Load header file.
    if (QFile::exists(headerFile)) {
        fileAccess.setFileName(headerFile);
        fileAccess.open(QIODevice::ReadOnly);
        QTextStream headerStream(&fileAccess);
        headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
        d->headerHtml = headerStream.readAll();
        fileAccess.close();
    }
    // Load Footer file
    if (QFile::exists(footerFile)) {
        fileAccess.setFileName(footerFile);
        fileAccess.open(QIODevice::ReadOnly);
        QTextStream headerStream(&fileAccess);
        headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
        d->footerHtml = headerStream.readAll();
        fileAccess.close();
    }
    // Load incoming file
    if (QFile::exists(incomingFile)) {
        fileAccess.setFileName(incomingFile);
        fileAccess.open(QIODevice::ReadOnly);
        QTextStream headerStream(&fileAccess);
        headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
        d->incomingHtml = headerStream.readAll();
        fileAccess.close();
    }
    // Load next Incoming file
    if (QFile::exists(nextIncomingFile)) {
        fileAccess.setFileName(nextIncomingFile);
        fileAccess.open(QIODevice::ReadOnly);
        QTextStream headerStream(&fileAccess);
        headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
        d->nextIncomingHtml = headerStream.readAll();
        fileAccess.close();
    }

    if (d->nextIncomingHtml.isEmpty()) {
        d->nextIncomingHtml = d->incomingHtml;
    }


    // Load outgoing file
    if (QFile::exists(outgoingFile)) {
        fileAccess.setFileName(outgoingFile);
        fileAccess.open(QIODevice::ReadOnly);
        QTextStream headerStream(&fileAccess);
        headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
        d->outgoingHtml = headerStream.readAll();
        fileAccess.close();
    }

    if (d->outgoingHtml.isEmpty()) {
        d->outgoingHtml = d->incomingHtml;
    }

    // Load next outgoing file
    if (QFile::exists(nextOutgoingFile)) {
        fileAccess.setFileName(nextOutgoingFile);
        fileAccess.open(QIODevice::ReadOnly);
        QTextStream headerStream(&fileAccess);
        headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
        d->nextOutgoingHtml = headerStream.readAll();
        fileAccess.close();
    }

    if (d->nextOutgoingHtml.isEmpty()) {
        d->nextOutgoingHtml = d->outgoingHtml;
    }

    // Load status file
    if (QFile::exists(statusFile)) {
        fileAccess.setFileName(statusFile);
        fileAccess.open(QIODevice::ReadOnly);
        QTextStream headerStream(&fileAccess);
        headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
        d->statusHtml = headerStream.readAll();
        fileAccess.close();
    }

    // Load Action Incoming file
    if (QFile::exists(actionIncomingFile)) {
        fileAccess.setFileName(actionIncomingFile);
        fileAccess.open(QIODevice::ReadOnly);
        QTextStream headerStream(&fileAccess);
        headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
        d->actionIncomingHtml = headerStream.readAll();
        fileAccess.close();
    }
    // Load Action Outgoing file
    if (QFile::exists(actionOutgoingFile)) {
        fileAccess.setFileName(actionOutgoingFile);
        fileAccess.open(QIODevice::ReadOnly);
        QTextStream headerStream(&fileAccess);
        headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
        d->actionOutgoingHtml = headerStream.readAll();
        fileAccess.close();
    }
    // Load FileTransfer Incoming file
    if (QFile::exists(fileTransferIncomingFile)) {
        fileAccess.setFileName(fileTransferIncomingFile);
        fileAccess.open(QIODevice::ReadOnly);
        QTextStream headerStream(&fileAccess);
        headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
        d->fileTransferIncomingHtml = headerStream.readAll();
        fileAccess.close();
    }

    if (d->fileTransferIncomingHtml.isEmpty() ||
            (!d->fileTransferIncomingHtml.contains("saveFileHandlerId") &&
             !d->fileTransferIncomingHtml.contains("saveFileAsHandlerId"))) {   // Create default html
        d->fileTransferIncomingHtml = d->incomingHtml;
        QString message = QString("%message%\n"
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
                                  "</div>")
                          .arg(i18n("Download"), i18n("Cancel"));
        d->fileTransferIncomingHtml.replace(QLatin1String("%message%"), message);
    }

    // Load VoiceClip Incoming file
    if (QFile::exists(voiceClipIncomingFile)) {
        fileAccess.setFileName(voiceClipIncomingFile);
        fileAccess.open(QIODevice::ReadOnly);
        QTextStream headerStream(&fileAccess);
        headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
        d->voiceClipIncomingHtml = headerStream.readAll();
        //kDebug() << "voiceClipIncoming HTML: " << d->voiceClipIncomingHtml;
        fileAccess.close();
    }

    if (d->voiceClipIncomingHtml.isEmpty() ||
            (!d->voiceClipIncomingHtml.contains("playVoiceHandlerId") &&
             !d->voiceClipIncomingHtml.contains("saveAsVoiceHandlerId"))) {   // Create default html
        d->voiceClipIncomingHtml = d->incomingHtml;
        QString message = QString("%message%\n"
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
                                  "</div>")
                          .arg(i18n("Play"), i18n("Save as"));
        d->voiceClipIncomingHtml.replace(QLatin1String("%message%"), message);
    }

    // Load outgoing file
    if (QFile::exists(outgoingStateUnknownFile)) {
        fileAccess.setFileName(outgoingStateUnknownFile);
        fileAccess.open(QIODevice::ReadOnly);
        QTextStream headerStream(&fileAccess);
        headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
        d->outgoingStateUnknownHtml = headerStream.readAll();
        //kDebug() << "Outgoing StateUnknown HTML: " << d->outgoingStateUnknownHtml;
        fileAccess.close();
    }

    if (QFile::exists(outgoingStateSendingFile)) {
        fileAccess.setFileName(outgoingStateSendingFile);
        fileAccess.open(QIODevice::ReadOnly);
        QTextStream headerStream(&fileAccess);
        headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
        d->outgoingStateSendingHtml = headerStream.readAll();
        //kDebug() << "Outgoing StateSending HTML: " << d->outgoingStateSendingHtml;
        fileAccess.close();
    }

    if (QFile::exists(outgoingStateSentFile)) {
        fileAccess.setFileName(outgoingStateSentFile);
        fileAccess.open(QIODevice::ReadOnly);
        QTextStream headerStream(&fileAccess);
        headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
        d->outgoingStateSentHtml = headerStream.readAll();
        //kDebug() << "Outgoing StateSent HTML: " << d->outgoingStateSentHtml;
        fileAccess.close();
    }

    if (QFile::exists(outgoingStateErrorFile)) {
        fileAccess.setFileName(outgoingStateErrorFile);
        fileAccess.open(QIODevice::ReadOnly);
        QTextStream headerStream(&fileAccess);
        headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
        d->outgoingStateErrorHtml = headerStream.readAll();
        //kDebug() << "Outgoing StateError HTML: " << d->outgoingStateErrorHtml;
        fileAccess.close();
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
        return compacted.insert(compacted.lastIndexOf('/') + 1, QString("_compact_"));
    }
}
