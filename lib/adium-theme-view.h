/***************************************************************************
 *   Copyright (C) 2010 by David Edmundson <kde@davidedmundson.co.uk>      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#ifndef ADIUMTHEMEVIEW_H
#define ADIUMTHEMEVIEW_H

#include <QtWebKit/QWebView>

#include "adium-theme-header-info.h"
#include "adium-theme-content-info.h"

#include <KEmoticons>

#include "ktpchat_export.h"

class AdiumThemeContentInfo;
class AdiumThemeHeaderInfo;
class AdiumThemeStatusInfo;
class ChatWindowStyle;

class QContextMenuEvent;

class KAction;

class KDE_TELEPATHY_CHAT_EXPORT AdiumThemeView : public QWebView
{
    Q_OBJECT
public:

    enum ChatType {
        GroupChat,
        SingleUserChat
    };

    explicit AdiumThemeView(QWidget *parent = 0);

    /** Loads the Theme data*/
    void load(ChatType chatType);

    /** Starts populating the HTML into the view*/
    void initialise(const AdiumThemeHeaderInfo&);

    const QString variantPath() const;
    const QString variantName() const;
    void setVariant(const QString& variant);
    ChatWindowStyle *chatStyle() const;

    /** Set the theme to use. Display will only change once initialise() is called.*/
    void setChatStyle(ChatWindowStyle* chatStyle);
    void setUseCustomFont(bool);
    QString fontFamily();
    int fontSize();
    void setFontFamily(QString fontFamily);
    void setFontSize(int fontSize);
    bool isCustomFont() const;
    bool isHeaderDisplayed() const;
    /** Set whether a header is displayed at the top of the window.
     * Output will only change once initialise() is called. */
    void setHeaderDisplayed(bool);
    /* .. font, backgrounds, everything else.*/

public Q_SLOTS:
    void addContentMessage(const AdiumThemeContentInfo&);
    void addStatusMessage(const AdiumThemeStatusInfo&);
    void onLinkClicked(const QUrl &);
    void onOpenLinkActionTriggered();

protected:
    virtual void contextMenuEvent(QContextMenuEvent *event);

private:
    ChatWindowStyle *m_chatStyle;
    QString m_variantPath;
    QString m_variantName;
    KEmoticons m_emoticons;
    bool m_useCustomFont;
    QString m_fontFamily;
    int m_fontSize;
    QString replaceHeaderKeywords(QString htmlTemplate, const AdiumThemeHeaderInfo&);
    QString replaceContentKeywords(QString& htmlTemplate, const AdiumThemeContentInfo&);
    QString replaceStatusKeywords(QString& htmlTemplate, const AdiumThemeStatusInfo&);
    QString replaceMessageKeywords(QString& htmlTemplate, const AdiumThemeMessageInfo&);

    QString formatTime(const QString&, const QDateTime&);

    QString m_defaultAvatar;
    AdiumThemeContentInfo m_lastContent;
    bool m_displayHeader;
    KAction *m_openLinkAction;

    void appendNewMessage(QString&);
    void appendNextMessage(QString&);

    bool m_webInspector;
};

#endif // ADIUMTHEMEVIEW_H


