/*
    Copyright (C) 2010  David Edmundson <kde@davidedmundson.co.uk>
    Copyright (C) 2011  Dominik Schmidt <dev@dominik-schmidt.de>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
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

    enum NotificationType {
        SystemErrorMessage,
        SystemInfoMessage
    };

    /**
     * starts a new chat with the textChannelPtr given only if the
     * chat doesn't already exist
     * @param incomingTextChannel new text channel
     */
    void startChat(const Tp::TextChannelPtr &incomingTextChannel, const Tp::AccountPtr &account);
    void removeTab(ChatTab *chatWidget);
    void setTabText(int index, const QString &newTitle);
    void setTabIcon(int index, const KIcon &newIcon);
    void setTabTextColor(int index,const QColor &color);

public slots:
    void removeTab(QWidget *chatWidget);

private slots:
    void closeCurrentTab();
    void onAudioCallTriggered();                                /** start an audio call */
    void onCurrentIndexChanged(int index);
    void onEnableSearchActions(bool enable);                    /** enables/disables menu search actions */
    void onFileTransferTriggered();                             /** start a file transfer (to be used only for 1on1 chats!) */
    void onFindNextText();                                      /** go to next text the user is searching for */
    void onFindPreviousText();                                  /** go to previous text the user is searching for */
    void onGenericOperationFinished(Tp::PendingOperation *op);
    void onInviteToChatTriggered();                             /** invite contact(s) to chat */
    void onNextTabActionTriggered();                            /** go to next tab in the tabwidget */
    void onPreviousTabActionTriggered();                        /** go to previous tab in the tabwidget */
    void onSearchActionToggled();                               /** toggle search bar visibility */
    void onTabStateChanged();
    void onTabTextChanged(const QString &newTitle);
    void onTabIconChanged(const KIcon &newIcon);
    void onVideoCallTriggered();                                /** start a video call */

protected slots:
    void showSettingsDialog();
    void showNotificationsDialog();

private:
    /** creats a new chat and adds it to the tab widget
     * @param channelPtr pointer to textChannel to use
     */
    void createNewChat(const Tp::TextChannelPtr &channelPtr, const Tp::AccountPtr &account);

    /** sends notification to the user via plasma desktop notification system
     * @param type notification type
     * @param errorMsg message to display
     */
    void sendNotificationToUser(NotificationType type, const QString &errorMsg);

    /** connects the neccessary chat tab signals with slots in chatwindow
     * @param chatTab chatTab object to connect
     */
    void setupChatTabSignals(ChatTab *chatTab);

    /** creates and adds custom actions for the chat window */
    void setupCustomActions();

    /** setters for chat actions */
    void setAudioCallEnabled(bool enable);
    void setFileTransferEnabled(bool enable);
    void setInviteToChatEnabled(bool enable);
    void setVideoCallEnabled(bool enable);

    /** starts audio call with given contact
     * @param account account sending the audio call request
     * @param contact contact with whom to start audio call
     */
    void startAudioCall(const Tp::AccountPtr &account, const Tp::ContactPtr &contact);

    /** starts file transfer
     * @param account account starting the file transfer
     * @param contact contact with whom to start file transfer
     */
    void startFileTransfer(const Tp::AccountPtr &account, const Tp::ContactPtr &contact);

    /** starts a video call with given contact
     * @param account account starting the file transfer
     * @param contact contact with whom to start file transfer
     */
    void startVideoCall(const Tp::AccountPtr &account, const Tp::ContactPtr &contact);

    KTabWidget *m_tabWidget;
};

#endif // CHATWINDOW_H
