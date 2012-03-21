/*
   kopetechat-window-style-manager.cpp - Manager all chat window styles

   Copyright (c) 2005      by Michaël Larouche     <larouche@kde.org>

   Kopete    (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

   *************************************************************************
   *                                                                       *
   * This program is free software; you can redistribute it and/or modify  *
   * it under the terms of the GNU General Public License as published by  *
   * the Free Software Foundation; either version 2 of the License, or     *
   * (at your option) any later version.                                   *
   *                                                                       *
   *************************************************************************
*/

#include "chat-window-style-manager.h"
#include "chat-style-plist-file-reader.h"
#include "chat-window-style.h"

// Qt includes
#include <QtCore/QStack>
#include <QtCore/QFileInfo>

// KDE includes
#include <KStandardDirs>
#include <KDirLister>
#include <KDebug>
#include <KUrl>
#include <KGlobal>
#include <KArchive>
#include <KZip>
#include <KTar>
#include <KMimeType>
#include <KIO/NetAccess>
#include <KSharedConfig>
#include <KConfigGroup>


class ChatWindowStyleManager::Private
{
public:
    Private(ChatWindowStyleManager *parent)
            : q(parent), styleDirLister(0) {}

    ~Private() {
        if (styleDirLister) {
            styleDirLister->deleteLater();
        }

        qDeleteAll(stylePool);
    }

    ChatWindowStyleManager *q;
    KDirLister *styleDirLister;
    QMap <QString, QString > availableStyles;

    // key = style id, value = ChatWindowStyle instance
    QHash<QString, ChatWindowStyle*> stylePool;

    QStack<KUrl> styleDirs;
};

ChatWindowStyleManager *ChatWindowStyleManager::self()
{
    static ChatWindowStyleManager s;
    return &s;
}

ChatWindowStyleManager::ChatWindowStyleManager(QObject *parent)
        : QObject(parent), d(new Private(this))
{
    kDebug() ;
}

ChatWindowStyleManager::~ChatWindowStyleManager()
{
    kDebug() ;
    delete d;
}

void ChatWindowStyleManager::loadStyles()
{
    // Make sure there exists a directory where chat styles can be installed to and it will be watched for changes
    KStandardDirs::locateLocal("data", QLatin1String("ktelepathy/styles/"));

    QStringList chatStyles = KGlobal::dirs()->findDirs("data", QLatin1String("ktelepathy/styles"));

    Q_FOREACH(const QString &styleDir, chatStyles) {
        kDebug() << styleDir;
        d->styleDirs.push(KUrl(styleDir));
    }

    d->styleDirLister = new KDirLister(this);
    d->styleDirLister->setDirOnlyMode(true);

    connect(d->styleDirLister, SIGNAL(newItems(KFileItemList)),
            this, SLOT(slotNewStyles(KFileItemList)));
    connect(d->styleDirLister, SIGNAL(completed()), this, SLOT(slotDirectoryFinished()));

    if (!d->styleDirs.isEmpty()) {
        d->styleDirLister->openUrl(d->styleDirs.pop(), KDirLister::Keep);
    }
}

QMap<QString, QString> ChatWindowStyleManager::getAvailableStyles() const
{
    return d->availableStyles;
}

int ChatWindowStyleManager::installStyle(const QString &styleBundlePath)
{
    QString localStyleDir;
    KStandardDirs::locateLocal("data", QLatin1String("ktelepathy/styles/"));
    QStringList chatStyles = KGlobal::dirs()->findDirs("data", QLatin1String("ktelepathy/styles"));
    // findDirs returns preferred paths first, let's check if one of them is writable
    Q_FOREACH(const QString& styleDir, chatStyles) {
        kDebug() << styleDir;
        if (QFileInfo(styleDir).isWritable()) {
            localStyleDir = styleDir;
            break;
        }
    }
    if (localStyleDir.isEmpty()) { // none of dirs is writable
        kDebug()<< "not writable";
        return StyleNoDirectoryValid;
    }

    KArchiveEntry *currentEntry = 0L;
    KArchiveDirectory *currentDir = 0L;
    KArchive *archive = 0L;

    QString currentBundleMimeType = KMimeType::findByPath(styleBundlePath, 0, false)->name();
    if (currentBundleMimeType == QLatin1String("application/zip")) {
        archive = new KZip(styleBundlePath);
    } else if (currentBundleMimeType == QLatin1String("application/x-compressed-tar") ||
               currentBundleMimeType == QLatin1String("application/x-bzip-compressed-tar") ||
               currentBundleMimeType == QLatin1String("application/x-gzip") ||
               currentBundleMimeType == QLatin1String("application/x-bzip")) {
        archive = new KTar(styleBundlePath);
    } else if (currentBundleMimeType == QLatin1String("application/octet-stream")) {
        archive = new KZip(styleBundlePath);
        if (!archive->open(QIODevice::ReadOnly)) {
            delete archive;
            kDebug() << "!zip";
            archive = new KTar(styleBundlePath);
            if (!archive->open(QIODevice::ReadOnly)) {
                delete archive;
                kDebug() << "!tar" << styleBundlePath;
                return StyleCannotOpen;
            }
        }
    } else {
        kDebug() << "unsupported file type" << currentBundleMimeType;
        kDebug() << styleBundlePath;
        return StyleUnknow;
    }

    if (archive == 0 ||  !archive->open(QIODevice::ReadOnly)) {
        delete archive;
        kDebug() << "cannot open theme file";
        return StyleCannotOpen;
    }

    const KArchiveDirectory *rootDir = archive->directory();

    // Ok where we go to check if the archive is valid.
    // Each time we found a correspondance to a theme bundle, we add a point to validResult.
    // A valid style bundle must have:
    // -a Contents, Contents/Resources, Co/Res/Incoming, Co/Res/Outgoing dirs
    // main.css, Footer.html, Header.html, Status.html files in Contents/Resources.
    // So for a style bundle to be valid, it must have a result greather than 8, because we test for 8 required entry.
    int validResult = 0;
    const QStringList entries = rootDir->entries();
    // Will be reused later.
    QStringList::ConstIterator entriesIt;
    for (entriesIt = entries.begin(); entriesIt != entries.end(); ++entriesIt) {
        currentEntry = const_cast<KArchiveEntry*>(rootDir->entry(*entriesIt));
    kDebug() << "Current entry name: " << currentEntry->name();
        if (currentEntry->isDirectory()) {
            currentDir = dynamic_cast<KArchiveDirectory*>(currentEntry);
            if (currentDir) {
                if (currentDir->entry(QLatin1String("Contents"))) {
                   kDebug() << "Contents found";
                   validResult += 1;
                }
                if (currentDir->entry(QLatin1String("Contents/Resources"))) {
                    kDebug() << "Contents/Resources found";
                    validResult += 1;
                }
            }
        }
    }
    kDebug() << "Valid result: " << QString::number(validResult);
    // The archive is a valid style bundle.
    if (validResult >= 2) {
        bool installOk = false;
        for (entriesIt = entries.begin(); entriesIt != entries.end(); ++entriesIt) {
            currentEntry = const_cast<KArchiveEntry*>(rootDir->entry(*entriesIt));
            if (currentEntry && currentEntry->isDirectory()) {
                // Ignore this MacOS X "garbage" directory in zip.
                if (currentEntry->name() == QLatin1String("__MACOSX")) {
                    continue;
                } else {
                    currentDir = dynamic_cast<KArchiveDirectory*>(currentEntry);
                    if (currentDir) {
                        currentDir->copyTo(localStyleDir + currentDir->name());
                        installOk = true;
                    }
                }
            }
        }

        archive->close();
        delete archive;

        if (installOk) {
            return StyleInstallOk;
        } else {
            return StyleUnknow;
        }
    } else {
        archive->close();
        delete archive;

        kDebug() << "style not valid";
        return StyleNotValid;
    }

    if (archive) {
        archive->close();
        delete archive;
    }

    return StyleUnknow;
}

bool ChatWindowStyleManager::removeStyle(const QString &styleId)
{
    Q_UNUSED(styleId)
//    kDebug() << styleId;
//    // Find for the current style in avaiableStyles map.
//    int foundStyleIdx = d->availableStyles.indexOf(styleId);

//    if (foundStyleIdx != -1) {
//        d->availableStyles.removeAt(foundStyleIdx);

//        // Remove and delete style from pool if needed.
//        if (d->stylePool.contains(styleId)) {
//            ChatWindowStyle *deletedStyle = d->stylePool[styleId];
//            d->stylePool.remove(styleId);
//            delete deletedStyle;
//        }

//        QStringList styleDirs = KGlobal::dirs()->findDirs("appdata", QString("styles/%1").arg(styleId));
//        if (styleDirs.isEmpty()) {
//            kDebug() << "Failed to find style" << styleId;
//            return false;
//        }

//        // attempt to delete all dirs with this style
//        int numDeleted = 0;
//        Q_FOREACH(const QString& stylePath, styleDirs) {
//            KUrl urlStyle(stylePath);
//            // Do the actual deletion of the directory style.
//            if (KIO::NetAccess::del(urlStyle, 0))
//                numDeleted++;
//        }
//        return numDeleted == styleDirs.count();
//    } else {
//        return false;
//    }
    return false;
}

ChatWindowStyle *ChatWindowStyleManager::getValidStyleFromPool(const QString &styleId)
{
    ChatWindowStyle *style = 0;
    style = getStyleFromPool(styleId);
    if (style) {
        return style;
    }

    kDebug() << "Trying default style";
    // Try default style
    style = getStyleFromPool(QLatin1String("renkoo.AdiumMessageStyle"));
    if (style) {
        return style;
    }

    kDebug() << "Trying first valid style";
    // Try first valid style
    Q_FOREACH(const QString& name, d->availableStyles) {
        style = getStyleFromPool(name);
        if (style) {
            return style;
        }
    }

    kDebug() << "Valid style not found!";
    return 0;
}

ChatWindowStyle *ChatWindowStyleManager::getStyleFromPool(const QString &styleId)
{
    if (d->stylePool.contains(styleId)) {
        kDebug() << styleId << " was on the pool";

        // NOTE: This is a hidden config switch for style developers
        // Check in the config if the cache is disabled.
        // if the cache is disabled, reload the style every time it's getted.
        KConfigGroup config(KGlobal::config(), "KopeteStyleDebug");
        bool disableCache = config.readEntry("disableStyleCache", false);
        if (disableCache) {
            d->stylePool[styleId]->reload();
        }

        return d->stylePool[styleId];
    }

    // Build a chat window style and list its variants, then add it to the pool.
    ChatWindowStyle *style = new ChatWindowStyle(styleId, ChatWindowStyle::StyleBuildNormal);
    if (!style->isValid()) {
        kDebug() << styleId << " is invalid style!";
        delete style;
        return 0;
    }

    d->stylePool.insert(styleId, style);
    kDebug() << styleId << " is just created";

    return style;
}

void ChatWindowStyleManager::slotNewStyles(const KFileItemList &dirList)
{
    Q_FOREACH(const KFileItem &item, dirList) {
        // Ignore data dir(from deprecated XSLT themes)
        if (!item.url().fileName().contains(QLatin1String("data"))) {
            kDebug() << "Listing: " << item.url().fileName();
            // If the style path is already in the pool, that's mean the style was updated on disk
            // Reload the style
            QString styleId = item.url().fileName();
            if (d->stylePool.contains(styleId)) {
                kDebug() << "Updating style: " << styleId;

                d->stylePool[styleId]->reload();

                // Add to available if required.
                if (!d->availableStyles.contains(styleId)) {

                    //FIXME this code is in two places.. this sucks!!!
                    ChatStylePlistFileReader plistReader(item.url().path().append(QLatin1String("/Contents/Info.plist")));
                    QString styleName = plistReader.CFBundleName();
                    if (plistReader.CFBundleName().isEmpty()) {
                        styleName = styleId;
                    }
                    d->availableStyles.insert(styleId, styleName);
                }
            } else {
                ChatStylePlistFileReader plistReader(item.url().path().append(QLatin1String("/Contents/Info.plist")));
                QString styleName = plistReader.CFBundleName();
                if (plistReader.CFBundleName().isEmpty()) {
                    styleName = styleId;
                }
                d->availableStyles.insert(styleId, styleName);
            }
        }
    }
}

void ChatWindowStyleManager::slotDirectoryFinished()
{
    // Start another scanning if the directories stack is not empty
    if (!d->styleDirs.isEmpty()) {
        kDebug() << "Starting another directory.";
        d->styleDirLister->openUrl(d->styleDirs.pop(), KDirLister::Keep);
    } else {
        Q_EMIT loadStylesFinished();
    }
}

#include "chat-window-style-manager.moc"
