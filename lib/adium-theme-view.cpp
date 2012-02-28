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

#include "adium-theme-view.h"

#include "adium-theme-header-info.h"
#include "adium-theme-content-info.h"
#include "adium-theme-message-info.h"
#include "adium-theme-status-info.h"
#include "chat-window-style-manager.h"
#include "chat-window-style.h"

#include <QtCore/QFile>
#include <QtCore/QTextCodec>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QFontDatabase>
#include <QtGui/QMenu>
#include <QtWebKit/QWebFrame>
#include <QtWebKit/QWebElement>
#include <QtWebKit/QWebInspector>
#include <QtWebKit/QWebSettings>

#include <KAction>
#include <KDebug>
#include <KEmoticonsTheme>
#include <KGlobal>
#include <KStandardDirs>
#include <KConfig>
#include <KConfigGroup>
#include <KMessageBox>
#include <KToolInvocation>
#include <KIconLoader>
#include <KProtocolInfo>

AdiumThemeView::AdiumThemeView(QWidget *parent)
        : QWebView(parent),
        // check iconPath docs for minus sign in -KIconLoader::SizeLarge
        m_defaultAvatar(KIconLoader::global()->iconPath(QLatin1String("im-user"),-KIconLoader::SizeLarge)),
        m_displayHeader(true)
{
    //blocks QWebView functionality which allows you to change page by dragging a URL onto it.
    setAcceptDrops(false);

    // don't let QWebView handle the links, we do
    page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

    QAction *defaultOpenLinkAction = pageAction(QWebPage::OpenLink);
    m_openLinkAction = new KAction(defaultOpenLinkAction->text(), this);
    connect(m_openLinkAction, SIGNAL(triggered()),
            this, SLOT(onOpenLinkActionTriggered()));

    connect(this, SIGNAL(linkClicked(QUrl)), this, SLOT(onLinkClicked(QUrl)));
}

void AdiumThemeView::load(ChatType chatType) {

    //determine the chat window style to use
    KSharedConfigPtr config = KSharedConfig::openConfig(QLatin1String("ktelepathyrc"));
    KConfigGroup appearanceConfig = config->group("Appearance");

    QString chatStyleName;
    if (chatType == AdiumThemeView::SingleUserChat) {
        chatStyleName = appearanceConfig.readEntry("styleName", "renkoo.AdiumMessageStyle");
    } else {
        chatStyleName = QLatin1String("simkete");
    }

    m_chatStyle = ChatWindowStyleManager::self()->getValidStyleFromPool(chatStyleName);
    if (m_chatStyle == 0 || !m_chatStyle->isValid()) {
        KMessageBox::error(this, i18n("Failed to load a valid theme. Your installation is broken. Check your kde path. "
                                      "Will now crash."));
    }

    QString variant = appearanceConfig.readEntry("styleVariant");
    if (!variant.isEmpty()) {
        m_variantPath = QString(QLatin1String("Variants/%1.css")).arg(variant);
        m_variantName = variant;

        // keep m_variantPath, m_variantName empty if there is no variant
    } else if (!m_chatStyle->getVariants().isEmpty()) {
        if (m_chatStyle->getVariants().contains(m_chatStyle->defaultVariantName())) {
            m_variantPath = QString(QLatin1String("Variants/%1.css")).arg(m_chatStyle->defaultVariantName());
            m_variantName = m_chatStyle->defaultVariantName();
        } else {
            m_variantPath = QString(QLatin1String("Variants/%1.css")).arg(m_chatStyle->getVariants().keys().first());
            m_variantName = m_chatStyle->getVariants().keys().first();
        }
    }

    m_displayHeader = appearanceConfig.readEntry("displayHeader", false);

    //special HTML debug mode. Debugging/Profiling only (or theme creating) should have no visible way to turn this flag on.
    m_webInspector = appearanceConfig.readEntry("debug", false);

    m_useCustomFont = appearanceConfig.readEntry("useCustomFont", false);
    m_fontFamily = appearanceConfig.readEntry("fontFamily", QWebSettings::globalSettings()->fontFamily(QWebSettings::StandardFont));
    m_fontSize = appearanceConfig.readEntry("fontSize", QWebSettings::globalSettings()->fontSize(QWebSettings::DefaultFontSize));
}


void AdiumThemeView::contextMenuEvent(QContextMenuEvent *event)
{
    QWebHitTestResult r = page()->mainFrame()->hitTestContent(event->pos());
    QUrl url = r.linkUrl();
    if (!url.isEmpty()) {
        // save current link url in openLinkAction
        m_openLinkAction->setData(url);

        QMenu menu(this);
        menu.addAction(m_openLinkAction);
        menu.addAction(pageAction(QWebPage::CopyLinkToClipboard));
        menu.exec(mapToGlobal(event->pos()));
    } else {
        QWebView::contextMenuEvent(event);
    }
}

void AdiumThemeView::initialise(const AdiumThemeHeaderInfo &chatInfo)
{
    QString headerHtml;
    QString templateHtml = m_chatStyle->getTemplateHtml();
    QString footerHtml = replaceHeaderKeywords(m_chatStyle->getFooterHtml(), chatInfo);
    QString extraStyleHtml = QLatin1String("@import url( \"main.css\" );");
    m_lastContent = AdiumThemeContentInfo();

    if (templateHtml.isEmpty()) {
        // if templateHtml is empty, we failed to load the fallback template file
        KMessageBox::error(this, i18n("Missing required file template.html - check your installation."));
    }

    if (m_displayHeader) {
        headerHtml = replaceHeaderKeywords(m_chatStyle->getHeaderHtml(), chatInfo);
    } //otherwise leave as blank.

    // set fontFamily and fontSize
    if (m_useCustomFont) {
        // use user specified fontFamily and Size
        settings()->setFontFamily(QWebSettings::StandardFont, m_fontFamily);
        settings()->setFontSize(QWebSettings::DefaultFontSize, m_fontSize);

        // since some themes are pretty odd and hardcode fonts to the css we need to override that
        // with some extra css. this may not work for all themes!
        extraStyleHtml.append (
            QString(QLatin1String("\n* {font-family:\"%1\" !important;font-size:%2pt !important};"))
            .arg( m_fontFamily )
            .arg( m_fontSize )
        );
    } else {
        // FIXME: we should inform the user if the chatStyle want's to use a fontFamily which is not present on the system
        QFontDatabase fontDB = QFontDatabase();
        kDebug() << "Theme font installed: " << m_chatStyle->defaultFontFamily()
        << fontDB.families().contains(m_chatStyle->defaultFontFamily());

        // use theme fontFamily/Size, if not existent, it falls back to systems default font
        settings()->setFontFamily(QWebSettings::StandardFont, m_chatStyle->defaultFontFamily());
        settings()->setFontSize(QWebSettings::DefaultFontSize, m_chatStyle->defaultFontSize());
    }

    //The templateHtml is in a horrific NSString format.
    //Want to use this rather than roll our own, as that way we can get templates from themes too
    //"%@" is each argument.
    // all other %'s are escaped.

    // first is baseref
    // second is extra style code (This is sometimes missing !!!!)
    // third is variant CSS
    // 4th is header
    // 5th is footer

    templateHtml.replace(QLatin1String("%%"), QLatin1String("%"));

    int numberOfPlaceholders = templateHtml.count(QLatin1String("%@"));

    int index = 0;
    index = templateHtml.indexOf(QLatin1String("%@"), index);
    templateHtml.replace(index, 2, QString(QLatin1String("file:///")).append(m_chatStyle->getStyleBaseHref()));

    if (numberOfPlaceholders == 5) {
        index = templateHtml.indexOf(QLatin1String("%@"), index);
        templateHtml.replace(index, 2, extraStyleHtml);
    }

    index = templateHtml.indexOf(QLatin1String("%@"), index);
    templateHtml.replace(index, 2, m_variantPath);

    index = templateHtml.indexOf(QLatin1String("%@"), index);
    templateHtml.replace(index, 2, headerHtml);

    index = templateHtml.indexOf(QLatin1String("%@"), index);
    templateHtml.replace(index, 2, footerHtml);

    setHtml(templateHtml);

    //hidden HTML debugging mode. Should have no visible way to turn it on.
    if (m_webInspector) {
        QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    }
}

void AdiumThemeView::setVariant(const QString &variant)
{
    m_variantName = variant;
    m_variantPath = QString(QLatin1String("Variants/%1.css")).arg(variant);

}

ChatWindowStyle *AdiumThemeView::chatStyle() const
{
    return m_chatStyle;
}

void AdiumThemeView::setChatStyle(ChatWindowStyle *chatStyle)
{
    m_chatStyle = chatStyle;

    //load the first variant
    QHash<QString, QString> variants = chatStyle->getVariants();
    if (!chatStyle->defaultVariantName().isEmpty()
            && variants.keys().contains(chatStyle->defaultVariantName())) {
        m_variantPath = variants.value(chatStyle->defaultVariantName());
        m_variantName = chatStyle->defaultVariantName();
    } else if (variants.keys().length() > 0) {
        m_variantPath = variants.values().first();
        m_variantName = variants.keys().first();
    } else {
        m_variantPath = QLatin1String("");
        m_variantName = QLatin1String("");
    }
}

QString AdiumThemeView::fontFamily()
{
    return m_fontFamily;
}

void AdiumThemeView::setFontFamily(QString fontFamily)
{
    kDebug();
    m_fontFamily = fontFamily;
}

int AdiumThemeView::fontSize()
{
    return m_fontSize;
}

void AdiumThemeView::setFontSize(int fontSize)
{
    kDebug();
    m_fontSize = fontSize;
}

void AdiumThemeView::setUseCustomFont(bool useCustomFont)
{
    kDebug();
    m_useCustomFont = useCustomFont;
}

bool AdiumThemeView::isCustomFont() const
{
    return m_useCustomFont;
}

bool AdiumThemeView::isHeaderDisplayed() const
{
    return m_displayHeader;
}

void AdiumThemeView::setHeaderDisplayed(bool displayHeader)
{
    m_displayHeader = displayHeader;
}


void AdiumThemeView::addContentMessage(const AdiumThemeContentInfo &contentMessage)
{
    QString styleHtml;
    bool consecutiveMessage = false;

    if (m_lastContent.senderScreenName() == contentMessage.senderScreenName()
        && m_lastContent.type() == contentMessage.type() )
    {
        consecutiveMessage = true;
    }
    m_lastContent = contentMessage;

    switch (contentMessage.type()) {
    case AdiumThemeMessageInfo::RemoteToLocal:
        if (consecutiveMessage) {
            styleHtml = m_chatStyle->getNextIncomingHtml();
        } else {
            styleHtml = m_chatStyle->getIncomingHtml();
        }
        break;
    case AdiumThemeMessageInfo::LocalToRemote:
        if (consecutiveMessage) {
            styleHtml = m_chatStyle->getNextOutgoingHtml();
        } else {
            styleHtml = m_chatStyle->getOutgoingHtml();
        }
        break;
    case AdiumThemeMessageInfo::HistoryRemoteToLocal:
        if (consecutiveMessage) {
            styleHtml = m_chatStyle->getHistoryNextIncomingHtml();
        } else {
            styleHtml = m_chatStyle->getHistoryIncomingHtml();
        }
        break;
    case AdiumThemeMessageInfo::HistoryLocalToRemote:
        if (consecutiveMessage) {
            styleHtml = m_chatStyle->getHistoryNextOutgoingHtml();
        } else {
            styleHtml = m_chatStyle->getHistoryOutgoingHtml();
        }
        break;
    default:
        qWarning() << "Unexpected message type to addContentMessage";
    }

    replaceContentKeywords(styleHtml, contentMessage);

    if (consecutiveMessage) {
        appendNextMessage(styleHtml);
    } else {
        appendNewMessage(styleHtml);
    }
}

void AdiumThemeView::addStatusMessage(const AdiumThemeStatusInfo& statusMessage)
{
    QString styleHtml = m_chatStyle->getStatusHtml();
    m_lastContent = AdiumThemeContentInfo();
    replaceStatusKeywords(styleHtml, statusMessage);
    appendNewMessage(styleHtml);
}

void AdiumThemeView::onLinkClicked(const QUrl &url)
{
    KToolInvocation::invokeBrowser(url.toString());
}

void AdiumThemeView::onOpenLinkActionTriggered()
{
    QUrl url = m_openLinkAction->data().toUrl();
    onLinkClicked(url);
}

/** Private */

QString AdiumThemeView::replaceHeaderKeywords(QString htmlTemplate, const AdiumThemeHeaderInfo & info)
{
    htmlTemplate.replace(QLatin1String("%chatName%"), info.chatName());
    htmlTemplate.replace(QLatin1String("%sourceName%"), info.sourceName());
    htmlTemplate.replace(QLatin1String("%destinationName%"), info.destinationName());
    htmlTemplate.replace(QLatin1String("%destinationDisplayName%"), info.destinationDisplayName());
    htmlTemplate.replace(QLatin1String("%incomingIconPath%"), (!info.incomingIconPath().isEmpty() ? info.incomingIconPath().toString() : m_defaultAvatar));
    htmlTemplate.replace(QLatin1String("%outgoingIconPath%"), (!info.outgoingIconPath().isEmpty() ? info.incomingIconPath().toString() : m_defaultAvatar));
    htmlTemplate.replace(QLatin1String("%timeOpened%"), KGlobal::locale()->formatTime(info.timeOpened().time()));
    htmlTemplate.replace(QLatin1String("%dateOpened%"), KGlobal::locale()->formatDate(info.timeOpened().date(), KLocale::LongDate));

    //FIXME time fields - remember to do both, steal the complicated one from Kopete code.
    // Look for %timeOpened{X}%
    QRegExp timeRegExp(QLatin1String("%timeOpened\\{([^}]*)\\}%"));
    int pos = 0;
    while ((pos = timeRegExp.indexIn(htmlTemplate , pos)) != -1) {
        QString timeKeyword = formatTime(timeRegExp.cap(1), info.timeOpened());
        htmlTemplate.replace(pos , timeRegExp.cap(0).length() , timeKeyword);
    }
    return htmlTemplate;
}

QString AdiumThemeView::replaceContentKeywords(QString& htmlTemplate, const AdiumThemeContentInfo& info)
{
    //userIconPath
    htmlTemplate.replace(QLatin1String("%userIconPath%"), !info.userIconPath().isEmpty() ? info.userIconPath() : m_defaultAvatar);
    //senderScreenName
    htmlTemplate.replace(QLatin1String("%senderScreenName%"), info.senderScreenName());
    //sender
    htmlTemplate.replace(QLatin1String("%sender%"), info.sender());
    //senderColor
    htmlTemplate.replace(QLatin1String("%senderColor%"), info.senderColor());
    //senderStatusIcon
    htmlTemplate.replace(QLatin1String("senderStatusIcon"), info.senderStatusIcon());
    //messageDirection
    htmlTemplate.replace(QLatin1String("%messageDirection%"), info.messageDirection());
    //senderDisplayName
    htmlTemplate.replace(QLatin1String("%senderDisplayName%"), info.senderDisplayName());

    //FIXME %textbackgroundcolor{X}%
    return replaceMessageKeywords(htmlTemplate, info);
}

QString AdiumThemeView::replaceStatusKeywords(QString &htmlTemplate, const AdiumThemeStatusInfo& info)
{
    htmlTemplate.replace(QLatin1String("%status%"), info.status());
    return replaceMessageKeywords(htmlTemplate, info);
}

QString AdiumThemeView::replaceMessageKeywords(QString &htmlTemplate, const AdiumThemeMessageInfo& info)
{
    //message
    QString message = info.message();

    htmlTemplate.replace(QLatin1String("%message%"), message);

    //service
    htmlTemplate.replace(QLatin1String("%service%"), info.service());
    //time
    htmlTemplate.replace(QLatin1String("%time%"), KGlobal::locale()->formatLocaleTime(info.time().time()));
    //shortTime
    htmlTemplate.replace(QLatin1String("%shortTime%"), KGlobal::locale()->formatLocaleTime(info.time().time(), KLocale::TimeWithoutSeconds | KLocale::TimeWithoutAmPm));
    //time{X}
    QRegExp timeRegExp(QLatin1String("%time\\{([^}]*)\\}%"));
    int pos = 0;
    while ((pos = timeRegExp.indexIn(htmlTemplate , pos)) != -1) {
        QString timeKeyword = formatTime(timeRegExp.cap(1), info.time());
        htmlTemplate.replace(pos , timeRegExp.cap(0).length() , timeKeyword);
    }

    htmlTemplate.replace(QLatin1String("%messageClasses%"), info.messageClasses());


    return htmlTemplate;
}

void AdiumThemeView::appendNewMessage(QString &html)
{
    //by making the JS return false evaluateJavaScript is a _lot_ faster, as it has nothing to convert to QVariant.
    //escape quotes, and merge HTML onto one line.
    QString js = QString(QLatin1String("appendMessage(\"%1\");false;")).arg(html.replace(QLatin1Char('"'),
                                                                            QLatin1String("\\\"")).replace(QLatin1Char('\n'), QLatin1String("")));
    page()->mainFrame()->evaluateJavaScript(js);
}

void AdiumThemeView::appendNextMessage(QString &html)
{
    QString js = QString(QLatin1String("appendNextMessage(\"%1\");false;")).arg(html.replace(QLatin1Char('"'),
                                                                                QLatin1String("\\\"")).replace(QLatin1Char('\n'), QLatin1String("")));
    page()->mainFrame()->evaluateJavaScript(js);
}


//taken from Kopete code
QString AdiumThemeView::formatTime(const QString &timeFormat, const QDateTime &dateTime)
{
    QString format = timeFormat;
    format.replace(QLatin1String("%a"), QLatin1String("ddd"));
    format.replace(QLatin1String("%b"), QLatin1String("MMM"));
    format.replace(QLatin1String("%d"), QLatin1String("dd"));
    format.replace(QLatin1String("%H"), QLatin1String("HH"));
    format.replace(QLatin1String("%M"), QLatin1String("MM"));
    format.replace(QLatin1String("%S"), QLatin1String("ss"));
    return dateTime.toString(format);
}

const QString AdiumThemeView::variantName() const
{
    return m_variantName;
}

const QString AdiumThemeView::variantPath() const
{
    return m_variantPath;
}



