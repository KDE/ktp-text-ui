/***************************************************************************
 *   Copyright (C) 2010 by David Edmundson <kde@davidedmundson.co.uk>      *
 *   Copyright (C) 2011 by Francesco Nwokeka <francesco.nwokeka@gmail.com> *
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

#include <QtCore/QString>
#include <QtGui/QWidget>
#include <QtWebKit/QWebPage>

#include <KIcon>
#include <KColorScheme>

#include <TelepathyQt/ReceivedMessage>

class AdiumThemeContentInfo;
class ChatSearchBar;
class ChatWidgetPrivate;
class QShowEvent;
class QKeyEvent;

class KDE_TELEPATHY_CHAT_EXPORT ChatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWidget(const Tp::TextChannelPtr &channel, const Tp::AccountPtr &account, QWidget *parent = 0);
    virtual ~ChatWidget();

    /** Returns pointer to account used for chat */
    Tp::AccountPtr account() const;

    /** Returns a pointer to the Chatwidget's search bar */
    ChatSearchBar *chatSearchBar() const;

    /** Returns the icon of this chat window */
    KIcon icon() const;

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

    /** Returns true if the user is currently typing or not */
    bool isUserTyping() const;

    QString spellDictionary() const;

    void setSpellDictionary(const QString &dict);

public Q_SLOTS:
    /** toggle the search bar visibility */
    void toggleSearchBar() const;

    /** Clear the list of unread messages
        call this when the widget is activated by the user.
      */
    void resetUnreadMessageCount();

protected:
    void changeEvent(QEvent *e);
    void resizeEvent(QResizeEvent *);
    void keyPressEvent(QKeyEvent *e);

protected Q_SLOTS:
    /** Show the received message in the chat window*/
    void handleIncomingMessage(const Tp::ReceivedMessage &message);

    /** Show notification about a received message */
    void notifyAboutIncomingMessage(const Tp::ReceivedMessage &message);

    /** Show the message sent in the chat window*/
    void handleMessageSent(const Tp::Message &message,
                           Tp::MessageSendingFlags flags,
                           const QString &sentMessageToken);

    /** send the text in the text area widget to the client handler*/
    void sendMessage();

    void onChatStatusChanged(const Tp::ContactPtr &contact, Tp::ChannelChatState state);

    void onContactPresenceChange(const Tp::ContactPtr &contact, const Tp::Presence &presence);

    void onContactAliasChanged(const Tp::ContactPtr &contact, const QString &alias);

    void onChannelInvalidated();

    void onInputBoxChanged();

    void chatViewReady();

Q_SIGNALS:
    /** Emitted whenever the title for the chat changes, normally the name of the contact or a topic*/
    void titleChanged(const QString &title);

    /** Emmitted if the icon for this channel changes*/
    void iconChanged(const KIcon &icon);

    /** Emmited whenever a message is received in this channel*/
    void messageReceived();

    /** emitted when searching for text */
    void searchTextComplete(bool found);

    /** Emitted when another contact in the channel starts/stops typing (if supported by the protocol)*/
    void userTypingChanged(bool);

    void contactPresenceChanged(Tp::Presence presence);

    void unreadMessagesChanged(int messages);

    /** Emitted when a notification for the chat window has been activated*/
    void notificationClicked();

private Q_SLOTS:
    /** received when user changes search criteria or when searching for text */
    void findTextInChat(const QString &text, QWebPage::FindFlags flags);
    void findNextTextInChat(const QString &text, QWebPage::FindFlags flags);
    void findPreviousTextInChat(const QString &text, QWebPage::FindFlags flags);
    void onFormatColorReleased();
    void onHistoryFetched(const QList<AdiumThemeContentInfo> &messages);

private:
    /** connects necessary signals for the channel */
    void setupChannelSignals();

    /** connects necessary signals for the contactModel */
    void setupContactModelSignals();

    void incrementUnreadMessageCount();
    virtual bool isOnTop() const;

    //FIXME this should be in the ktelepathy lib
    static KIcon iconForPresence(Tp::ConnectionPresenceType presence);

    ChatWidgetPrivate * const d;
};

#endif // CHATWIDGET_H
