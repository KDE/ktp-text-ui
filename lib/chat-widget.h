/***************************************************************************
 *   Copyright (C) 2010 by David Edmundson <kde@davidedmundson.co.uk>      *
 *   Copyright (C) 2011 by Francesco Nwokeka <francesco.nwokeka@gmail.com> *
 *   Copyright (C) 2014 by Marcin Ziemiński <zieminn@gmail.com>            *
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

#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include "ktpchat_export.h"
#include "otr-status.h"
#include "types.h"

#include <QtCore/QString>
#include <QWidget>
#include <QWebEnginePage>

#include <QIcon>
#include <KColorScheme>

#include <TelepathyQt/ReceivedMessage>

#include <KTp/presence.h>
#include <KTp/message.h>
#include <KTp/OTR/types.h>

class ChatSearchBar;
class ChatWidgetPrivate;
class QShowEvent;
class QKeyEvent;
class ShareProvider;

class KDE_TELEPATHY_CHAT_EXPORT ChatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWidget(const Tp::TextChannelPtr &channel, const Tp::AccountPtr &account, QWidget *parent = 0);
    virtual ~ChatWidget();

    /** Returns pointer to account used for chat */
    Tp::AccountPtr account() const;

    /** Returns a pointer to the Chatwidget's search bar */
    ChatSearchBar* chatSearchBar() const;

    /** Returns the icon of this chat window */
    QIcon icon() const;

    /** Returns the icon for the account used in this chat window */
    QIcon accountIcon() const;

    /** returns whether the chat is considered a group chat */
    bool isGroupChat() const;

    /** invalidates the use of the chat.
     * @param enable flag to validate/invalidate chatWidget
     */
    void setChatEnabled(bool enable);

    /** Sets textchannel to given one */
    void setTextChannel(const Tp::TextChannelPtr &newTextChannelPtr);

    /** Returns the text channel pointer of the chatWidget */
    Tp::TextChannelPtr textChannel() const;

    /** Returns the name of this chat window */
    QString title() const;

    /** Returns the suggested color for the title of the window */
    QColor titleColor() const;

    int unreadMessageCount() const;

    QString spellDictionary() const;

    void setSpellDictionary(const QString &dict);

    QString currentKeyboardLayoutLanguage() const;

    void setCurrentKeyboardLayoutLanguage(const QString &language);

    void addEmoticonToChat(const QString &emoticon);

    /** Returns the chat state of remote contact */
    Tp::ChannelChatState remoteChatState();

    bool previousConversationAvailable();

    void clear();

    qreal zoomFactor() const;

    void setZoomFactor(qreal zoomFactor);

    /** Is this widget visible and in the active window */
    virtual bool isOnTop() const;

    /** Starts otr session */
    void startOtrSession();

    /** Stops otr session */
    void stopOtrSession();

    /** Athenticates contact in the context of otr conversation */
    void authenticateBuddy();

    /** Returns OtrStatus linked to the channel represented by this tab */
    OtrStatus otrStatus() const;

    /** If block then send message box is disabled */
    void blockTextInput(bool block);

public Q_SLOTS:
    /** toggle the search bar visibility */
    void toggleSearchBar() const;

    /** Mark that the following messages have been seen by the user.
      */
    void acknowledgeMessages();

    void updateSendMessageShortcuts(const QList<QKeySequence> &shortcuts);

    void reloadTheme();

protected:
    void changeEvent(QEvent *e);
    void resizeEvent(QResizeEvent *);
    void keyPressEvent(QKeyEvent *e);
    virtual void dropEvent(QDropEvent *e);
    virtual void dragEnterEvent(QDragEnterEvent *e);

protected Q_SLOTS:
    /** Show the received message in the chat window*/
    void handleIncomingMessage(const Tp::ReceivedMessage &message, bool alreadyNotified = false);

    /** Show the message sent in the chat window*/
    void handleMessageSent(const Tp::Message &message,
                           Tp::MessageSendingFlags flags,
                           const QString &sentMessageToken);

    /** send the text in the text area widget to the client handler*/
    void sendMessage();

    void onChatStatusChanged(const Tp::ContactPtr &contact, Tp::ChannelChatState state);

    void onContactBlockStatusChanged(const Tp::ContactPtr &contact, bool blocked);

    void onContactPresenceChange(const Tp::ContactPtr &contact, const KTp::Presence &presence);

    void onContactAliasChanged(const Tp::ContactPtr &contact, const QString &alias);

    void onContactClientTypesChanged(const Tp::ContactPtr &contact, const QStringList &clientTypes);

    void onParticipantsChanged(Tp::Contacts groupMembersAdded,
                               Tp::Contacts groupLocalPendingMembersAdded,
                               Tp::Contacts groupRemotePendingMembersAdded,
                               Tp::Contacts groupMembersRemoved,
                               Tp::Channel::GroupMemberChangeDetails details);

    void onChannelInvalidated();

    void onInputBoxChanged();

    void chatViewReady();

Q_SIGNALS:
    /** Emitted whenever the title for the chat changes, normally the name of the contact or a topic*/
    void titleChanged(const QString &title);

    /** Emmitted if the icon for this channel changes*/
    void iconChanged(const QIcon &icon);

    /** Emmited whenever a message is received in this channel. It is up to the parent application to acknowledge these messages*/
    void messageReceived(const Tp::ReceivedMessage &message);

    /** emitted when searching for text */
    void searchTextComplete(bool found);

    /** Emitted when another contact in the channel starts/stops typing (if supported by the protocol)*/
    void userTypingChanged(Tp::ChannelChatState);

    /** Emitted when a contact is blocked or unblocked */
    void contactBlockStatusChanged(bool blocked);

    void contactPresenceChanged(KTp::Presence presence);

    void unreadMessagesChanged();

    /** Emitted when a notification for the chat window has been activated*/
    void notificationClicked();

    /** Emitted when zoom level in chat view changes */
    void zoomFactorChanged(qreal zoomFactor);

    /** Emitted when OTRTrustLevel changes in the channel */
    void otrStatusChanged(OtrStatus otrStatus);

private Q_SLOTS:
    /** received when user changes search criteria or when searching for text */
    void findTextInChat(const QString &text, QWebEnginePage::FindFlags flags);
    void findNextTextInChat(const QString &text, QWebEnginePage::FindFlags flags);
    void findPreviousTextInChat(const QString &text, QWebEnginePage::FindFlags flags);
    void onHistoryFetched(const QList<KTp::Message> &messages);
    void onChatPausedTimerExpired();
    void currentPresenceChanged(const Tp::Presence &presence);

    void temporaryFileTransferChannelCreated(Tp::PendingOperation *operation);
    void temporaryFileTransferStateChanged(Tp::FileTransferState, Tp::FileTransferStateChangeReason);

    void onContactsViewContextMenuRequested(const QPoint &point);
    void onFileTransferMenuActionTriggered();
    void onMessageWidgetSwitchOnlineActionTriggered();
    void onOpenContactChatWindowClicked();
    void onShowContactDetailsClicked();
    void onShareImageMenuActionTriggered();
    void onShareProviderFinishedSuccess(ShareProvider *provider, const QString &imageUrl);
    void onShareProviderFinishedFailure(ShareProvider *provider, const QString &errorMessage);
    void onSendFileClicked();

    void onOTRTrustLevelChanged(KTp::OTRTrustLevel trustLevel, KTp::OTRTrustLevel previous);
    void onOTRsessionRefreshed();
    void onPeerAuthenticationRequestedQA(const QString &question);
    void onPeerAuthenticationRequestedSS();
    void onPeerAuthenticationConcluded(bool authenticated);
    void onPeerAuthenticationInProgress();
    void onPeerAuthenticationAborted();
    void onPeerAuthenticationFailed();

private:
    /** connects necessary signals for the channel */
    void setupChannelSignals();

    /** connects necessary signals for the contactModel */
    void setupContactModelSignals();

    /** Saves pair <target Id,language option selected> in a file */
    void saveSpellCheckingOption();

    /** Loads language option for specified target Id */
    void loadSpellCheckingOption();

    /** Loads theme into the AdiumThemeView */
    void initChatArea();

    /** connects necessary signals for the otr proxy channel */
    void setupOTR();

    bool m_previousConversationAvailable;

    ChatWidgetPrivate * const d;
};

#endif // CHATWIDGET_H
