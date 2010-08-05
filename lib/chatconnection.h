/***************************************************************************
 *   Copyright (C) 2010 by David Edmundson <kde@davidedmundson.co.uk>      *
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

#ifndef CHATCONNECTION_H
#define CHATCONNECTION_H

#include <QObject>
#include <TelepathyQt4/Account>
#include <TelepathyQt4/Connection>
#include <TelepathyQt4/TextChannel>

using namespace Tp;

class ChatConnection : public QObject
{
    Q_OBJECT
public:
    explicit ChatConnection(QObject *parent, const AccountPtr, const ConnectionPtr,  QList<ChannelPtr>);

    const AccountPtr account()
    {
        return m_account;
    };
    const ConnectionPtr connection()
    {
        return m_connection;
    };
    const TextChannelPtr channel()
    {
        return m_channel;
    };


signals:
    void channelReadyStateChanged(bool newState);

public slots:



private:
    AccountPtr m_account;
    ConnectionPtr m_connection;
    TextChannelPtr m_channel;

private slots:
    void onChannelReady(Tp::PendingOperation*);
    void onPendingContactsReady(Tp::PendingOperation*);
};

#endif // CHATCONNECTION_H


