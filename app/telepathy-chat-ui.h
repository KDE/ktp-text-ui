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


#ifndef TELEPATHYCHATUI_H
#define TELEPATHYCHATUI_H

#include <TelepathyQt/AbstractClientHandler>
#include <TelepathyQt/AccountManager>

#include <KTp/telepathy-handler-application.h>

namespace KTp {
class AbstractMessageFilter;
}

namespace Tp {
class ReceivedMessage;
}

class ChatTab;
class ChatWindow;
class QEventLoopLocker;

class TelepathyChatUi : public KTp::TelepathyHandlerApplication, public Tp::AbstractClientHandler
{
    Q_OBJECT
public:
    TelepathyChatUi(int& argc, char* argv[]);
    ~TelepathyChatUi();

    virtual void handleChannels(const Tp::MethodInvocationContextPtr<> & context,
            const Tp::AccountPtr &account,
            const Tp::ConnectionPtr &connection,
            const QList<Tp::ChannelPtr> &channels,
            const QList<Tp::ChannelRequestPtr> &channelRequests,
            const QDateTime &userActionTime,
            const Tp::AbstractClientHandler::HandlerInfo &handlerInfo);

    virtual bool bypassApproval() const;

private Q_SLOTS:
    void dettachTab(ChatTab *tab);

    void onTabAboutToClose(ChatTab *tab);
    void onWindowAboutToClose(ChatWindow *window);
    void onGroupChatMessageReceived(const Tp::ReceivedMessage &msg);
    void onChannelInvalidated();
    void onConnectionStatusChanged(Tp::ConnectionStatus);

private:
    void takeChannel(const Tp::TextChannelPtr &channel, const Tp::AccountPtr &account, bool ref = true);
    void releaseChannel(const Tp::TextChannelPtr &channel, const Tp::AccountPtr &account, bool unref = true);
    void connectChannelNotifications(const Tp::TextChannelPtr &textChannel, bool enable);
    void connectAccountNotifications(const Tp::AccountPtr &account, bool enable);
    bool isHiddenChannel(const Tp::AccountPtr &account, const Tp::TextChannelPtr &channel,
                         Tp::TextChannelPtr *oldChannel) const;
    ChatWindow* createWindow();

    Tp::AccountManagerPtr m_accountManager;
    QList<ChatWindow*> m_chatWindows;

    QHash<Tp::TextChannelPtr, Tp::AccountPtr> m_channelAccountMap;
    KTp::AbstractMessageFilter *m_notifyFilter;
    QEventLoopLocker *m_eventLoopLocker;
};

#endif // TELEPATHYCHATUI_H
