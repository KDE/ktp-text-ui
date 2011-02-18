/*
    Copyright (C) 2010  David Edmundson <kde@davidedmundson.co.uk>
    Copyright (C) 2011  Dominik Schmidt <dev@dominik-schmidt.de>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include "chatwidget.h"

#include <KXmlGuiWindow>
#include <KTabWidget>

class KIcon;

class ChatWindow : public KXmlGuiWindow
{
Q_OBJECT

public:
    ChatWindow();
    virtual ~ChatWindow();

    void addTab(ChatWidget *chatWidget);
    void removeTab(ChatWidget *chatWidget);

public slots:
    void onCurrentIndexChanged(int index);
    void updateTabText(const QString &newTitle);
    void updateTabIcon(const KIcon &newIcon);
    void onUserTypingChanged(bool isTyping);

protected slots:
    void showSettingsDialog();
    void showNotificationsDialog();

private:
    KTabWidget *m_tabWidget;
};

#endif // CHATWINDOW_H
