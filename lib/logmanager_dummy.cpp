/*
    Copyright (C) 2013 Daniel Vrátil <dvratil@redhat.com>

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

#include <TelepathyQt/Types>

class LogManager::Private
{
  public:
    Private(): scrollbackLength(10)
    {
    }

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
    return false;
}

void LogManager::setTextChannel(const Tp::AccountPtr &account, const Tp::TextChannelPtr &textChannel)
{
    Q_UNUSED(account);
    Q_UNUSED(textChannel);
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
    QMetaObject::invokeMethod(this, "fetched", Qt::QueuedConnection,
                              Q_ARG(QList<KTp::Message>, QList<KTp::Message>()));
}

void LogManager::fetchHistory(int n)
{
    Q_UNUSED(n);

    fetchScrollback();
}

void LogManager::onEventsFinished(Tpl::PendingOperation *po)
{
    Q_UNUSED(po);
}

#include "moc_logmanager.cpp"
