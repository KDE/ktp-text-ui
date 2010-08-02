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

#include "mainwindow.h"
#include "chatwindow.h"

inline ChannelClassList channelClassList()
{
    ChannelClassList filters;
    QMap<QString, QDBusVariant> filter;
    filter.insert(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".ChannelType"),
                  QDBusVariant(TELEPATHY_INTERFACE_CHANNEL_TYPE_TEXT));
    filters.append(filter);
    return filters;
}


MainWindow::MainWindow() :
        KTabWidget(),
        AbstractClientHandler(channelClassList())
{
}


void MainWindow::handleChannels(const MethodInvocationContextPtr<> &context,
                                       const AccountPtr & account,
                                       const ConnectionPtr & connection,
                                       const QList< ChannelPtr > & channels,
                                       const QList< ChannelRequestPtr > & ,
                                       const QDateTime & ,
                                       const QVariantMap&
                                      )
{
    ChatConnection* chatConnection = new ChatConnection(this, account, connection, channels);
    ChatWindow* newWindow = new ChatWindow(chatConnection, this);

    addTab(newWindow,"test");
    resize(newWindow->sizeHint());// FUDGE

    context->setFinished();
}

