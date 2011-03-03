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

#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include "kdetelepathychat_export.h"

#include <QtCore/QString>
#include <QtGui/QWidget>
#include <KIcon>
#include <KColorScheme>

#include <TelepathyQt4/ReceivedMessage>



class ChatWidgetPrivate;
class QShowEvent;

class KDE_TELEPATHY_CHAT_EXPORT ChatWidget : public QWidget
{
    Q_OBJECT

public:
    enum TitleColor {
        Default = KColorScheme::NormalText,
        Offline = KColorScheme::InactiveText,
        UnreadMessages = KColorScheme::ActiveText,
        UnreadStatus = KColorScheme::NeutralText,
        UnauthorizedContact = KColorScheme::NegativeText,
        CurrentlyTyping = KColorScheme::PositiveText
    };
    static QColor colorForRole(ChatWidget::TitleColor role);

    explicit ChatWidget(const Tp::TextChannelPtr & channel, QWidget *parent = 0);
    virtual ~ChatWidget();

    /** Returns the text channel pointer of the chatWidget */
    Tp::TextChannelPtr textChannel() const;

    /** Returns the name of this chat window*/
    QString title() const;

    /** Returns the icon of this chat window */
    KIcon icon() const;


    QColor titleColor() const;

    // unread messages methods
    /** Queried by standard isNewMessageUnread() **/
    virtual bool isOnTop() const;

    /** Decides whether a currently processed message should increment the unread messages counter **/
    virtual bool isNewMessageUnread();

    int unreadMessages() const;
    void incrementUnreadMessages();
    void resetUnreadMessages();

protected:
    void changeEvent(QEvent *e);
    void resizeEvent(QResizeEvent *);
    void showEvent(QShowEvent *e);


public slots:
    virtual void showOnTop();

protected slots:
    /** Show the received message in the chat window*/
    void handleIncomingMessage(const Tp::ReceivedMessage & message);

    /** Show notification about a received message */
    void notifyAboutIncomingMessage(const Tp::ReceivedMessage & message);

    /** Show the message sent in the chat window*/
    void handleMessageSent(const Tp::Message & message,
                           Tp::MessageSendingFlags flags,
                           const QString & sentMessageToken);

    /** send the text in the text area widget to the client handler*/
    void sendMessage();

    void onChatStatusChanged(const Tp::ContactPtr & contact, Tp::ChannelChatState state);

    void onContactPresenceChange(const Tp::ContactPtr & contact, const Tp::Presence & presence);

    void onContactAliasChanged(const Tp::ContactPtr & contact, const QString & alias);

    void onInputBoxChanged();

    void chatViewReady();

signals:
    /** Emitted whenever the title for the chat changes, normally the name of the contact or a topic*/
    void titleChanged(const QString & title);

    /** Emmitted if the icon for this channel changes*/
    void iconChanged(const KIcon & icon);

    /** Emmited whenever a message is received in this channel*/
    void messageReceived();

    /** Emitted when another contact in the channel starts/stops typing (if supported by the protocol)*/
    void userTypingChanged(bool);

    void contactPresenceChanged(Tp::Presence presence);

    void unreadMessagesChanged(int messages);

private slots:
    void onFormatColorReleased();

private:
    //FIXME this should be in the ktelepathy lib
    static KIcon iconForPresence(Tp::ConnectionPresenceType presence);
    static QColor colorForPresence(Tp::ConnectionPresenceType presence);

    ChatWidgetPrivate * const d;
};

#endif // CHATWIDGET_H
