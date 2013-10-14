/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright 2013  David Edmundson <kde@davidedmundson.co.uk>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef MESSAGEVIEW_H
#define MESSAGEVIEW_H

#include "adium-theme-view.h"

/** This class splits out all the message handling code from ChatWidget that will soon be replaced by
 * a DeclarativeView + Conversation/MessagesModel from common-internals.
 *
 * Functionality should closely match the classes listed above, but API can be different.
 *
 * This is to allow a drop in replacement of the QML text view when it is ready
 */

namespace KTp{
    class AbstractMessageFilter;
}
class LogManager;

class MessageView : public AdiumThemeView
{
    Q_OBJECT
public:
    explicit MessageView(QWidget *parent = 0);
    virtual ~MessageView();

    //FIXME I neeed the account set once, text channel multiple times, the name is a bit rubbish.
    void setTextChannel(const Tp::AccountPtr &account, const Tp::TextChannelPtr &textChannel);

    void acknowledgeMessages();

    void initChatArea();

    void reloadTheme();

    void clear();

Q_SIGNALS:
    //notify that a message has been recieved or sent, this is needed for notifications in the parent app
    void newMessage(const KTp::Message &message);

private Q_SLOTS:
    /** Show the received message in the chat window*/

    //FIXME can we drop this last parameter?
    void handleIncomingMessage(const Tp::ReceivedMessage& message, bool alreadyNotified=false);

    /** Show the message sent in the chat window*/
    void handleMessageSent(const Tp::Message &message,
                           Tp::MessageSendingFlags flags,
                           const QString &sentMessageToken);

    void onHistoryFetched(const QList<KTp::Message> &messages);

    void onChatViewReady();

private:
    Tp::AccountPtr m_account;
    Tp::TextChannelPtr m_channel;
    bool m_chatViewInitialized;
    bool m_logsLoaded;
    uint m_exchangedMessagesCount;
    LogManager *m_logManager;
};

#endif // MESSAGEVIEW_H
