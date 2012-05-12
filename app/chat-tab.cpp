/***************************************************************************
 *   Copyright (C) 2011 by Dominik Schmidt <kde@dominik-schmidt.de>        *
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

#include "chat-tab.h"

#include "chat-widget.h"

#include "defines.h"

#include <KTabWidget>
#include <KDebug>
#include <QStackedWidget>
#include <KDE/KColorScheme>
#include <KDE/KWindowSystem>

#include <TelepathyQt/Account>
#include <TelepathyQt/TextChannel>
#include "chat-window.h"

ChatTab::ChatTab(const Tp::TextChannelPtr& channel, const Tp::AccountPtr& account, QWidget* parent)
    : ChatWidget(channel, account, parent)
    , m_chatWindow(0)
{
    connect(this, SIGNAL(notificationClicked()), SLOT(showOnTop()));

    // connect account connection status
    connect(account.data(), SIGNAL(connectionStatusChanged(Tp::ConnectionStatus)), this, SLOT(onConnectionStatusChanged(Tp::ConnectionStatus)));
}

ChatTab::~ChatTab()
{
}

void ChatTab::setChatWindow(ChatWindow* window)
{
    // remove the tab from current ChatWindow
    if (m_chatWindow) {
        m_chatWindow->removeTab(this);
    }

    m_chatWindow = window;

    // set tab in new chatWindow
    if (m_chatWindow) {
        m_chatWindow->addTab(this);
    }
}

ChatWindow* ChatTab::chatWindow() const
{
    return m_chatWindow;
}

void ChatTab::showOnTop()
{
    kDebug() << "Show this widget on top" << title();
    if (m_chatWindow) {
        m_chatWindow->focusChat(this);
    } else {
        kError() << "Attempting to focus chatTab without chatWindow being set!";
    }

    m_chatWindow->raise();
    KWindowSystem::forceActiveWindow(m_chatWindow->winId());
}

void ChatTab::onConnectionStatusChanged(Tp::ConnectionStatus status)
{
    // request a new text channel for the chat

    Tp::ChannelRequestHints hints;
    hints.setHint(QLatin1String("org.kde.telepathy"),QLatin1String("suppressWindowRaise"), QVariant(true));

    if (status == Tp::ConnectionStatusConnected) {
        if (textChannel()->targetHandleType() == Tp::HandleTypeContact) {
            account()->ensureTextChat(textChannel()->targetId(),
                                      QDateTime::currentDateTime(),
                                      QLatin1String(KTP_TEXTUI_CLIENT_PATH),
                                      hints);
        } else if (textChannel()->targetHandleType() == Tp::HandleTypeRoom) {

            account()->ensureTextChatroom(textChannel()->targetId(),
                                          QDateTime::currentDateTime(),
                                          QLatin1String(KTP_TEXTUI_CLIENT_PATH),
                                          hints);
        }
    }
}
