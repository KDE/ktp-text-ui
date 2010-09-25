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

#include "adiumthemeview.h"

#include "adiumthemeheaderinfo.h"
#include "adiumthemecontentinfo.h"
#include "adiumthememessageinfo.h"
#include "adiumthemestatusinfo.h"

#include "chatwindowstylemanager.h"

#include <QDebug>
#include <QWebFrame>
#include <QWebElement>
#include <QFile>
#include <QTextCodec>
#include <QTextDocument> //needed for Qt::escape
#include <QWebInspector>
#include <QWebSettings>

#include <KDebug>
#include <KEmoticonsTheme>
#include <KGlobal>
#include <KStandardDirs>
#include <KConfig>
#include <KConfigGroup>
#include <KMessageBox>


AdiumThemeView::AdiumThemeView(QWidget *parent) :
        QWebView(parent),
        m_displayHeader(true)
{
    //determine the chat window style to use (from the Kopete config file).

    KSharedConfigPtr config = KSharedConfig::openConfig("ktelepathyrc");
    KConfigGroup appearanceConfig = config->group("Appearance");

    QString chatStyleName = appearanceConfig.readEntry("styleName", "renkoo.AdiumMessageStyle");
    m_chatStyle = ChatWindowStyleManager::self()->getValidStyleFromPool(chatStyleName);
    if (m_chatStyle == 0 || !m_chatStyle->isValid()) {
        KMessageBox::error(this, "Failed to load a valid theme. Please make sure you run the chat window configuration program first. Will now crash.");
    }

    QString variant = appearanceConfig.readEntry("styleVariant");
    if(!variant.isEmpty()) {
        m_variantPath = QString("Variants/%1.css").arg(variant);
        m_variantName = variant;
    } else {
        m_variantPath = QString("Variants/%1.css").arg(m_chatStyle->defaultVariantName());
        m_variantName = m_chatStyle->defaultVariantName();
    }
    m_displayHeader = appearanceConfig.readEntry("displayHeader", false);


    //special HTML debug mode. Debugging/Profiling only (or theme creating) should have no visible way to turn this flag on.
    m_webInspector = appearanceConfig.readEntry("debug", false);
}

void AdiumThemeView::initialise(const AdiumThemeHeaderInfo &chatInfo)
{
    QString templateHtml;
    QString templateFileName(KGlobal::dirs()->findResource("data", "ktelepathy/template.html"));

    templateHtml = m_chatStyle->getTemplateHtml();

    if (templateHtml.isEmpty()) {
        //FIXME, move this to ChatStyle (maybe?)
        QString templateFileName(KGlobal::dirs()->findResource("data", "ktelepathy/template.html"));

        if (! templateFileName.isEmpty() && QFile::exists(templateFileName)) {
            QFile fileAccess;

            fileAccess.setFileName(templateFileName);
            fileAccess.open(QIODevice::ReadOnly);
            QTextStream headerStream(&fileAccess);
            headerStream.setCodec(QTextCodec::codecForName("UTF-8"));
            templateHtml = headerStream.readAll();
            fileAccess.close();
        } else {
            KMessageBox::error(this, "Missing required file template.html - check your installation.");
        }
    }

    QString headerHtml;
    if (m_displayHeader) {
        headerHtml = replaceHeaderKeywords(m_chatStyle->getHeaderHtml(), chatInfo);
    } //otherwise leave as blank.
    QString footerHtml;
    footerHtml = replaceHeaderKeywords(m_chatStyle->getFooterHtml(), chatInfo);

    QString extraStyleHtml = "@import url( \"main.css\" );";

    //The templateHtml is in a horrific NSString format.
    //Want to use this rather than roll our own, as that way we can get templates from themes too
    //"%@" is each argument.
    // all other %'s are escaped.

    // first is baseref
    // second is extra style code (This is sometimes missing !!!!)
    // third is variant CSS
    // 4th is header
    // 5th is footer

    templateHtml.replace("%%", "%");

    int numberOfPlaceholders = templateHtml.count("%@");

    int index = 0;
    index = templateHtml.indexOf("%@", index);
    templateHtml.replace(index, 2, QString("file:///").append(m_chatStyle->getStyleBaseHref()));

    if (numberOfPlaceholders == 5) {
        index = templateHtml.indexOf("%@", index);
        templateHtml.replace(index, 2, extraStyleHtml);
    }

    index = templateHtml.indexOf("%@", index);
    templateHtml.replace(index, 2, m_variantPath);

    index = templateHtml.indexOf("%@", index);
    templateHtml.replace(index, 2, headerHtml);

    index = templateHtml.indexOf("%@", index);
    templateHtml.replace(index, 2, footerHtml);

    setHtml(templateHtml);
    m_lastSender = "";

    //hidden HTML debugging mode. Should have no visible way to turn it on.
    if (m_webInspector) {
        QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    }
}

void AdiumThemeView::setVariant(const QString &variant)
{
    m_variantName = variant;
    m_variantPath = QString("Variants/%1.css").arg(variant);

    //FIXME update the display!
    initialise(m_chatInfo);
}

ChatWindowStyle* AdiumThemeView::chatStyle() const
{
    return m_chatStyle;
}

void AdiumThemeView::setChatStyle(ChatWindowStyle *chatStyle)
{
    m_chatStyle = chatStyle;

    //load the first variant
    QHash<QString, QString> variants = chatStyle->getVariants();
    if(!chatStyle->defaultVariantName().isEmpty()) {
        m_variantPath = variants.value(chatStyle->defaultVariantName());
        m_variantName = chatStyle->defaultVariantName();
    } else if (variants.keys().length() > 0) {
        m_variantPath = variants.values()[0];
        m_variantName = variants.keys()[0];
    } else {
        m_variantPath = "";
        m_variantName = "";
    }
    initialise(m_chatInfo);
}


bool AdiumThemeView::isHeaderDisplayed() const
{
    return m_displayHeader;
}

void AdiumThemeView::setHeaderDisplayed(bool displayHeader)
{
    m_displayHeader = displayHeader;
    initialise(m_chatInfo);
}


void AdiumThemeView::addContentMessage(const AdiumThemeContentInfo &contentMessage)
{
    QString styleHtml;
    bool consecutiveMessage = false;

    if (m_lastSender == contentMessage.senderScreenName()) {
        consecutiveMessage = true;
    } else {
        m_lastSender = contentMessage.senderScreenName();
    }

    switch (contentMessage.type()) {
    case TelepathyChatMessageInfo::RemoteToLocal:
        if (consecutiveMessage) {
            styleHtml = m_chatStyle->getNextIncomingHtml();
        } else {
            styleHtml = m_chatStyle->getIncomingHtml();
        }
        break;
    case TelepathyChatMessageInfo::LocalToRemote:
        if (consecutiveMessage) {
            styleHtml = m_chatStyle->getNextOutgoingHtml();
        } else {
            styleHtml = m_chatStyle->getOutgoingHtml();
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
    m_lastSender = "";
    replaceStatusKeywords(styleHtml, statusMessage);
    appendNewMessage(styleHtml);
}

void AdiumThemeView::addMessage(const TelepathyChatMessageInfo &message)
{
    QString styleHtml;
    bool consecutiveMessage = false;

    qDebug() << m_lastSender;

    //FIXME "if group consecutive messages....{
    if (m_lastSender == message.senderScreenName()) {
        consecutiveMessage = true;
    } else {
        m_lastSender = message.senderScreenName();
    }

    switch (message.type()) {
    case TelepathyChatMessageInfo::RemoteToLocal:
        if (consecutiveMessage) {
            styleHtml = m_chatStyle->getNextIncomingHtml();
        } else {
            styleHtml = m_chatStyle->getIncomingHtml();
        }
        break;
    case TelepathyChatMessageInfo::LocalToRemote:
        if (consecutiveMessage) {
            styleHtml = m_chatStyle->getNextOutgoingHtml();
        } else {
            styleHtml = m_chatStyle->getOutgoingHtml();
        }
        break;
    case TelepathyChatMessageInfo::Status:
        styleHtml = m_chatStyle->getStatusHtml();
        consecutiveMessage = false;
        break;
    }

    QString messageHtml = m_emoticons.theme().parseEmoticons(Qt::escape(message.message()));

    styleHtml.replace("%message%", messageHtml);
    styleHtml.replace("%messageDirection%", message.messageDirection());
    styleHtml.replace("%sender%", message.senderDisplayName()); // FIXME sender is complex: not always this
    styleHtml.replace("%senderScreenName%", message.senderScreenName());
    styleHtml.replace("%time%", KGlobal::locale()->formatTime(message.time().time(), true));
    styleHtml.replace("%shortTime%", KGlobal::locale()->formatTime(message.time().time(), false));
    styleHtml.replace("%userIconPath%", "outgoing_icon.png");// this fallback should be done in the messageinfo
    styleHtml.replace("%messageClasses%", message.messageClasses());
    styleHtml.replace("%service%", message.service());
    styleHtml.replace("%userIcons%", message.userIcons());
    styleHtml.replace("%status%", "idle");


    // Look for %time{X}%
    QRegExp timeRegExp("%time\\{([^}]*)\\}%");
    int pos = 0;
    while ((pos = timeRegExp.indexIn(styleHtml , pos)) != -1) {
        QString timeKeyword = formatTime(timeRegExp.cap(1), message.time());
        styleHtml.replace(pos , timeRegExp.cap(0).length() , timeKeyword);
    }

    if (consecutiveMessage) {
        appendNextMessage(styleHtml);
    } else {
        appendNewMessage(styleHtml);
    }
}


QString AdiumThemeView::replaceHeaderKeywords(QString htmlTemplate, const AdiumThemeHeaderInfo & info)
{
    htmlTemplate.replace("%chatName%", info.chatName());
    htmlTemplate.replace("%sourceName%", info.sourceName());
    htmlTemplate.replace("%destinationName%", info.destinationName());
    htmlTemplate.replace("%destinationDisplayName%", info.destinationDisplayName());
    htmlTemplate.replace("%incomingIconPath%", info.incomingIconPath().toString());
    htmlTemplate.replace("%outgoingIconPath%", info.outgoingIconPath().toString());
    htmlTemplate.replace("%timeOpened%", KGlobal::locale()->formatDateTime(info.timeOpened()));

    //FIXME time fields - remember to do both, steal the complicated one from Kopete code.
    // Look for %timeOpened{X}%
    QRegExp timeRegExp("%timeOpened\\{([^}]*)\\}%");
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
    htmlTemplate.replace("%userIconPath%", info.userIconPath());
    //senderScreenName
    htmlTemplate.replace("%senderScreenName%", info.senderScreenName());
    //sender
    htmlTemplate.replace("%sender%", info.sender());
    //senderColor
    htmlTemplate.replace("%senderColor%", info.senderColor());
    //senderStatusIcon
    htmlTemplate.replace("senderStatusIcon", info.senderStatusIcon());
    //messageDirection
    htmlTemplate.replace("%messageDirection%", info.messageDirection());
    //senderDisplayName
    htmlTemplate.replace("%senderDisplayName%", info.senderDisplayName());

    //FIXME %textbackgroundcolor{X}%
    return replaceMessageKeywords(htmlTemplate, info);
}

QString AdiumThemeView::replaceStatusKeywords(QString &htmlTemplate, const AdiumThemeStatusInfo& info)
{
    htmlTemplate.replace("%status%", info.status());
    return replaceMessageKeywords(htmlTemplate, info);
}

QString AdiumThemeView::replaceMessageKeywords(QString &htmlTemplate, const AdiumThemeMessageInfo& info)
{
    //message
    htmlTemplate.replace("%message%", info.message());
    //time
    htmlTemplate.replace("%time%", KGlobal::locale()->formatTime(info.time().time(), true));
    //shortTime
    htmlTemplate.replace("%shortTime%", KGlobal::locale()->formatTime(info.time().time(), false));
    //time{X}
    QRegExp timeRegExp("%time\\{([^}]*)\\}%");
    int pos = 0;
    while ((pos = timeRegExp.indexIn(htmlTemplate , pos)) != -1) {
        QString timeKeyword = formatTime(timeRegExp.cap(1), info.time());
        htmlTemplate.replace(pos , timeRegExp.cap(0).length() , timeKeyword);
    }
    return htmlTemplate;
}


void AdiumThemeView::appendNewMessage(QString &html)
{
    //by making the JS return false evaluateJavaScript is a _lot_ faster, as it has nothing to convert to QVariant.
    //escape quotes, and merge HTML onto one line.
    QString js = QString("appendMessage(\"%1\");false;").arg(html.replace('"', "\\\"").replace('\n', ""));
    page()->mainFrame()->evaluateJavaScript(js);
}

void AdiumThemeView::appendNextMessage(QString &html)
{
    QString js = QString("appendNextMessage(\"%1\");false;").arg(html.replace('"', "\\\"").replace('\n', ""));
    page()->mainFrame()->evaluateJavaScript(js);
}


//taken from Kopete code
QString AdiumThemeView::formatTime(const QString &_timeFormat, const QDateTime &dateTime)
{
    char buffer[256];
#ifdef Q_WS_WIN
    QString timeFormat = _timeFormat;
    // some formats are not supported on windows (gnu extension?)
    timeFormat = timeFormat.replace(QLatin1String("%e"), QLatin1String("%d"));
    timeFormat = timeFormat.replace(QLatin1String("%T"), QLatin1String("%H:%M:%S"));
#else
    const QString timeFormat = _timeFormat;
#endif
    // Get current time
    time_t timeT = dateTime.toTime_t();
    // Convert it to local time representation.
    struct tm* loctime = localtime(&timeT);
    strftime(buffer, 256, timeFormat.toAscii(), loctime);

    return QString(buffer);
}

const QString AdiumThemeView::variantName() const
{
    return m_variantName;
}

const QString AdiumThemeView::variantPath() const
{
    return m_variantPath;
}
