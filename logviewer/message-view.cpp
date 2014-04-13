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
#include <KTp/message-processor.h>

#include <KDebug>
#include <KIconLoader>

#include <QLabel>
#include <QResizeEvent>

#include <KTp/Logger/log-manager.h>
#include <KTp/Logger/pending-logger-logs.h>

#include <TelepathyQt/Account>

MessageView::MessageView(QWidget *parent) :
    AdiumThemeView(parent),
    m_infoLabel(new QLabel(this))
{

    loadSettings();

    QFont font = m_infoLabel->font();
    font.setBold(true);
    m_infoLabel->setFont(font);
    m_infoLabel->setAlignment(Qt::AlignCenter);

    connect(this, SIGNAL(loadFinished(bool)), SLOT(processStoredEvents()));
}

void MessageView::loadLog(const Tp::AccountPtr &account, const KTp::LogEntity &entity,
                          const Tp::ContactPtr &contact, const QDate &date,
                          const QPair< QDate, QDate > &nearestDates)
{
    if (account.isNull() || !entity.isValid()) {
        //note contact can be null
        showInfoMessage(i18n("Unknown or invalid contact"));
        return;
    }

    m_infoLabel->hide();
    m_account = account;
    // FIXME: Workaround for a bug, probably in QGlib which causes that
    // m_entity = m_entity results in invalid m_entity->m_class being null
    if (m_entity != entity) {
        m_entity = entity;
    }
    m_contact = m_contact.dynamicCast<Tp::Contact>(contact);
    m_date = date;
    m_prev = nearestDates.first;
    m_next = nearestDates.second;

    if (entity.entityType() == Tp::HandleTypeRoom) {
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

    KTp::LogManager *logManager = KTp::LogManager::instance();
    KTp::PendingLoggerLogs *pendingLogs = logManager->queryLogs(m_account, m_entity, m_date);
    connect(pendingLogs, SIGNAL(finished(KTp::PendingLoggerOperation*)), SLOT(onEventsLoaded(KTp::PendingLoggerOperation*)));
}

void MessageView::showInfoMessage(const QString& message)
{
    m_infoLabel->setText(message);
    m_infoLabel->show();
    m_infoLabel->raise();
    m_infoLabel->setGeometry(0, 0, width(), height());
}

void MessageView::resizeEvent(QResizeEvent* e)
{
    m_infoLabel->setGeometry(0, 0, e->size().width(), e->size().height());

    QWebView::resizeEvent(e);
}

void MessageView::setHighlightText(const QString &text)
{
    m_highlightedText = text;
}

void MessageView::clearHighlightText()
{
    setHighlightText(QString());
}

void MessageView::onEventsLoaded(KTp::PendingLoggerOperation *po)
{
    KTp::PendingLoggerLogs *pl = qobject_cast<KTp::PendingLoggerLogs*>(po);
    m_events << pl->logs();

    /* Wait with initialization for the first event so that we can know when the chat session started */
    AdiumThemeHeaderInfo headerInfo;
    headerInfo.setDestinationDisplayName(m_contact.isNull() ? m_entity.alias() : m_contact->alias());
    headerInfo.setChatName(m_contact.isNull() ? m_entity.alias() : m_contact->alias());
    headerInfo.setGroupChat(m_entity.entityType() == Tp::HandleTypeRoom);
    headerInfo.setSourceName(m_account->displayName());
    headerInfo.setIncomingIconPath(m_contact.isNull() ? QString() : m_contact->avatarData().fileName);
    headerInfo.setService(m_account->serviceName());
    // check iconPath docs for minus sign in -KIconLoader::SizeMedium
    headerInfo.setServiceIconPath(KIconLoader::global()->iconPath(m_account->iconName(), -KIconLoader::SizeMedium));
    if (pl->logs().count() > 0) {
        headerInfo.setTimeOpened(pl->logs().first().time());
    }

    initialise(headerInfo);
}

bool logMessageOlderThan(const KTp::LogMessage &e1, const KTp::LogMessage &e2)
{
    return e1.time() < e2.time();
}

bool logMessageNewerThan(const KTp::LogMessage &e1, const KTp::LogMessage &e2)
{
    return e1.time() > e2.time();
}

void MessageView::processStoredEvents()
{
    AdiumThemeStatusInfo prevConversation;
    if (m_prev.isValid()) {
        prevConversation = AdiumThemeStatusInfo(AdiumThemeMessageInfo::HistoryStatus);
        prevConversation.setMessage(QString(QLatin1String("<a href=\"#x-prevConversation\">&lt;&lt;&lt; %1</a>")).arg(i18n("Older conversation")));
        prevConversation.setTime(QDateTime(m_prev));
    }

    AdiumThemeStatusInfo nextConversation;
    if (m_next.isValid()) {
        nextConversation = AdiumThemeStatusInfo(AdiumThemeMessageInfo::HistoryStatus);
        nextConversation.setMessage(QString(QLatin1String("<a href=\"#x-nextConversation\">%1 &gt;&gt;&gt;</a>")).arg(i18n("Newer conversation")));
        nextConversation.setTime(QDateTime(m_next));
    }

    if (m_sortMode == MessageView::SortOldestTop) {
        if (m_prev.isValid()) {
            addAdiumStatusMessage(nextConversation);
        }
        qSort(m_events.begin(), m_events.end(), logMessageOlderThan);
    } else if (m_sortMode == MessageView::SortNewestTop) {
        if (m_next.isValid()) {
            addAdiumStatusMessage(prevConversation);
        }
        qSort(m_events.begin(), m_events.end(), logMessageNewerThan);
    }

    if (m_events.isEmpty()) {
        showInfoMessage(i18n("There are no logs for this day"));
    }

    while (!m_events.isEmpty()) {
        const KTp::LogMessage msg = m_events.takeFirst();
        KTp::MessageContext ctx(m_account, Tp::TextChannelPtr());
        KTp::Message message = KTp::MessageProcessor::instance()->processIncomingMessage(msg, ctx);
        addMessage(message);
    }

    if (m_sortMode == MessageView::SortOldestTop && m_next.isValid()) {
        addAdiumStatusMessage(prevConversation);
    } else if (m_sortMode == MessageView::SortNewestTop && m_prev.isValid()) {
        addAdiumStatusMessage(nextConversation);
    }

    /* Can't highlight the text directly, we need to wait for the JavaScript in
     * AdiumThemeView to include the log messages into DOM. */
    QTimer::singleShot(100, this, SLOT(doHighlightText()));
}

void MessageView::onLinkClicked(const QUrl &link)
{
    // Don't emit the signal directly, KWebView does not like when we reload the
    // page from an event handler (and then chain up) and we can't guarantee
    // that everyone will use QueuedConnection when connecting to
    // conversationSwitchRequested() slot

    if (link.fragment() == QLatin1String("x-nextConversation")) {
        // Q_EMIT conversationSwitchRequested(m_next)
        QMetaObject::invokeMethod(this, "conversationSwitchRequested", Qt::QueuedConnection,
            Q_ARG(QDate, m_next));
        return;
    }

    if (link.fragment() == QLatin1String("x-prevConversation")) {
        // Q_EMIT conversationSwitchRequested(m_prev)
        QMetaObject::invokeMethod(this, "conversationSwitchRequested", Qt::QueuedConnection,
            Q_ARG(QDate, m_prev));
        return;
    }

    AdiumThemeView::onLinkClicked(link);
}

void MessageView::loadSettings()
{
    const KConfig config(QLatin1String("ktelepathyrc"));
    const KConfigGroup group = config.group("LogViewer");
    m_sortMode = static_cast<SortMode>(group.readEntry("SortMode", static_cast<int>(SortOldestTop)));
}

void MessageView::reloadTheme()
{
    loadSettings();
    loadLog(m_account, m_entity, m_contact, m_date, qMakePair(m_prev, m_next));
}

void MessageView::doHighlightText()
{
    findText(QString());
    if (!m_highlightedText.isEmpty()) {
        findText(m_highlightedText, QWebPage::HighlightAllOccurrences |
                                    QWebPage::FindWrapsAroundDocument);
    }
}
