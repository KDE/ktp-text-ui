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

#include "chatview.h"
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


ChatView::ChatView(QWidget *parent) :
        QWebView(parent),
        m_displayHeader(true)
{
    //determine the chat window style to use (from the Kopete config file).
    //FIXME use our own config file. I think we probably want everything from the appearance config group in ours, so it's a simple change.

    KSharedConfigPtr config = KSharedConfig::openConfig("ktelepathyrc");
    KConfigGroup appearanceConfig = config->group("Appearance");

    QString chatStyleName = appearanceConfig.readEntry("styleName", "Renkoo.AdiumMessageStyle");
    m_chatStyle = ChatWindowStyleManager::self()->getValidStyleFromPool(chatStyleName);
    if (m_chatStyle == 0 || !m_chatStyle->isValid()) {
        KMessageBox::error(this, "Failed to load a valid Kopete theme. Please make sure you run the chat window configuration program first.");
    }

    QString variant = appearanceConfig.readEntry("styleVariant");
    m_variantPath = QString("Variants/%1.css").arg(variant);
    m_displayHeader = appearanceConfig.readEntry("displayHeader", false);


    //special HTML debug mode. Debugging/Profiling only (or theme creating) should have no visible way to turn this flag on.
    m_webInspector = appearanceConfig.readEntry("debug", false);
}

void ChatView::initialise(const TelepathyChatInfo &chatInfo)
{
    QString templateHtml;
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

    QString headerHtml;
    if (m_displayHeader) {
        headerHtml = replaceHeaderKeywords(m_chatStyle->getHeaderHtml(), chatInfo);
    } //otherwise leave as blank.

    templateHtml.replace("%baseRef%", m_chatStyle->getStyleBaseHref());
    templateHtml.replace("%extraStyleCode%", ""); // FIXME once we get some font/background from the config file, put it here
    templateHtml.replace("%variant%", m_variantPath);
    templateHtml.replace("%header%", headerHtml);
    templateHtml.replace("%footer%", m_chatStyle->getFooterHtml());

    setHtml(templateHtml);
    lastSender = "";

    //hidden HTML debugging mode. Should have no visible way to turn it on.
    if (m_webInspector) {
        QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
        QWebInspector* inspector = new QWebInspector(0);
        inspector->setPage(page());
        inspector->show();
    }
}

const QString ChatView::variant() const
{
    return m_variantPath;
}

void ChatView::setVariant(const QString &variant)
{
    m_variantPath = QString("Variants/%1.css").arg(variant);

    //FIXME update the display!
    initialise(m_chatInfo);
}

ChatWindowStyle* ChatView::chatStyle() const
{
    return m_chatStyle;
}

void ChatView::setChatStyle(ChatWindowStyle *chatStyle)
{
    m_chatStyle = chatStyle;

    //load the first variant
    QHash<QString, QString> variants = chatStyle->getVariants();
    if (variants.keys().length() > 0) {
        m_variantPath = variants.values()[0];
    } else {
        m_variantPath = "";
    }


    initialise(m_chatInfo);
}


bool ChatView::isHeaderDisplayed() const
{
    return m_displayHeader;
}

void ChatView::setHeaderDisplayed(bool displayHeader)
{
    m_displayHeader = displayHeader;
    initialise(m_chatInfo);
}

void ChatView::addMessage(const TelepathyChatMessageInfo &message)
{
    QString styleHtml;
    bool consecutiveMessage = false;

    //FIXME "if group consecutive messages....{
    if (lastSender == message.senderScreenName()) {
        consecutiveMessage = true;
    } else {
        lastSender = message.senderScreenName();
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
    styleHtml.replace("%time%", KGlobal::locale()->formatTime(message.time().time(), true));
    styleHtml.replace("%userIconPath%", "Outgoing/buddy_icon.png");// this fallback should be done in the messageinfo

    // Look for %time{X}%
    QRegExp timeRegExp("%time\\{([^}]*)\\}%");
    int pos=0;
    while( (pos=timeRegExp.indexIn(styleHtml , pos) ) != -1 )
    {
            QString timeKeyword = formatTime( timeRegExp.cap(1), message.time() );
            styleHtml.replace( pos , timeRegExp.cap(0).length() , timeKeyword );
    }

    if (consecutiveMessage) {
        appendNextMessage(styleHtml);
    } else {
        appendNewMessage(styleHtml);
    }
}


QString ChatView::replaceHeaderKeywords(QString htmlTemplate, const TelepathyChatInfo & info)
{
    htmlTemplate.replace("%chatName%", info.chatName());
    htmlTemplate.replace("%sourceName%", info.sourceName());
    htmlTemplate.replace("%destinationName%", info.destinationName());
    htmlTemplate.replace("%destinationDisplayName%", info.destinationDisplayName());
    htmlTemplate.replace("%incomingIconPath%", info.incomingIconPath().toString());
    htmlTemplate.replace("%outgoingIconPath%", info.outgoingIconPath().toString());
    htmlTemplate.replace("%timeOpened%", KGlobal::locale()->formatDateTime(info.timeOpened()));
    //FIXME time fields - remember to do both, steal the complicated one from Kopete code.


    return htmlTemplate;
}

void ChatView::appendNewMessage(QString &html)
{
    //by making the JS return false evaluateJavaScript is a _lot_ faster, as it has nothing to convert to QVariant.
    //escape quotes, and merge HTML onto one line.
    QString js = QString("appendMessage(\"%1\");false;").arg(html.replace('"', "\\\"").replace('\n', ""));
    page()->mainFrame()->evaluateJavaScript(js);
}

void ChatView::appendNextMessage(QString &html)
{
    QString js = QString("appendNextMessage(\"%1\");false;").arg(html.replace('"', "\\\"").replace('\n', ""));
    page()->mainFrame()->evaluateJavaScript(js);
}


//taken from Kopete code
QString ChatView::formatTime(const QString &_timeFormat, const QDateTime &dateTime)
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
        struct tm* loctime = localtime (&timeT);
        strftime (buffer, 256, timeFormat.toAscii(), loctime);

        return QString(buffer);
}
