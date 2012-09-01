/***************************************************************************
 *   Copyright (C) 2012 by David Edmundson <kde@davidedmundson.co.uk>      *
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

#include "message-view.h"

#include "adium-theme-view.h"
#include "adium-theme-status-info.h"
#include "message-processor.h"

#include <KDebug>

#include <TelepathyLoggerQt4/LogManager>
#include <TelepathyLoggerQt4/PendingEvents>
#include <TelepathyLoggerQt4/TextEvent>
#include <TelepathyQt/Account>

MessageView::MessageView(QWidget *parent) :
    AdiumThemeView(parent)
{
    connect(this, SIGNAL(loadFinished(bool)), SLOT(onLoadFinished()));
}


void MessageView::loadLog(const Tp::AccountPtr &account, const Tpl::EntityPtr &entity,
                          const Tp::ContactPtr &contact, const QDate &date,
                          const QPair< QDate, QDate > &nearestDates)
{

    if (account.isNull() || entity.isNull()) {
        //note contact can be null
        kWarning() << "invalid account/contact. Not loading log";
        return;
    }
    
    m_account = account;
    m_entity = entity;
    m_contact = contact;
    m_date = date;
    m_prev = nearestDates.first;
    m_next = nearestDates.second;
    m_initialized = false;
    m_templateLoaded = false;

    if (entity->entityType() == Tpl::EntityTypeRoom) {
        load(AdiumThemeView::GroupChat);
    } else {
        load(AdiumThemeView::SingleUserChat);
    }

    Tp::Avatar avatar = m_account->avatar();
    if (!avatar.avatarData.isEmpty()) {
        m_accountAvatar = QString(QLatin1String("data:%1;base64,%2")).
                            arg(avatar.MIMEType.isEmpty() ? QLatin1String("image/*") : avatar.MIMEType).
                            arg(QString::fromLatin1(m_account->avatar().avatarData.toBase64().data()));
    }

    Tpl::LogManagerPtr logManager = Tpl::LogManager::instance();
    Tpl::PendingEvents *pendingEvents  = logManager->queryEvents(m_account, m_entity, Tpl::EventTypeMaskText, m_date);
    connect(pendingEvents, SIGNAL(finished(Tpl::PendingOperation*)), SLOT(onEventsLoaded(Tpl::PendingOperation*)));
}

void MessageView::setHighlightText(const QString &text)
{
    m_highlightedText = text;
}

void MessageView::clearHighlightText()
{
    setHighlightText(QString());
}

void MessageView::onLoadFinished()
{
    processStoredEvents();

    m_templateLoaded = true;
}

void MessageView::onEventsLoaded(Tpl::PendingOperation *po)
{
    Tpl::PendingEvents *pe = qobject_cast<Tpl::PendingEvents*>(po);

    /* Wait with initialization for the first event so that we can know when the chat session started */
    if (!m_initialized) {
        AdiumThemeHeaderInfo headerInfo;
        headerInfo.setDestinationDisplayName(m_contact.isNull() ? m_entity->alias() : m_contact->alias());
        headerInfo.setChatName(m_contact.isNull() ? m_entity->alias() : m_contact->alias());
        headerInfo.setGroupChat(m_entity->entityType() == Tpl::EntityTypeRoom);
        headerInfo.setSourceName(m_account->displayName());
        headerInfo.setIncomingIconPath(m_contact.isNull() ? QString() : m_contact->avatarData().fileName);

        if (pe->events().count() > 0 && !pe->events().first().isNull()) {
            headerInfo.setTimeOpened(pe->events().first()->timestamp());
        }

        initialise(headerInfo);

        m_initialized = true;
    }

    m_events << pe->events();

    /* Don't add retrieved messages until template is loaded */
    if (m_templateLoaded) {
        processStoredEvents();
    }
}


void MessageView::processStoredEvents()
{
    if (m_prev.isValid()) {
        AdiumThemeStatusInfo message(AdiumThemeMessageInfo::HistoryStatus);
        message.setMessage(QString(QLatin1String("<a href=\"#x-prevConversation\">&lt;&lt;&lt; %1</a>")).arg(i18n("Previous conversation")));
        message.setService(m_account->serviceName());
        message.setTime(QDateTime(m_prev));

        addStatusMessage(message);
    }

    while (!m_events.isEmpty()) {

        const Tpl::TextEventPtr textEvent(m_events.takeFirst().staticCast<Tpl::TextEvent>());

        AdiumThemeMessageInfo::MessageType type;
        QString iconPath;

        if(textEvent->sender()->identifier() == m_account->normalizedName()) {
            type = AdiumThemeMessageInfo::HistoryLocalToRemote;
            iconPath = m_accountAvatar;
        } else {
            type = AdiumThemeMessageInfo::HistoryRemoteToLocal;
            /* FIXME Add support for avatars in MUCs */
            if (m_entity->entityType() == Tpl::EntityTypeContact) {
                if (m_contact) {
                    iconPath = m_contact->avatarData().fileName;
                }
            }
        }

        AdiumThemeContentInfo message(type);
        message.setMessage(MessageProcessor::instance()->processIncomingMessage(textEvent).finalizedMessage());
        message.setService(m_account->serviceName());
        message.setSenderDisplayName(textEvent->sender()->alias());
        message.setSenderScreenName(textEvent->sender()->identifier());
        message.setTime(textEvent->timestamp());
        message.setUserIconPath(iconPath);

        kDebug()    << textEvent->timestamp()
                    << "from" << textEvent->sender()->identifier()
                    << "to" << textEvent->receiver()->identifier()
                    << textEvent->message();

        addContentMessage(message);
    }

    if (m_next.isValid()) {
        AdiumThemeStatusInfo message(AdiumThemeMessageInfo::HistoryStatus);
        message.setMessage(QString(QLatin1String("<a href=\"#x-nextConversation\">%1 &gt;&gt;&gt;</a>")).arg(i18n("Next conversation")));
        message.setService(m_account->serviceName());
        message.setTime(QDateTime(m_next));

        addStatusMessage(message);
    }

    /* Can't highlight the text directly, we need to wait for the JavaScript in
     * AdiumThemeView to include the log messages into DOM. */
    QTimer::singleShot(100, this, SLOT(doHighlightText()));
}

void MessageView::onLinkClicked(const QUrl &link)
{
    if (link.fragment() == QLatin1String("x-nextConversation")) {
        Q_EMIT conversationSwitchRequested(m_next);
        return;
    }

    if (link.fragment() == QLatin1String("x-prevConversation")) {
        Q_EMIT conversationSwitchRequested(m_prev);
        return;
    }

    AdiumThemeView::onLinkClicked(link);
}


void MessageView::doHighlightText()
{
    findText(QString());
    findText(m_highlightedText, QWebPage::HighlightAllOccurrences | QWebPage::FindWrapsAroundDocument);
}
