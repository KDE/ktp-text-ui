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

#include "chat-widget.h"

#include <KXmlGuiWindow>
#include <KTabWidget>

class KIcon;
class ChatTab;

class ChatWindow : public KXmlGuiWindow
{
Q_OBJECT

public:
    ChatWindow();
    virtual ~ChatWindow();

    /**
     * starts a new chat with the textChannelPtr given only if the
     * chat doesn't already exist
     * @param incomingTextChannel new text channel
     */
    void startChat(Tp::TextChannelPtr incomingTextChannel);
    void removeTab(ChatTab *chatWidget);
    void setTabText(int index, const QString &newTitle);
    void setTabIcon(int index, const KIcon &newIcon);
    void setTabTextColor(int index,const QColor &color);

public slots:
    void removeTab(QWidget *chatWidget);

private slots:
    void onCurrentIndexChanged(int index);
    void onEnableSearchActions(bool enable);    /** enables/disables menu search actions */
    void onFindNextText();                      /** go to next text the user is searching for */
    void onFindPreviousText();                  /** go to previous text the user is searching for */
    void onSearchActionToggled();               /** toggle search bar visibility */
    void onTabStateChanged();
    void onTabTextChanged(const QString &newTitle);
    void onTabIconChanged(const KIcon &newIcon);

protected slots:
    void showSettingsDialog();
    void showNotificationsDialog();

private:
    /** creats a new chat and adds it to the tab widget
     * @param channelPtr pointer to textChannel to use
     */
    void createNewChat(Tp::TextChannelPtr channelPtr);

    /** connects the neccessary chat tab signals with slots in chatwindow
     * @param chatTab chatTab object to connect
     */
    void setupChatTabSignals(ChatTab *chatTab);

    KTabWidget *m_tabWidget;
};

#endif // CHATWINDOW_H
