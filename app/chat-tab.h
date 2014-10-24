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

#ifndef CHATTAB_H
#define CHATTAB_H

#include "chat-widget.h"

#include <QtCore/QString>
#include <QWidget>
#include <KIcon>
#include <KColorScheme>

class ChatWidgetPrivate;
class ChatWindow;
class QShowEvent;

class ChatTab : public ChatWidget
{
    Q_OBJECT

public:
    explicit ChatTab(const Tp::TextChannelPtr &channel, const Tp::AccountPtr &account, QWidget *parent = 0);
    virtual ~ChatTab();

    /** set's a new chatWindow for the tab
     * @param window chatWindow to set tab to
     */
    void setChatWindow(ChatWindow* window);

    /** returns currently set chatWindow */
    ChatWindow* chatWindow() const;

public Q_SLOTS:
    void showOnTop();

Q_SIGNALS:
    void aboutToClose(ChatTab *tab);

private Q_SLOTS:
    /** connect account's connection status.
     * This re-enables open chats if user goes offline and then back online */
    void onConnectionStatusChanged(Tp::ConnectionStatus);

private:
    /** pointer to chat window the tab is nested in */
    ChatWindow *m_chatWindow;
};

#endif // CHATWIDGET_H
