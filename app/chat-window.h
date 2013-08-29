/*
    Copyright (C) 2010  David Edmundson    <kde@davidedmundson.co.uk>
    Copyright (C) 2011  Dominik Schmidt    <dev@dominik-schmidt.de>
    Copyright (C) 2011  Francesco Nwokeka  <francesco.nwokeka@gmail.com>

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
#include <KAction>

namespace Sonnet {
    class DictionaryComboBox;
}

class KIcon;
class ChatTab;
class QLabel;

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

    void destroyTab(ChatTab *tab);
    void setTabText(int index, const QString &newTitle);
    void setTabIcon(int index, const KIcon &newIcon);
    void setTabTextColor(int index,const QColor &color);

    /** retrieves tab with given textChannel if it exists
     * @param incomingTextChannel textChannel to search for
     */
    ChatTab* getTab(const Tp::AccountPtr &account, const Tp::TextChannelPtr &incomingTextChannel);
    ChatTab* getCurrentTab();

    void focusChat(ChatTab *tab);

    void addTab(ChatTab *tab);
    void removeTab(ChatTab *tab);

Q_SIGNALS:
    /** to emit before closing a window. This signal tells telepathyChatUi to remove the closed
     * window from it's list of open windows */
    void aboutToClose(ChatWindow *window);
    void detachRequested(ChatTab *tab);

public Q_SLOTS:
    void destroyTab(QWidget *chatWidget);

protected:
    virtual bool event(QEvent *e);


private Q_SLOTS:
    void tabBarContextMenu(int  index, const QPoint &  globalPos);
    void closeCurrentTab();
    void onAudioCallTriggered();                                /** start an audio call */
    void onBlockContactTriggered();                             /** Blocks contact */
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
    void onUnblockContactTriggered();                           /** Unblocks contact when already blocked */
    void onShareDesktopTriggered();                             /** start a desktop share */
    void onOpenLogTriggered();                                  /** Starts ktp-log-viewer accountId contactId */
    void onClearViewTriggered();                                /** Clears current view */
    void setTabSpellDictionary(const QString &dict);            /** set the spelling language for the current chat tab*/
    void toggleBlockButton(bool contactIsBlocked);              /** Toggle block/unblock action according to the flag */
    void updateAccountIcon();                                   /** Update account icon fake action */
    void onAddEmoticon(const QString& emoticon);                /** Add the corresponding emoticon*/
    void onZoomIn();
    void onZoomOut();
    void onZoomFactorChanged(qreal zoom);
    void updateSendMessageShortcuts();
    void onReloadTheme();
    void onCollaborateDocumentTriggered();

protected Q_SLOTS:
    void showSettingsDialog();
    void showNotificationsDialog();

private:
    /** sends notification to the user via plasma desktop notification system
     * @param type notification type
     * @param errorMsg message to display
     */
    void sendNotificationToUser(NotificationType type, const QString &errorMsg);

    /** removes chat tab signals. This is used when reparenting the chat tab
     * (see "detachTab" )
     */
    void removeChatTabSignals(ChatTab *chatTab);

    /** connects the necessary chat tab signals with slots in chatwindow
     * @param chatTab chatTab object to connect
     */
    void setupChatTabSignals(ChatTab *chatTab);

    /** creates and adds custom actions for the chat window */
    void setupCustomActions();

    /** setters for chat actions */
    void setAudioCallEnabled(bool enable);
    void setBlockEnabled(bool enable);
    void setFileTransferEnabled(bool enable);
    void setInviteToChatEnabled(bool enable);
    void setVideoCallEnabled(bool enable);
    void setShareDesktopEnabled(bool enable);
    void setPreviousConversationsEnabled(bool enable);
    void setCollaborateDocumentEnabled(bool enable);

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
     * @param account account starting the video call
     * @param contact contact with whom to start the video call
     */
    void startVideoCall(const Tp::AccountPtr &account, const Tp::ContactPtr &contact);

    /** starts a desktop sharing session with given contact
     * @param account account starting the desktop share
     * @param contact contact with whom to start desktop share
     */
    void startShareDesktop(const Tp::AccountPtr &account, const Tp::ContactPtr &contact);

    /** Returns whether there's at least one tab with unread message */
    bool hasUnreadMessages() const;

    /**
     * Offers a collaborative document to a contact, asking the user for which document to offer first.
     * @param account The account offering the document
     * @param targetContact The contact to offer the document to
     */
    void offerDocumentToContact(const Tp::AccountPtr& account, const Tp::ContactPtr& targetContact);

    /**
     * @brief Offers a collaborative document to a chatroom.
     * @param account The account offering the document
     * @param roomName The chatroom identifier to offer the document to
     */
    void offerDocumentToChatroom(const Tp::AccountPtr& account, const QString& roomName);

    KAction *m_sendMessage;

    KTabWidget *m_tabWidget;

    Sonnet::DictionaryComboBox *m_spellDictCombo;
    QLabel *m_accountIconLabel;
    qreal m_zoomFactor;
};

#endif // CHATWINDOW_H
