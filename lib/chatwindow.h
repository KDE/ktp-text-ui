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

#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QtCore/QString>
#include <QtGui/QWidget>
#include <KIcon>

#include <TelepathyQt4/ReceivedMessage>

class ChatWindowPrivate;

class ChatWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWindow(const Tp::TextChannelPtr & channel, QWidget *parent = 0);
    virtual ~ChatWindow();

    /** Returns the name of this chat window*/
    QString title() const;

protected:
    void changeEvent(QEvent *e);

protected slots:
    /** Show the received message in the chat window*/
    void handleIncomingMessage(const Tp::ReceivedMessage & message);

    /** Show the message sent in the chat window*/
    void handleMessageSent(const Tp::Message & message,
                           Tp::MessageSendingFlags flags,
                           const QString & sentMessageToken);

    /** send the text in the text area widget to the client handler*/
    void sendMessage();

    void onChatStatusChanged(const Tp::ContactPtr & contact, Tp::ChannelChatState state);

    void onContactPresenceChange(const Tp::ContactPtr & contact, uint type);

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

private slots:
    void onFormatColorReleased();

private:
    void init();
    //FIXME this should be in the ktelepathy lib
    static KIcon iconForPresence(uint presence);

    ChatWindowPrivate * const d;
};

#endif // CHATWINDOW_H
