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

#include <KTp/message-processor.h>

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

    QWebSettings *ws = settings();
    ws->setAttribute(QWebSettings::ZoomTextOnly, true);
}

void AdiumThemeView::load(ChatType chatType) {

    //determine the chat window style to use
    KSharedConfigPtr config = KSharedConfig::openConfig(QLatin1String("ktelepathyrc"));
    KConfigGroup appearanceConfig;

    if (chatType == AdiumThemeView::SingleUserChat) {
        appearanceConfig = config->group("Appearance");
        m_chatStyle = ChatWindowStyleManager::self()->getValidStyleFromPool(appearanceConfig.readEntry(QLatin1String("styleName"), "renkoo.AdiumMessageStyle"));
    } else {
        appearanceConfig = config->group("GroupAppearance");
        m_chatStyle = ChatWindowStyleManager::self()->getValidStyleFromPool(appearanceConfig.readEntry(QLatin1String("styleName"), "SimKete.AdiumMessageStyle"));
    }

    if (m_chatStyle == 0 || !m_chatStyle->isValid()) {
        KMessageBox::error(this, i18n("Failed to load a valid theme. Your installation is broken. Check your kde path. "
                                      "Will now crash."));
    }

    QString variant = appearanceConfig.readEntry(QLatin1String("styleVariant"));
    if (!variant.isEmpty()) {
        m_variantName = variant;
        m_variantPath = m_chatStyle->getVariants().value(variant);

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

    m_showPresenceChanges = appearanceConfig.readEntry("showPresenceChanges", true);
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

void AdiumThemeView::wheelEvent(QWheelEvent* event)
{
    // Zoom text on Ctrl + Scroll
    if (event->modifiers() & Qt::CTRL) {
        qreal factor = zoomFactor();
        if (event->delta() > 0) {
            factor += 0.1;
        } else if (event->delta() < 0) {
            factor -= 0.1;
        }
        setZoomFactor(factor);
        Q_EMIT zoomFactorChanged(factor);

        event->accept();
        return;
    }

    QWebView::wheelEvent(event);
}

void AdiumThemeView::initialise(const AdiumThemeHeaderInfo &chatInfo)
{
    QString headerHtml;
    QString templateHtml = m_chatStyle->getTemplateHtml();
    QString footerHtml = replaceHeaderKeywords(m_chatStyle->getFooterHtml(), chatInfo);
    QString extraStyleHtml = m_chatStyle->messageViewVersion() < 3 ? QLatin1String("")
                                                                   : QLatin1String("@import url( \"main.css\" );");
    m_lastContent = AdiumThemeContentInfo();

    if (templateHtml.isEmpty()) {
        // if templateHtml is empty, we failed to load the fallback template file
        KMessageBox::error(this, i18n("Missing required file Template.html - check your installation."));
    }

    if (m_displayHeader) {
        if (chatInfo.isGroupChat()) {
            // In group chats header should be replaced by topic
            headerHtml = replaceHeaderKeywords(m_chatStyle->getTopicHtml(), chatInfo);
        } else {
            headerHtml = replaceHeaderKeywords(m_chatStyle->getHeaderHtml(), chatInfo);
        }
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
    templateHtml.replace(index, 2, QString(QLatin1String("file://")).append(m_chatStyle->getStyleBaseHref()));

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

    // Inject the scripts and the css just before the end of the head tag
    index = templateHtml.indexOf(QLatin1String("</head>"));
    templateHtml.insert(index, KTp::MessageProcessor::instance()->header());

    //kDebug() << templateHtml;

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

void AdiumThemeView::setShowPresenceChanges(bool showPresenceChanges)
{
    kDebug();
    m_showPresenceChanges = showPresenceChanges;
}

bool AdiumThemeView::showPresenceChanges() const
{
    return m_showPresenceChanges;
}

bool AdiumThemeView::isHeaderDisplayed() const
{
    return m_displayHeader;
}

void AdiumThemeView::setHeaderDisplayed(bool displayHeader)
{
    m_displayHeader = displayHeader;
}

void AdiumThemeView::clear()
{
    if (!page()->mainFrame()->url().isEmpty()) {
        page()->mainFrame()->setHtml(QString());
    }
}

void AdiumThemeView::addContentMessage(const AdiumThemeContentInfo &contentMessage)
{
    QString styleHtml;
    bool consecutiveMessage = false;
    bool willAddMoreContentObjects = false; // TODO Find out how this is used in Adium
    bool replaceLastContent = false; // TODO use this

    // contentMessage is const, we need a non-const one to append message classes
    AdiumThemeContentInfo message(contentMessage);

    // 2 consecutive messages can be combined when:
    //  * Sender is the same
    //  * Message type is the same
    //  * Both have the "mention" class, or none of them have it
    //  * Theme does not disable consecutive messages
    if (m_lastContent.senderScreenName() == message.senderScreenName()
        && m_lastContent.type() == message.type()
        && m_lastContent.messageClasses().contains(QLatin1String("mention")) == message.messageClasses().contains(QLatin1String("mention"))
        && !m_chatStyle->disableCombineConsecutive()) {
        consecutiveMessage = true;
        message.appendMessageClass(QLatin1String("consecutive"));
    }

    m_lastContent = message;

    switch (message.type()) {
    case AdiumThemeMessageInfo::RemoteToLocal:
        if (consecutiveMessage) {
            styleHtml = m_chatStyle->getIncomingNextContentHtml();
        } else {
            styleHtml = m_chatStyle->getIncomingContentHtml();
        }
        break;
    case AdiumThemeMessageInfo::LocalToRemote:
        if (consecutiveMessage) {
            styleHtml = m_chatStyle->getOutgoingNextContentHtml();
        } else {
            styleHtml = m_chatStyle->getOutgoingContentHtml();
        }
        break;
    case AdiumThemeMessageInfo::HistoryRemoteToLocal:
        if (consecutiveMessage) {
            styleHtml = m_chatStyle->getIncomingNextHistoryHtml();
        } else {
            styleHtml = m_chatStyle->getIncomingHistoryHtml();
        }
        break;
    case AdiumThemeMessageInfo::HistoryLocalToRemote:
        if (consecutiveMessage) {
            styleHtml = m_chatStyle->getOutgoingNextHistoryHtml();
        } else {
            styleHtml = m_chatStyle->getOutgoingHistoryHtml();
        }
        break;
    default:
        kWarning() << "Unexpected message type to addContentMessage";
    }

    replaceContentKeywords(styleHtml, message);

    AppendMode mode = appendMode(message,
                                 consecutiveMessage,
                                 willAddMoreContentObjects,
                                 replaceLastContent);

    appendMessage(styleHtml, message.script(), mode);
}

void AdiumThemeView::addStatusMessage(const AdiumThemeStatusInfo& statusMessage)
{
    QString styleHtml;
    bool consecutiveMessage = false;
    bool willAddMoreContentObjects = false; // TODO Find out how this is used in Adium
    bool replaceLastContent = false; // TODO use this

    // statusMessage is const, we need a non-const one to append message classes
    AdiumThemeStatusInfo message(statusMessage);

    if (m_lastContent.type() == message.type() && !m_chatStyle->disableCombineConsecutive()) {
        consecutiveMessage = true;
        message.appendMessageClass(QLatin1String("consecutive"));
    }

    m_lastContent = AdiumThemeContentInfo(statusMessage.type());

    switch (message.type()) {
    case AdiumThemeMessageInfo::Status:
        styleHtml = m_chatStyle->getStatusHtml();
        break;
    case AdiumThemeMessageInfo::HistoryStatus:
        styleHtml = m_chatStyle->getStatusHistoryHtml();
        break;
    default:
        kWarning() << "Unexpected message type to addStatusMessage";
    }

    replaceStatusKeywords(styleHtml, message);

    AppendMode mode = appendMode(message,
                                 consecutiveMessage,
                                 willAddMoreContentObjects,
                                 replaceLastContent);

    appendMessage(styleHtml, message.script(), mode);
}

QString AdiumThemeView::appendScript(AdiumThemeView::AppendMode mode)
{
    //by making the JS return false evaluateJavaScript is a _lot_ faster, as it has nothing to convert to QVariant.
    //escape quotes, and merge HTML onto one line.
    switch (mode) {
    case AppendMessageWithScroll:
        kDebug() << "AppendMessageWithScroll";
        return QLatin1String("checkIfScrollToBottomIsNeeded(); appendMessage(\"%1\"); scrollToBottomIfNeeded(); false;");
    case AppendNextMessageWithScroll:
        kDebug() << "AppendNextMessageWithScroll";
        return QLatin1String("checkIfScrollToBottomIsNeeded(); appendNextMessage(\"%1\"); scrollToBottomIfNeeded(); false;");
    case AppendMessage:
        kDebug() << "AppendMessage";
        return QLatin1String("appendMessage(\"%1\"); false;");
    case AppendNextMessage:
        kDebug() << "AppendNextMessage";
        return QLatin1String("appendNextMessage(\"%1\"); false;");
    case AppendMessageNoScroll:
        kDebug() << "AppendMessageNoScroll";
        return QLatin1String("appendMessageNoScroll(\"%1\"); false;");
    case AppendNextMessageNoScroll:
        kDebug() << "AppendNextMessageNoScroll";
        return QLatin1String("appendNextMessageNoScroll(\"%1\"); false;");
    case ReplaceLastMessage:
        kDebug() << "ReplaceLastMessage";
        return QLatin1String("replaceLastMessage(\"%1\"); false");
    default:
        kWarning() << "Unhandled append mode!";
        return QLatin1String("%1");
    }
}

AdiumThemeView::AppendMode AdiumThemeView::appendMode(const AdiumThemeMessageInfo &message,
                                                      bool consecutive,
                                                      bool willAddMoreContentObjects, // TODO Find out how this is used in Adium
                                                      bool replaceLastContent)
{
    AdiumThemeView::AppendMode mode = AppendModeError;
    // scripts vary by style version
    if (!m_chatStyle->hasCustomTemplateHtml() && m_chatStyle->messageViewVersion() >= 4) {
        // If we're using the built-in template HTML, we know that it supports our most modern scripts
        if (replaceLastContent)
            mode = ReplaceLastMessage;
        else if (willAddMoreContentObjects) {
            mode = (consecutive ? AppendNextMessageNoScroll : AppendMessageNoScroll);
        } else {
            mode = (consecutive ? AppendNextMessage : AppendMessage);
        }
    } else  if (m_chatStyle->messageViewVersion() >= 3) {
        if (willAddMoreContentObjects) {
            mode = (consecutive ? AppendNextMessageNoScroll : AppendMessageNoScroll);
        } else {
            mode = (consecutive ? AppendNextMessage : AppendMessage);
        }
    } else if (m_chatStyle->messageViewVersion() >= 1) {
        mode = (consecutive ? AppendNextMessage : AppendMessage);
    } else if (m_chatStyle->hasCustomTemplateHtml() && (message.type() == AdiumThemeContentInfo::Status ||
                                                        message.type() == AdiumThemeContentInfo::HistoryStatus)) {
        // Old styles with a custom Template.html had Status.html files without 'insert' divs coupled
        // with a APPEND_NEXT_MESSAGE_WITH_SCROLL script which assumes one exists.
        mode = AppendMessageWithScroll;
    } else {
        mode = (consecutive ? AppendNextMessageWithScroll : AppendMessageWithScroll);
    }

    return mode;
}

void AdiumThemeView::appendMessage(QString &html, const QString &script, AppendMode mode)
{
    QString js = appendScript(mode).arg(html.replace(QLatin1Char('\"'), QLatin1String("\\\""))
                                            .replace(QLatin1Char('\n'), QLatin1String("")));
    page()->mainFrame()->evaluateJavaScript(js);

    if (!script.isEmpty()) {
        page()->mainFrame()->evaluateJavaScript(script);
    }
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
    htmlTemplate.replace(QLatin1String("%topic%"), info.chatName());
    htmlTemplate.replace(QLatin1String("%sourceName%"), info.sourceName());
    htmlTemplate.replace(QLatin1String("%destinationName%"), info.destinationName());
    htmlTemplate.replace(QLatin1String("%destinationDisplayName%"), info.destinationDisplayName());
    htmlTemplate.replace(QLatin1String("%incomingIconPath%"), (!info.incomingIconPath().isEmpty() ? info.incomingIconPath().toString() : m_defaultAvatar));
    htmlTemplate.replace(QLatin1String("%outgoingIconPath%"), (!info.outgoingIconPath().isEmpty() ? info.outgoingIconPath().toString() : m_defaultAvatar));
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
    htmlTemplate.replace(QLatin1String("%serviceIconImg%"),
                         QString::fromLatin1("<img src=\"%1\" class=\"serviceIcon\" />").arg(info.serviceIconImg()));
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
    htmlTemplate.replace(QLatin1String("%senderStatusIcon%"), info.senderStatusIcon());
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

    if(info.messageDirection() == QLatin1String("rtl")) {
        message.prepend(QString::fromLatin1("<div dir=\"rtl\">"));
        message.append(QLatin1String("</div>"));
    }

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

    //messageDirection
    htmlTemplate.replace(QLatin1String("%messageDirection%"), info.messageDirection());
    htmlTemplate.replace(QLatin1String("%messageClasses%"), info.messageClasses());


    return htmlTemplate;
}

//taken from Kopete code
QString AdiumThemeView::formatTime(const QString &timeFormat, const QDateTime &dateTime)
{
    QString format = timeFormat;

    // see "man date"

    // Just discard the modifiers
    format.replace(QLatin1String("%-"), QLatin1String("%")); // (hyphen) do not pad the field
    format.replace(QLatin1String("%_"), QLatin1String("%")); // (underscore) pad with spaces
    format.replace(QLatin1String("%0"), QLatin1String("%")); // (zero) pad with zeros
    format.replace(QLatin1String("%^"), QLatin1String("%")); // use upper case if possible
    format.replace(QLatin1String("%#"), QLatin1String("%")); // use opposite case if possible

    // Now do the real replacement
    format.replace(QLatin1String("%a"), QLatin1String("ddd"));        // locale's abbreviated weekday name (e.g., Sun)
    format.replace(QLatin1String("%A"), QLatin1String("dddd"));       // locale's full weekday name (e.g., Sunday)
    format.replace(QLatin1String("%b"), QLatin1String("MMM"));        // locale's abbreviated month name (e.g., Jan)
    format.replace(QLatin1String("%B"), QLatin1String("MMMM"));       // locale's full month name (e.g., January)
    format.replace(QLatin1String("%c"), QLatin1String("ddd MMM d hh:mm:ss yyyy")); // FIXME locale's date and time (e.g., Thu Mar  3 23:05:25 2005)
    format.replace(QLatin1String("%C"), QLatin1String(""));           // FIXME century; like %Y, except omit last two digits (e.g., 20)
    format.replace(QLatin1String("%d"), QLatin1String("dd"));         // day of month (e.g., 01)
    format.replace(QLatin1String("%D"), QLatin1String("MM/dd/yy"));   // date; same as %m/%d/%y
    format.replace(QLatin1String("%e"), QLatin1String("d"));          // FIXME day of month, space padded; same as %_d
    format.replace(QLatin1String("%F"), QLatin1String("yyyy-MM-dd")); // full date; same as %Y-%m-%d
    format.replace(QLatin1String("%g"), QLatin1String(""));           // FIXME last two digits of year of ISO week number (see %G)
    format.replace(QLatin1String("%G"), QLatin1String(""));           // year of ISO week number (see %V); normally useful only with %V
    format.replace(QLatin1String("%h"), QLatin1String("MMM"));        // same as %b
    format.replace(QLatin1String("%H"), QLatin1String("HH"));         // hour (00..23)
    format.replace(QLatin1String("%I"), QLatin1String("hh"));         // FIXME hour (01..12)
    format.replace(QLatin1String("%j"), QLatin1String(""));           // FIXME day of year (001..366)
    format.replace(QLatin1String("%k"), QLatin1String("H"));          // hour, space padded ( 0..23); same as %_H
    format.replace(QLatin1String("%l"), QLatin1String("h"));          // hour, space padded ( 0..23); same as %_H
    format.replace(QLatin1String("%m"), QLatin1String("MM"));         // month (01..12)
    format.replace(QLatin1String("%M"), QLatin1String("mm"));         // minute (00..59)
    format.replace(QLatin1String("%n"), QLatin1String("\n"));         // a newline
    format.replace(QLatin1String("%N"), QLatin1String("zzz"));        // FIXME nanoseconds (000000000..999999999)
    format.replace(QLatin1String("%p"), QLatin1String("AP"));         // locale's equivalent of either AM or PM; blank if not known
    format.replace(QLatin1String("%P"), QLatin1String("ap"));         // like %p, but lower case
    format.replace(QLatin1String("%r"), QLatin1String("hh:mm:ss AP")); // FIXME locale's 12-hour clock time (e.g., 11:11:04 PM)
    format.replace(QLatin1String("%R"), QLatin1String("HH:mm"));      // 24-hour hour and minute; same as %H:%M
    format.replace(QLatin1String("%s"), QLatin1String(""));           // FIXME seconds since 1970-01-01 00:00:00 UTC
    format.replace(QLatin1String("%S"), QLatin1String("ss"));         // second (00..60)
    format.replace(QLatin1String("%t"), QLatin1String("\t"));         // a tab
    format.replace(QLatin1String("%T"), QLatin1String("HH:mm:ss"));   // time; same as %H:%M:%S
    format.replace(QLatin1String("%u"), QLatin1String(""));           // FIXME day of week (1..7); 1 is Monday
    format.replace(QLatin1String("%U"), QLatin1String(""));           // FIXME week number of year, with Sunday as first day of week (00..53)
    format.replace(QLatin1String("%V"), QLatin1String(""));           // FIXME ISO week number, with Monday as first day of week (01..53)
    format.replace(QLatin1String("%w"), QLatin1String(""));           // FIXME day of week (0..6); 0 is Sunday
    format.replace(QLatin1String("%W"), QLatin1String(""));           // FIXME week number of year, with Monday as first day of week (00..53)
    format.replace(QLatin1String("%x"), QLatin1String("MM/dd/yy"));   // FIXME locale's date representation (e.g., 12/31/99)
    format.replace(QLatin1String("%x"), QLatin1String("HH:mm:ss"));   // FIXME locale's time representation (e.g., 23:13:48)
    format.replace(QLatin1String("%y"), QLatin1String("yy"));         // last two digits of year (00..99)
    format.replace(QLatin1String("%Y"), QLatin1String("yyyy"));       // year
    format.replace(QLatin1String("%z"), QLatin1String(""));           // FIXME +hhmm numeric time zone (e.g., -0400)
    format.replace(QLatin1String("%:z"), QLatin1String(""));          // FIXME +hh:mm numeric time zone (e.g., -04:00)
    format.replace(QLatin1String("%::z"), QLatin1String(""));         // FIXME +hh:mm::ss numeric time zone (e.g., -04:00:00)
    format.replace(QLatin1String("%:::z"), QLatin1String(""));        // FIXME numeric time zone with : to necessary precision (e.g., -04, +05:30)
    format.replace(QLatin1String("%Z"), QLatin1String(""));           // FIXME alphabetic time zone abbreviation (e.g., EDT)

    // Last is the literal %
    format.replace(QLatin1String("%%"), QLatin1String("%"));          // a literal %

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
