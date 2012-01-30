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

void MessageView::loadLog(const Tp::AccountPtr &account, const Tpl::EntityPtr &entity, const QDate &date)
{
    m_account = account;
    m_entity = entity;
    m_date = date;

    //FIXME check entity type, set as appropriately.
    load(AdiumThemeView::SingleUserChat);


    AdiumThemeHeaderInfo headerInfo;
    headerInfo.setDestinationDisplayName(m_entity->alias());
    headerInfo.setChatName(m_entity->alias());
    //  TODO set up other headerInfo here.
    initialise(headerInfo);
}

void MessageView::onLoadFinished()
{
    //load stuff here.
    Tpl::LogManagerPtr logManager = Tpl::LogManager::instance();
    Tpl::PendingEvents *pendingEvents  = logManager->queryEvents(m_account, m_entity, Tpl::EventTypeMaskText, m_date);
    connect(pendingEvents, SIGNAL(finished(Tpl::PendingOperation*)), SLOT(onEventsLoaded(Tpl::PendingOperation*)));
}

void MessageView::onEventsLoaded(Tpl::PendingOperation *po)
{
    Tpl::PendingEvents *pe = qobject_cast<Tpl::PendingEvents*>(po);

    QList<AdiumThemeContentInfo> messages;

    Q_FOREACH(const Tpl::EventPtr &event, pe->events()) {
        const Tpl::TextEventPtr textEvent(event.staticCast<Tpl::TextEvent>());

        AdiumThemeMessageInfo::MessageType type;
        QString iconPath;

        if(event->sender()->identifier() == m_account->normalizedName()) {
            type = AdiumThemeMessageInfo::HistoryLocalToRemote;
        } else {
            type = AdiumThemeMessageInfo::HistoryRemoteToLocal;
        }


        AdiumThemeContentInfo message(type);
        message.setMessage(textEvent->message());
        message.setService(m_account->serviceName());
        message.setSenderDisplayName(textEvent->sender()->alias());
        message.setSenderScreenName(textEvent->sender()->identifier());
        message.setTime(textEvent->timestamp());
        message.setUserIconPath(iconPath);
        kDebug()    << textEvent->timestamp()
                    << "from" << textEvent->sender()->identifier()
                    << "to" << textEvent->receiver()->identifier()
                    << textEvent->message();

        messages.append(message);

        addContentMessage(message);
    }
}
