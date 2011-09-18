//Stolen from Kopete.

/*
   kopetechat-window-style.h - A Chat Window Style.

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
#ifndef CHATWINDOWSTYLE_H
#define CHATWINDOWSTYLE_H

#include <QtCore/QHash>
#include <kdetelepathychat_export.h>

/**
* This class represent a single chat window style.
*
* @author Michaël Larouche <larouche@kde.org>
*/
class KDE_TELEPATHY_CHAT_EXPORT ChatWindowStyle : public QObject
{
public:
    /**
     * StyleVariants is a typedef to a QHash
     * key = Variant Name
     * value = Path to variant CSS file.
     * Path is relative to Resources directory.
     */
    typedef QHash<QString, QString> StyleVariants;

    /**
     * This enum specifies the mode of the constructor
     * - StyleBuildFast : Build the style the fatest possible
     * - StyleBuildNormal : List all variants of this style. Require a async dir list.
     */
    enum StyleBuildMode { StyleBuildFast, StyleBuildNormal};

    /**
     * @brief Build a single chat window style.
     *
     */
    explicit ChatWindowStyle(const QString &styleId, StyleBuildMode styleBuildMode = StyleBuildNormal);
    ChatWindowStyle(const QString &styleId, const QString &variantPath,
                    StyleBuildMode styleBuildMode = StyleBuildFast);
    virtual ~ChatWindowStyle();

    /**
     * Checks if the style is valid
     */
    bool isValid() const;

    /**
     * Get the list of all variants for this theme.
     * If the variant aren't listed, it call the lister
     * before returning the list of the Variants.
     * If the variant are listed, it just return the cached
     * variant list.
     * @return the StyleVariants QMap.
     */
    StyleVariants getVariants();

    /**
     * Get the style path.
     * The style path points to the directory where the style is located.
     * ex: ~/.kde/share/apps/kopete/styles/StyleName/
     *
     * @return the style path based.
     */
    QString id() const;

    /**
     * Get the style resource directory.
     * Resources directory is the base where all CSS, HTML and images are located.
     *
     * Adium(and now Kopete too) style directories are disposed like this:
     * StyleName/
     *          Contents/
     *            Resources/
     *
     * @return the path to the resource directory.
     */
    QString getStyleBaseHref() const;

    /** Returns true if this style supports showing a header at the top of the chat window*/

    bool hasHeader() const;


    QString getTemplateHtml() const;
    QString getHeaderHtml() const;
    QString getFooterHtml() const;
    QString getIncomingHtml() const;
    QString getNextIncomingHtml() const;
    QString getOutgoingHtml() const;
    QString getNextOutgoingHtml() const;
    QString getStatusHtml() const;

    QString getHistoryIncomingHtml() const;
    QString getHistoryNextIncomingHtml() const;
    QString getHistoryOutgoingHtml() const;
    QString getHistoryNextOutgoingHtml() const;

    QString getActionIncomingHtml() const;
    QString getActionOutgoingHtml() const;

    QString getFileTransferIncomingHtml() const;

    QString getVoiceClipIncomingHtml() const;

    QString getOutgoingStateSendingHtml() const;
    QString getOutgoingStateSentHtml() const;
    QString getOutgoingStateErrorHtml() const;
    QString getOutgoingStateUnknownHtml() const;

    /**
     * Check if the style has the support for Kopete Action template (Kopete extension)
     * @return true if the style has Action template.
     */
    bool hasActionTemplate() const;

    /**
     * Check if the supplied variant has a compact form
     */
    bool hasCompact(const QString & variant) const;

    /**
     * Return the compact version of the given style variant.
     * For the unmodified style, this returns "Variants/_compact_.css"
     */
    QString compact(const QString & variant) const;

    /**
     * Reload style from disk.
     */
    void reload();

    QString defaultVariantName();
    QString defaultFontFamily();
    int defaultFontSize();
private:
    /**
     * Read style HTML files from disk
     */
    void readStyleFiles();

    /**
     * Init this class
     */
    void init(const QString &styleName, StyleBuildMode styleBuildMode);

    /**
     * List available variants for the current style.
     */
    void listVariants();

    enum InternalIdentifier {
        Template,
        Status,

        Header,
        Footer,

        Incoming,
        IncomingNext,
        Outgoing,
        OutgoingNext,

        HistoryIncoming,
        HistoryIncomingNext,
        HistoryOutgoing,
        HistoryOutgoingNext,

        ActionIncoming,
        ActionOutgoing,

        FileTransferIncoming,
        VoiceClipIncoming,

        OutgoingStateUnknown,
        OutgoingStateSending,
        OutgoingStateSent,
        OutgoingStateError
        //InfoPlist
    };

    /**
     * Abbreviate d->templateContents.insert( internalIdentifier, content )
     */
    void setContent(InternalIdentifier id, const QString &content);

    /**
     * Abbreviate d->templateContents.value( internalIdentifier )
     */
    QString content(InternalIdentifier id) const;

    /**
     * If subType is empty, use the superType
     */
    void inheritContent(InternalIdentifier subType, InternalIdentifier superType);

private:
    class Private;
    Private * const d;
};

#endif
