/*
    Copyright (C) 2011  Dominik Schmidt <kde@dominik-schmidt.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "logmanager.h"

#include "adium-theme-content-info.h"

#include <KDebug>


#ifdef TELEPATHY_LOGGER_QT4_FOUND
#include <TelepathyLoggerQt4/Init>
#include <TelepathyLoggerQt4/Entity>
#include <TelepathyLoggerQt4/PendingDates>
#include <TelepathyLoggerQt4/PendingEvents>
#include <TelepathyLoggerQt4/Event>
#include <TelepathyLoggerQt4/TextEvent>
#include <TelepathyLoggerQt4/CallEvent>
#include <TelepathyLoggerQt4/LogManager>

#include <glib-object.h>
#include <QGlib/Init>
#endif

#include <TelepathyQt/Types>
#include <TelepathyQt/AvatarData>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/ReceivedMessage>

LogManager::LogManager(QObject *parent)
    : QObject(parent),
    m_fetchAmount(10)
{
#ifdef TELEPATHY_LOGGER_QT4_FOUND
    g_type_init();
    QGlib::init();
    Tpl::init();

    m_logManager = Tpl::LogManager::instance();
    if (m_logManager.isNull()) {
        qWarning() << "LogManager not found";
        Q_ASSERT(false);
    }

#else
    kWarning() << "text-ui was built without log support";
#endif
}

LogManager::~LogManager()
{

}

bool LogManager::exists() const
{
#ifdef TELEPATHY_LOGGER_QT4_FOUND
    return m_logManager->exists(m_account, m_contactEntity, Tpl::EventTypeMaskText);
#else
    return false;
#endif
}

void LogManager::setTextChannel(const Tp::AccountPtr &account, const Tp::TextChannelPtr &textChannel)
{
    m_textChannel = textChannel;
    m_account = account;
}

void LogManager::setFetchAmount(int n)
{
    m_fetchAmount = n;
}

void LogManager::fetchLast()
{
    kDebug();
#ifdef TELEPATHY_LOGGER_QT4_FOUND
    if (!m_account.isNull() && !m_textChannel.isNull() && m_textChannel->targetHandleType() == Tp::HandleTypeContact) {
        Tpl::EntityPtr contactEntity = Tpl::Entity::create(m_textChannel->targetContact()->id().toLatin1().data(),
                                                Tpl::EntityTypeContact,
                                                NULL,
                                                NULL);

        Tpl::PendingDates* dates = m_logManager->queryDates( m_account, contactEntity, Tpl::EventTypeMaskText);
        connect(dates, SIGNAL(finished(Tpl::PendingOperation*)), SLOT(onDatesFinished(Tpl::PendingOperation*)));
        return;
    }
    //in all other cases finish immediately.
#else
    QList<AdiumThemeContentInfo> messages;
    Q_EMIT fetched(messages);
#endif

}

#ifdef TELEPATHY_LOGGER_QT4_FOUND
void LogManager::onDatesFinished(Tpl::PendingOperation* po)
{
    Tpl::PendingDates *pd = (Tpl::PendingDates*) po;

    if (pd->isError()) {
        qWarning() << "error in PendingDates:" << pd->errorMessage();
        return;
    }

    QList<QDate> dates = pd->dates();

    if( !dates.isEmpty() ) {
        QDate date = dates.last();

        kDebug() << pd->account()->uniqueIdentifier() << pd->entity()->identifier() << dates;

        Tpl::PendingEvents* events = m_logManager->queryEvents( pd->account(), pd->entity(), Tpl::EventTypeMaskAny, date);
        connect(events, SIGNAL(finished(Tpl::PendingOperation*)), SLOT(onEventsFinished(Tpl::PendingOperation*)));
    } else {
        QList<AdiumThemeContentInfo> messages;
        Q_EMIT fetched(messages);
    }
}

void LogManager::onEventsFinished(Tpl::PendingOperation* po)
{
    Tpl::PendingEvents *pe = (Tpl::PendingEvents*) po;

    if (pe->isError()) {
        qWarning() << "error in PendingEvents" << pe->errorMessage();
        return;
    }

    QStringList queuedMessageTokens;
    if(!m_textChannel.isNull()) {
        Q_FOREACH(const Tp::ReceivedMessage &message, m_textChannel->messageQueue()) {
            queuedMessageTokens.append(message.messageToken());
        }
    }
    kDebug() << "queuedMessageTokens" << queuedMessageTokens;


    // get last n (m_fetchLast) messages that are not queued
    QList<Tpl::EventPtr> allEvents = pe->events();
    QList<Tpl::TextEventPtr> events;
    QList<Tpl::EventPtr>::iterator i = allEvents.end();
    while (i-- != allEvents.begin() && (events.count() <= m_fetchAmount)) {
        Tpl::TextEventPtr textEvent = (*i).dynamicCast<Tpl::TextEvent>();
        if(!textEvent.isNull()) {
            if(!queuedMessageTokens.contains(textEvent->messageToken())) {
                events.prepend( textEvent );
            }
        }
    }


    QList<AdiumThemeContentInfo> messages;
    Q_FOREACH(const Tpl::TextEventPtr& event, events) {
        AdiumThemeMessageInfo::MessageType type;
        QString iconPath;
        Tp::ContactPtr contact;
        if(event->sender()->identifier() == m_account->normalizedName()) {
            type = AdiumThemeMessageInfo::HistoryLocalToRemote;
            if (m_account->connection()) {
                contact = m_account->connection()->selfContact();
            }
        } else {
            type = AdiumThemeMessageInfo::HistoryRemoteToLocal;
            contact = m_textChannel->targetContact();
        }
        iconPath = contact->avatarData().fileName;

        AdiumThemeContentInfo message(type);
        message.setMessage(event->message());
        message.setService(m_account->serviceName());
        message.setSenderDisplayName(event->sender()->alias());
        message.setSenderScreenName(event->sender()->alias());
        message.setTime(event->timestamp());
        message.setUserIconPath(iconPath);
        kDebug()    << event->timestamp()
                    << "from" << event->sender()->identifier()
                    << "to" << event->receiver()->identifier()
                    << event->message();

        messages.append(message);
    }

    kDebug() << "emit all messages" << messages.count();
    Q_EMIT fetched(messages);
}
#endif
