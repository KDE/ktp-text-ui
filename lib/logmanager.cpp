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

#include <KTp/message-processor.h>

#include <KDebug>

#include <TelepathyLoggerQt4/Init>
#include <TelepathyLoggerQt4/LogWalker>
#include <TelepathyLoggerQt4/Entity>
#include <TelepathyLoggerQt4/PendingEvents>
#include <TelepathyLoggerQt4/Event>
#include <TelepathyLoggerQt4/TextEvent>
#include <TelepathyLoggerQt4/LogManager>

#include <TelepathyQt/Types>
#include <TelepathyQt/AvatarData>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/ReceivedMessage>

LogManager::LogManager(QObject *parent)
    : QObject(parent),
    m_scrolbackLength(10)
{
    Tpl::init();

    m_logManager = Tpl::LogManager::instance();
    if (m_logManager.isNull()) {
        qWarning() << "LogManager not found";
        Q_ASSERT(false);
    }
}

LogManager::~LogManager()
{

}

bool LogManager::exists() const
{
    if (m_account.isNull() || m_textChannel.isNull() ) {
        return false;
    }

    Tpl::EntityPtr contactEntity;
    if (m_textChannel->targetHandleType() == Tp::HandleTypeContact) {
        contactEntity = Tpl::Entity::create(m_textChannel->targetContact()->id().toLatin1().data(),
                                            Tpl::EntityTypeContact, NULL, NULL);
    } else if (m_textChannel->targetHandleType() == Tp::HandleTypeRoom) {
        contactEntity = Tpl::Entity::create(m_textChannel->targetId().toLatin1().data(),
                                            Tpl::EntityTypeRoom, NULL, NULL);
    } else {
        return false;
    }

    return m_logManager->exists(m_account, contactEntity, Tpl::EventTypeMaskText);
}

void LogManager::setTextChannel(const Tp::AccountPtr &account, const Tp::TextChannelPtr &textChannel)
{
    m_textChannel = textChannel;
    m_account = account;
}

void LogManager::setScrollbackLength(int n)
{
    m_scrolbackLength = n;
}

int LogManager::scrollbackLength() const
{
    return m_scrolbackLength;
}

void LogManager::fetchScrollback()
{
    fetchHistory(m_scrolbackLength);
}

void LogManager::fetchHistory(int n)
{
    if (n > 0 && !m_account.isNull() && !m_textChannel.isNull()) {
        Tpl::EntityPtr contactEntity;
        if (m_textChannel->targetHandleType() == Tp::HandleTypeContact) {
            contactEntity = Tpl::Entity::create(m_textChannel->targetContact()->id().toLatin1().data(),
                                                Tpl::EntityTypeContact, NULL, NULL);
        } else if (m_textChannel->targetHandleType() == Tp::HandleTypeRoom) {
            contactEntity = Tpl::Entity::create(m_textChannel->targetId().toLatin1().data(),
                                                Tpl::EntityTypeRoom, NULL, NULL);
        }

        if (!contactEntity.isNull()) {
            Tpl::LogWalkerPtr walker = m_logManager->queryWalkFilteredEvents(
                m_account, contactEntity, Tpl::EventTypeMaskText, 0, 0);
            Tpl::PendingEvents *events = walker->queryEvents(n);
            connect(events, SIGNAL(finished(Tpl::PendingOperation*)),
                    this, SLOT(onEventsFinished(Tpl::PendingOperation*)));
            return;
        }
    }

    //in all other cases finish immediately.
    QList<KTp::Message> messages;
    Q_EMIT fetched(messages);
}

bool operator<(const Tpl::EventPtr &e1, const Tpl::EventPtr &e2)
{
    return e1->timestamp() < e2->timestamp();
}

void LogManager::onEventsFinished(Tpl::PendingOperation *po)
{
    Tpl::PendingEvents *pe = (Tpl::PendingEvents*) po;

    if (pe->isError()) {
        qWarning() << "error in PendingEvents" << pe->errorMessage();
        return;
    }

    QStringList queuedMessageTokens;
    if (!m_textChannel.isNull()) {
        Q_FOREACH(const Tp::ReceivedMessage &message, m_textChannel->messageQueue()) {
            queuedMessageTokens.append(message.messageToken());
        }
    }
    kDebug() << "queuedMessageTokens" << queuedMessageTokens;


    // get last n (m_fetchLast) messages that are not queued
    QList<Tpl::EventPtr> allEvents = pe->events();

    // See https://bugs.kde.org/show_bug.cgi?id=317866
    // Uses the operator< overload above
    qSort(allEvents);

    QList<KTp::Message> messages;
    Q_FOREACH (const Tpl::EventPtr &event, allEvents) {
        const Tpl::TextEventPtr textEvent = event.dynamicCast<Tpl::TextEvent>();
        if (!textEvent.isNull()) {
            if (!queuedMessageTokens.contains(textEvent->messageToken())) {
                const KTp::Message message = KTp::MessageProcessor::instance()->processIncomingMessage(textEvent, m_account, m_textChannel);
                messages.append(message);
            }
        }
    }

    kDebug() << "emit all messages" << messages.count();
    Q_EMIT fetched(messages);
}
