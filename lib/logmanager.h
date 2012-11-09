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


#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include "adium-theme-content-info.h"

#include <TelepathyLoggerQt4/LogManager>
#include <TelepathyLoggerQt4/Entity>

#include <TelepathyQt/Types>
#include <TelepathyQt/Account>
#include <TelepathyQt/Contact>


namespace Tpl {
    class PendingOperation;
}


class LogManager : public QObject
{
    Q_OBJECT

public:
    explicit LogManager(QObject *parent = 0);
    virtual ~LogManager();

    bool exists() const;

    void setTextChannel(const Tp::AccountPtr &account, const Tp::TextChannelPtr &textChannel);
    void setFetchAmount(int n);
    void fetchLast();

Q_SIGNALS:
    void fetched(const QList<AdiumThemeContentInfo> &messages);

private Q_SLOTS:
    void onDatesFinished(Tpl::PendingOperation *po);
    void onEventsFinished(Tpl::PendingOperation *po);

private:
    Tp::AccountPtr m_account;
    Tp::TextChannelPtr m_textChannel;
    Tpl::EntityPtr m_contactEntity;
    Tpl::LogManagerPtr m_logManager;

    int m_fetchAmount;
};

#endif // LOGMANAGER_H
