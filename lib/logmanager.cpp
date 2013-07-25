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

class LogManager::Private
{
  public:
    Private(): scrollbackLength(10)
    {
    }

    Tp::AccountPtr account;
    Tp::TextChannelPtr textChannel;
    Tpl::EntityPtr contactEntity;
    Tpl::LogManagerPtr logManager;
    int scrollbackLength;
};

LogManager::LogManager(QObject *parent)
    : QObject(parent),
    d(new Private)
{
    Tpl::init();

    d->logManager = Tpl::LogManager::instance();
    if (d->logManager.isNull()) {
        qWarning() << "LogManager not found";
        Q_ASSERT(false);
    }
}

LogManager::~LogManager()
{
    delete d;
}

bool LogManager::exists() const
{
    if (d->account.isNull() || d->textChannel.isNull() ) {
        return false;
    }

    Tpl::EntityPtr contactEntity;
    if (d->textChannel->targetHandleType() == Tp::HandleTypeContact) {
        contactEntity = Tpl::Entity::create(d->textChannel->targetContact()->id().toLatin1().data(),
                                            Tpl::EntityTypeContact, NULL, NULL);
    } else if (d->textChannel->targetHandleType() == Tp::HandleTypeRoom) {
        contactEntity = Tpl::Entity::create(d->textChannel->targetId().toLatin1().data(),
                                            Tpl::EntityTypeRoom, NULL, NULL);
    } else {
        return false;
    }

    return d->logManager->exists(d->account, contactEntity, Tpl::EventTypeMaskText);
}

void LogManager::setTextChannel(const Tp::AccountPtr &account, const Tp::TextChannelPtr &textChannel)
{
    d->textChannel = textChannel;
    d->account = account;
}

void LogManager::setScrollbackLength(int n)
{
    d->scrollbackLength = n;
}

int LogManager::scrollbackLength() const
{
    return d->scrollbackLength;
}

void LogManager::fetchScrollback()
{
    fetchHistory(d->scrollbackLength);
}

void LogManager::fetchHistory(int n)
{
    if (n > 0 && !d->account.isNull() && !d->textChannel.isNull()) {
        Tpl::EntityPtr contactEntity;
        if (d->textChannel->targetHandleType() == Tp::HandleTypeContact) {
            contactEntity = Tpl::Entity::create(d->textChannel->targetContact()->id().toLatin1().data(),
                                                Tpl::EntityTypeContact, NULL, NULL);
        } else if (d->textChannel->targetHandleType() == Tp::HandleTypeRoom) {
            contactEntity = Tpl::Entity::create(d->textChannel->targetId().toLatin1().data(),
                                                Tpl::EntityTypeRoom, NULL, NULL);
        }

        if (!contactEntity.isNull()) {
            Tpl::LogWalkerPtr walker = d->logManager->queryWalkFilteredEvents(
                d->account, contactEntity, Tpl::EventTypeMaskText, 0, 0);
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
    if (!d->textChannel.isNull()) {
        Q_FOREACH(const Tp::ReceivedMessage &message, d->textChannel->messageQueue()) {
            queuedMessageTokens.append(message.messageToken());
        }
    }
    kDebug() << "queuedMessageTokens" << queuedMessageTokens;


    // get last n (d->fetchLast) messages that are not queued
    QList<Tpl::EventPtr> allEvents = pe->events();

    // See https://bugs.kde.org/show_bug.cgi?id=317866
    // Uses the operator< overload above
    qSort(allEvents);

    QList<KTp::Message> messages;
    Q_FOREACH (const Tpl::EventPtr &event, allEvents) {
        const Tpl::TextEventPtr textEvent = event.dynamicCast<Tpl::TextEvent>();
        if (!textEvent.isNull()) {
            if (!queuedMessageTokens.contains(textEvent->messageToken())) {
                const KTp::Message message = KTp::MessageProcessor::instance()->processIncomingMessage(textEvent, d->account, d->textChannel);
                messages.append(message);
            }
        }
    }

    kDebug() << "emit all messages" << messages.count();
    Q_EMIT fetched(messages);
}
