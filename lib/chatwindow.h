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

#include <QWidget>
#include <QString>
#include <KIcon>

#include <TelepathyQt4/ReceivedMessage>
#include "chatconnection.h"


namespace Ui
{
class ChatWindow;
}

class ChatWindowPrivate;

class MessageBoxEventFilter : public QObject
{
    Q_OBJECT
public:
    MessageBoxEventFilter(QObject* parent = 0) : QObject(parent) {};
    virtual ~MessageBoxEventFilter() {}
protected:
    bool eventFilter(QObject *obj, QEvent *event);
Q_SIGNALS:
    void returnKeyPressed();
};

class ChatWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWindow(ChatConnection* chat, QWidget *parent = 0);
    ~ChatWindow();


    /** Returns the name of this chat window*/
    QString title();

protected:
    void changeEvent(QEvent *e);

protected slots:
    /** Show the received message in the chat window*/
    void handleIncomingMessage(const Tp::ReceivedMessage  &message);

    /** Show the message sent in the chat window*/
    void handleMessageSent(const Tp::Message  &message, Tp::MessageSendingFlags flags, const QString &sentMessageToken);

    /** send the text in the text area widget to the client handler*/
    void sendMessage();

    /** Enable/Disable buttons in the chat window*/
    void updateEnabledState(bool enabled);

    void onChatStatusChanged(Tp::ContactPtr contact, ChannelChatState state);

    void onContactPresenceChange(Tp::ContactPtr, uint type);

    void onInputBoxChanged();

    void chatViewReady();



signals:
    /** Emitted whenever the title for the chat changes, normally the name of the contact or a topic*/
    void titleChanged(QString title);

    /** Emmitted if the icon for this channel changes*/
    void iconChanged(KIcon icon);

    /** Emmited whenever a message is received in this channel*/
    void messageReceived();

    /** Emitted when another contact in the channel starts/stops typing (if supported by the protocol)*/
    void userTypingChanged(bool);


private slots:
    void onFormatColorReleased();

private:
    KIcon iconForPresence(uint presence);

    Ui::ChatWindow *ui;
    ChatConnection* m_chatConnection; //FIXME deprecate this whole class when factories is finished.
    ChatWindowPrivate *d;

};

#endif // CHATWINDOW_H
