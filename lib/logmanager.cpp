/*
    Copyright (C) 2011  Dominik Schmidt <kde@dominik-schmidt.de>
    Copyright (C) 2013  Daniel Vrátil <dvratil@redhat.com>

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
#include <KTp/Logger/log-entity.h>
#include <KTp/Logger/log-manager.h>
#include <KTp/Logger/pending-logger-dates.h>
#include <KTp/Logger/pending-logger-logs.h>

#include <KDebug>

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
    KTp::LogEntity contactEntity;
    int scrollbackLength;
};

LogManager::LogManager(QObject *parent)
    : QObject(parent),
    d(new Private)
{
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

    return KTp::LogManager::instance()->logsExist(d->account, d->contactEntity);
}

void LogManager::setTextChannel(const Tp::AccountPtr &account, const Tp::TextChannelPtr &textChannel)
{
    d->textChannel = textChannel;
    d->account = account;

    if (d->account.isNull() || d->textChannel.isNull()) {
        return;
    }

    KTp::LogEntity contactEntity;
    if (d->textChannel->targetHandleType() == Tp::HandleTypeContact) {
        d->contactEntity = KTp::LogEntity(KTp::LogEntity::EntityTypeContact,
                                       d->textChannel->targetContact()->id(),
                                       d->textChannel->targetContact()->alias());
    } else if (d->textChannel->targetHandleType() == Tp::HandleTypeRoom) {
        d->contactEntity = KTp::LogEntity(KTp::LogEntity::EntityTypeRoom,
                                       d->textChannel->targetId());
    }
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
        if (d->contactEntity.isValid()) {
            KTp::LogManager *manager = KTp::LogManager::instance();
            KTp::PendingLoggerDates *dates = manager->queryDates(d->account, d->contactEntity);
            connect(dates, SIGNAL(finished(KTp::PendingLoggerOperation*)),
                    this, SLOT(onDatesFinished(KTp::PendingLoggerOperation*)));
            return;
        }
    }

    //in all other cases finish immediately.
    QList<KTp::Message> messages;
    Q_EMIT fetched(messages);
}

void LogManager::onDatesFinished(KTp::PendingLoggerOperation* po)
{
    KTp::PendingLoggerDates *datesOp = qobject_cast<KTp::PendingLoggerDates*>(po);
    if (datesOp->hasError()) {
        kWarning() << "Failed to fetch dates:" << datesOp->error();
        Q_EMIT fetched(QList<KTp::Message>());
        return;
    }

    const QList<QDate> dates = datesOp->dates();
    if (dates.isEmpty()) {
        Q_EMIT fetched(QList<KTp::Message>());
        return;
    }

    KTp::LogManager *manager = KTp::LogManager::instance();
    KTp::PendingLoggerLogs *logs = manager->queryLogs(datesOp->account(), datesOp->entity(),
                                                      dates.last());
    connect(logs, SIGNAL(finished(KTp::PendingLoggerOperation*)),
            this, SLOT(onEventsFinished(KTp::PendingLoggerOperation*)));
}

void LogManager::onEventsFinished(KTp::PendingLoggerOperation *op)
{
    KTp::PendingLoggerLogs *logsOp = qobject_cast<KTp::PendingLoggerLogs*>(op);
    if (logsOp->hasError()) {
        kWarning() << "Failed to fetch events:" << logsOp->error();
        Q_EMIT fetched(QList<KTp::Message>());
        return;
    }

    // get last n (d->fetchLast) messages that are not queued
    const QList<KTp::LogMessage> allMessages = logsOp->logs();
    QList<KTp::Message> messages;
    const KTp::MessageContext ctx(d->account, d->textChannel);
    for (int i = 0; i < d->scrollbackLength && i < allMessages.count(); ++i) {
        const KTp::LogMessage message = allMessages[i];
        messages << KTp::MessageProcessor::instance()->processIncomingMessage(message, ctx);
    }

    kDebug() << "emit all messages" << messages.count();
    Q_EMIT fetched(messages);
}
