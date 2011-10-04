/*
    <one line to give the library's name and an idea of what it does.>
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

#ifdef TELEPATHY_LOGGER_QT4_FOUND
#include <TelepathyLoggerQt4/LogManager>
#include <TelepathyLoggerQt4/Entity>
#endif

#include <TelepathyQt4/Types>
#include <TelepathyQt4/Account>
#include <TelepathyQt4/Contact>


namespace Tpl {
    class PendingOperation;
}


class LogManager : public QObject
{
    Q_OBJECT

public:
    explicit LogManager(const Tp::AccountPtr &account, const Tp::ContactPtr &contact, QObject *parent = 0);
    virtual ~LogManager();

    bool exists() const;

    void setTextChannel(const Tp::TextChannelPtr &textChannel);
    void setFetchAmount(int n);
    void fetchLast();

Q_SIGNALS:
    void fetched(const QList<AdiumThemeContentInfo> &messages);

#ifdef TELEPATHY_LOGGER_QT4_FOUND
private Q_SLOTS:
    void onDatesFinished(Tpl::PendingOperation* po);
    void onEventsFinished(Tpl::PendingOperation* po);
#endif

private:
    Tp::AccountPtr m_account;
    Tp::ContactPtr m_contact;
    Tp::TextChannelPtr m_textChannel;
#ifdef TELEPATHY_LOGGER_QT4_FOUND
    Tpl::EntityPtr m_contactEntity;
    Tpl::LogManagerPtr m_logManager;
#endif


    int m_fetchAmount;
};

#endif // LOGMANAGER_H
