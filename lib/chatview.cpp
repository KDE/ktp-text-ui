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
#include <QDebug>
#include <QWebFrame>
#include <QWebElement>
#include <QFile>
#include <QTextCodec>
#include <QTextDocument> //needed for Qt::escape

#include <KDebug>
#include <KEmoticonsTheme>
#include <KGlobal>
#include <KStandardDirs>
#include <KConfig>
#include <KConfigGroup>
#include <KMessageBox>


ChatView::ChatView(QWidget *parent) :
        QWebView(parent)
{
    //determine the chat window style to use (from the Kopete config file).
    //FIXME use our own config file. I think we probably want everything from the appearance config group in ours, so it's a simple change.

    KConfig config(KGlobal::dirs()->findResource("config", "kopeterc"));
    KConfigGroup appearanceConfig = config.group("Appearance");

    qDebug() << QString("Loading ") << appearanceConfig.readEntry("styleName");

    m_chatStyle = new ChatWindowStyle(appearanceConfig.readEntry("styleName")); //FIXME this gets leaked !!! //FIXME hardcoded style
    if (!m_chatStyle->isValid()) {
        KMessageBox::error(this, "Failed to load a valid Kopete theme. Note this current version reads chat window settings from your Kopete config file.");
    }

    m_variantPath = appearanceConfig.readEntry("styleVariant");
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
        KMessageBox::error(this, "Missing required file Template.html - check your installation.");
    }

    QString headerHtml;
    if (m_showHeader) {
        headerHtml = replaceHeaderKeywords(m_chatStyle->getHeaderHtml(), chatInfo);
    } //otherwise leave as blank.

    templateHtml.replace("%baseRef%", m_chatStyle->getStyleBaseHref());
    templateHtml.replace("%extraStyleCode%", ""); // FIXME once we get some font from the config file, put it here
    templateHtml.replace("%variant%", m_variantPath);
    templateHtml.replace("%header%", headerHtml);
    templateHtml.replace("%footer%", m_chatStyle->getFooterHtml());

    setHtml(templateHtml);
}



void ChatView::addMessage(TelepathyChatMessageInfo & message)
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
    styleHtml.replace("%time%", message.time().toString());
    styleHtml.replace("%userIconPath%", "Outgoing/buddy_icon.png");// this fallback should be done in the messageinfo

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
    htmlTemplate.replace("%timeOpened%", info.timeOpened().toString()); //FIXME use KLocale to get format.
    //FIXME time fields - remember to do both, steal the complicated one from Kopete code.

    return htmlTemplate;
}

void ChatView::appendNewMessage(QString html)
{
    //by making the JS return false evaluateJavaScript is a _lot_ faster, as it has nothing to convert to QVariant.
    //escape quotes, and merge HTML onto one line.
    QString js = QString("appendMessage(\"%1\");false;").arg(html.replace('"', "\\\"").replace('\n', ""));
    page()->mainFrame()->evaluateJavaScript(js);
}

void ChatView::appendNextMessage(QString html)
{
    QString js = QString("appendNextMessage(\"%1\");false;").arg(html.replace('"', "\\\"").replace('\n', ""));
    page()->mainFrame()->evaluateJavaScript(js);
}
