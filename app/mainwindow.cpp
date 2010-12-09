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

#include <KColorScheme>

#include <TelepathyQt4/ChannelClassSpecList>


inline ChannelClassList channelClassList()
{
    ChannelClassList filters;
    QMap<QString, QDBusVariant> filter;
    filter.insert(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".ChannelType"),
                  QDBusVariant(TELEPATHY_INTERFACE_CHANNEL_TYPE_TEXT));
    filter.insert(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".TargetHandleType"),
                  QDBusVariant((uint) Tp::HandleTypeContact));
    filters.append(filter);

    filter.clear();
    filter.insert(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".ChannelType"),
                  QDBusVariant(TELEPATHY_INTERFACE_CHANNEL_TYPE_TEXT));
    filter.insert(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".TargetHandleType"),
                  QDBusVariant((uint) Tp::HandleTypeRoom));
    filters.append(filter);
    return filters;
}


MainWindow::MainWindow() :
        KTabWidget(),
        AbstractClientHandler(ChannelClassSpecList())
{
    setTabReorderingEnabled(true);
    connect(this, SIGNAL(currentChanged(int)), SLOT(onCurrentIndexChanged(int)));
}

void MainWindow::handleChannels(const MethodInvocationContextPtr<> &context,
        const AccountPtr &account,
        const ConnectionPtr &connection,
        const QList<ChannelPtr> &channels,
        const QList<ChannelRequestPtr> &requestsSatisfied,
        const QDateTime &userActionTime,
        const AbstractClientHandler::HandlerInfo &handlerInfo)
{
    ChatConnection* chatConnection = new ChatConnection(this, account, connection, channels);
    ChatWindow* newWindow = new ChatWindow(chatConnection, this);

    addTab(newWindow,KIcon("user-online"),"");

    connect(newWindow, SIGNAL(titleChanged(QString)), SLOT(updateTabText(QString)));
    connect(newWindow,SIGNAL(iconChanged(KIcon)), SLOT(updateTabIcon(KIcon)));
    connect(newWindow, SIGNAL(userTypingChanged(bool)), SLOT(onUserTypingChanged(bool)));

    resize(newWindow->sizeHint() - QSize(50, 50));// FUDGE

    context->setFinished();
}

void MainWindow::updateTabText(QString newTitle)
{
    //find out which widget made the call, and update the correct tab.
    QWidget* sender = qobject_cast<QWidget*>(QObject::sender());
    if (sender) {
        int tabIndexToChange = indexOf(sender);
        setTabText(tabIndexToChange, newTitle);

        if (tabIndexToChange == currentIndex())
        {
            onCurrentIndexChanged(tabIndexToChange);
        }
    }
}

void MainWindow::updateTabIcon(KIcon newIcon)
{
    //find out which widget made the call, and update the correct tab.
    QWidget* sender = qobject_cast<QWidget*>(QObject::sender());
    if (sender) {
        int tabIndexToChange = indexOf(sender);
        setTabIcon(tabIndexToChange, newIcon);
    }
}


void MainWindow::onCurrentIndexChanged(int index)
{
    ChatWindow* chat = qobject_cast<ChatWindow*>(widget(index));
    setWindowTitle(chat->title());
}

void MainWindow::onUserTypingChanged(bool isTyping)
{
    QWidget* sender = qobject_cast<QWidget*>(QObject::sender());
    if (sender)
    {
        KColorScheme scheme(QPalette::Active, KColorScheme::Window);
        int tabIndex = indexOf(sender);
        if (isTyping)
        {
            this->setTabTextColor(tabIndex, scheme.foreground(KColorScheme::PositiveText).color() );
        }
        else
        {
            this->setTabTextColor(tabIndex, scheme.foreground(KColorScheme::NormalText).color() );
        }
    }
}
