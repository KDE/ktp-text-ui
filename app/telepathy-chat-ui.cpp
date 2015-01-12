/*
    Copyright (C) 2010  David Edmundson    <kde@davidedmundson.co.uk>
    Copyright (C) 2011  Dominik Schmidt    <dev@dominik-schmidt.de>
    Copyright (C) 2011  Francesco Nwokeka  <francesco.nwokeka@gmail.com>
    Copyright (C) 2014  Daniel Vrátil      <dvratil@redhat.com>

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

#include "telepathy-chat-ui.h"
#include "chat-tab.h"
#include "chat-window.h"
#include "text-chat-config.h"
#include "notify-filter.h"
#include "text-chat-config.h"
#include "defines.h"

#include <KConfigGroup>
#include <KWindowSystem>

#include <QDebug>
#include <QEventLoopLocker>

#include <TelepathyQt/ChannelClassSpec>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/ChannelRequest>
#include <TelepathyQt/ChannelRequestHints>

#include <KTp/message-processor.h>


inline Tp::ChannelClassSpecList channelClassList()
{
    return Tp::ChannelClassSpecList() << Tp::ChannelClassSpec::textChat()
                                      << Tp::ChannelClassSpec::unnamedTextChat()
                                      << Tp::ChannelClassSpec::textChatroom();
}


TelepathyChatUi::TelepathyChatUi(int &argc, char *argv[])
    : KTp::TelepathyHandlerApplication(argc, argv, -1, -1),
      AbstractClientHandler(channelClassList())
{
    m_eventLoopLocker = 0;
    m_notifyFilter = new NotifyFilter;
    ChatWindow *window = createWindow();
    window->show();
}

TelepathyChatUi::~TelepathyChatUi()
{
    Q_FOREACH (const Tp::TextChannelPtr &channel, m_channelAccountMap.keys()) {
        channel->requestClose();
    }
    delete m_notifyFilter;
}

ChatWindow* TelepathyChatUi::createWindow()
{
    ChatWindow* window = new ChatWindow();

    connect(window, SIGNAL(detachRequested(ChatTab*)), this, SLOT(dettachTab(ChatTab*)));
    connect(window, SIGNAL(aboutToClose(ChatWindow*)), this, SLOT(onWindowAboutToClose(ChatWindow*)));

    m_chatWindows.push_back(window);

    return window;
}

bool TelepathyChatUi::isHiddenChannel(const Tp::AccountPtr &account,
                                      const Tp::TextChannelPtr& channel,
                                      Tp::TextChannelPtr *oldChannel) const
{
    if (channel->targetHandleType() != Tp::HandleTypeRoom) {
        return false;
    }

    QHash<Tp::TextChannelPtr,Tp::AccountPtr>::const_iterator it = m_channelAccountMap.constBegin();
    for ( ; it != m_channelAccountMap.constEnd(); ++it) {
        if (channel->targetId() == it.key()->targetId()
            && channel->targetHandleType() == it.key()->targetHandleType()
            && account == it.value())
        {
            *oldChannel = it.key();
            return true;
        }
    }

    return false;
}

void TelepathyChatUi::dettachTab(ChatTab* tab)
{
    ChatWindow* window = createWindow();
    tab->setChatWindow(window);
    window->show();
}

void TelepathyChatUi::handleChannels(const Tp::MethodInvocationContextPtr<> & context,
        const Tp::AccountPtr &account,
        const Tp::ConnectionPtr &connection,
        const QList<Tp::ChannelPtr> &channels,
        const QList<Tp::ChannelRequestPtr> &channelRequests,
        const QDateTime &userActionTime,
        const Tp::AbstractClientHandler::HandlerInfo &handlerInfo)
{
    Q_UNUSED(connection);
    Q_UNUSED(userActionTime);
    Q_UNUSED(handlerInfo);

    Tp::TextChannelPtr textChannel;
    Q_FOREACH(const Tp::ChannelPtr & channel, channels) {
        textChannel = Tp::TextChannelPtr::dynamicCast(channel);
        if (textChannel) {
            break;
        }
    }

    Q_ASSERT(textChannel);

    /*this works round a "bug" in which kwin will _deliberately_ stop the TextUi claiming focus
     * because it thinks the user is busy interacting with the contact list.
     * If the special hint org.kde.telepathy forceRaiseWindow is set to true, then we use KWindowSystem::forceActiveWindow
     * to claim focus.
     */
    bool windowRaise = true;

    //find the relevant channelRequest
    Q_FOREACH(const Tp::ChannelRequestPtr channelRequest, channelRequests) {
        windowRaise = !channelRequest->hints().hint(QLatin1String("org.kde.telepathy"), QLatin1String("suppressWindowRaise")).toBool();
    }

    qDebug() << "Incomming channel" << textChannel->targetId();
    qDebug() << "raise window hint set to: " << windowRaise;

    Tp::TextChannelPtr oldTextChannel;
    const bool isKnown = isHiddenChannel(account, textChannel, &oldTextChannel);
    if (isKnown) {
        // windowRaise is false, this is just an update after reconnect, so update
        // cache, but don't create window
        if (!windowRaise) {
            releaseChannel(oldTextChannel, account, false);
            takeChannel(textChannel, account, false);
            return;
        }
    }

    bool tabFound = false;

    //search for any tabs which are already handling this channel.
    for (int i = 0; i < m_chatWindows.count() && !tabFound; ++i) {
        ChatWindow *window = m_chatWindows.at(i);
        ChatTab* tab = window->getTab(account, textChannel);

        if (tab) {
            tabFound = true;
            if (windowRaise) {
                window->focusChat(tab);                 // set focus on selected tab
                KWindowSystem::forceActiveWindow(window->winId());
            }

            // check if channel is invalid. Replace only if invalid
            // You get this status if user goes offline and then back on without closing the chat
            if (!tab->textChannel()->isValid()) {
                tab->setTextChannel(textChannel);    // replace with new one
                tab->setChatEnabled(true);           // re-enable chat
            }
        }
    }

    //if it's a group chat, we've been invited to. Join it
    if (textChannel->groupLocalPendingContacts().contains(textChannel->groupSelfContact())) {
        textChannel->groupAddContacts(QList<Tp::ContactPtr>() << textChannel->groupSelfContact());
    }

    //if there is currently no tab containing the incoming channel.
    if (!tabFound) {
        ChatWindow* window = 0;
        switch (TextChatConfig::instance()->openMode()) {
            case TextChatConfig::FirstWindow:
                window = m_chatWindows.count()?m_chatWindows[0]:createWindow();
                break;
            case TextChatConfig::NewWindow:
                //as we now create a window on load, if we are in one window per chat mode
                //we need to check if the first made window is empty
                if (m_chatWindows.count() == 1 && ! m_chatWindows[0]->getCurrentTab()) {
                    window = m_chatWindows[0];
                } else {
                    window = createWindow();
                }
                break;
        }

        Q_ASSERT(window);

        ChatTab* tab = new ChatTab(textChannel, account);
        tab->setChatWindow(window);
        connect(tab, SIGNAL(aboutToClose(ChatTab*)),
                this, SLOT(onTabAboutToClose(ChatTab*)));
        window->show();

        if (windowRaise) {
            KWindowSystem::forceActiveWindow(window->winId());
        }
    }

    // the channel now has a tab and a window that owns it, so we can release it
    if (!oldTextChannel.isNull()) {
        releaseChannel(oldTextChannel, account);
    }

    context->setFinished();
}

bool TelepathyChatUi::bypassApproval() const
{
    return false;
}

void TelepathyChatUi::onTabAboutToClose(ChatTab *tab)
{
    const Tp::TextChannelPtr channel = tab->textChannel();

    // Close 1-on-1 chats, but keep group chats opened if user has configured so
    if (channel->targetHandleType() == Tp::HandleTypeContact || !TextChatConfig::instance()->dontLeaveGroupChats()) {
        channel->requestClose();
    } else {
        takeChannel(channel, tab->account());
    }
}

void TelepathyChatUi::onWindowAboutToClose(ChatWindow* window)
{
    Q_ASSERT(window);
    m_chatWindows.removeOne(window);

    // Take all tabs now. When tab emits aboutToClose, it's too late to call KGlobal::ref(),
    Q_FOREACH (ChatTab *tab, window->tabs()) {
        disconnect(tab, SIGNAL(aboutToClose(ChatTab*)),
                   this, SLOT(onTabAboutToClose(ChatTab*)));
        onTabAboutToClose(tab);
    }
}

void TelepathyChatUi::takeChannel(const Tp::TextChannelPtr& channel, const Tp::AccountPtr& account, bool ref)
{
    m_channelAccountMap.insert(channel, account);
    connectChannelNotifications(channel, true);
    connectAccountNotifications(account, true);

    if (ref && !m_eventLoopLocker) {
        m_eventLoopLocker = new QEventLoopLocker();
    }
}

void TelepathyChatUi::releaseChannel(const Tp::TextChannelPtr& channel, const Tp::AccountPtr& account, bool unref)
{
    m_channelAccountMap.remove(channel);
    connectChannelNotifications(channel, false);
    if (m_channelAccountMap.keys(account).count() == 0) {
        connectAccountNotifications(account, false);
    }

    if (unref && m_eventLoopLocker) {
        delete m_eventLoopLocker;
        m_eventLoopLocker = 0;
    }
}

void TelepathyChatUi::connectAccountNotifications(const Tp::AccountPtr& account, bool enable)
{
    if (enable) {
        connect(account.constData(), SIGNAL(connectionStatusChanged(Tp::ConnectionStatus)),
                this, SLOT(onConnectionStatusChanged(Tp::ConnectionStatus)),
                Qt::UniqueConnection);
    } else {
        disconnect(account.constData(), SIGNAL(connectionStatusChanged(Tp::ConnectionStatus)),
                   this, SLOT(onConnectionStatusChanged(Tp::ConnectionStatus)));
    }
}


void TelepathyChatUi::connectChannelNotifications(const Tp::TextChannelPtr &textChannel, bool enable)
{
    if (enable) {
        connect(textChannel.constData(), SIGNAL(messageReceived(Tp::ReceivedMessage)),
                this, SLOT(onGroupChatMessageReceived(Tp::ReceivedMessage)));
        connect(textChannel.constData(), SIGNAL(invalidated(Tp::DBusProxy*,QString,QString)),
                this, SLOT(onChannelInvalidated()));
    } else {
        disconnect(textChannel.constData(), SIGNAL(messageReceived(Tp::ReceivedMessage)),
                this, SLOT(onGroupChatMessageReceived(Tp::ReceivedMessage)));
        disconnect(textChannel.constData(), SIGNAL(invalidated(Tp::DBusProxy*,QString,QString)),
                this, SLOT(onChannelInvalidated()));
    }
}


void TelepathyChatUi::onGroupChatMessageReceived(const Tp::ReceivedMessage& message)
{
    const Tp::TextChannelPtr channel(qobject_cast<Tp::TextChannel*>(sender()));
    Tp::AccountPtr account = m_channelAccountMap.value(channel);

    KTp::Message processedMessage(KTp::MessageProcessor::instance()->processIncomingMessage(message, account, channel));
    m_notifyFilter->filterMessage(processedMessage, KTp::MessageContext(account, channel));
}

void TelepathyChatUi::onChannelInvalidated()
{
    const Tp::TextChannelPtr channel(qobject_cast<Tp::TextChannel*>(sender()));
    releaseChannel(channel, m_channelAccountMap.value(channel));
}

void TelepathyChatUi::onConnectionStatusChanged(Tp::ConnectionStatus status)
{
    if (status != Tp::ConnectionStatusConnected) {
        return;
    }

    Tp::ChannelRequestHints hints;
    hints.setHint(QLatin1String("org.kde.telepathy"),QLatin1String("suppressWindowRaise"), QVariant(true));

    const Tp::AccountPtr account(qobject_cast<Tp::Account*>(sender()));
    Q_FOREACH (const Tp::TextChannelPtr &channel, m_channelAccountMap.keys(account)) {
        account->ensureTextChatroom(channel->targetId(),
                                    QDateTime::currentDateTime(),
                                    QLatin1String(KTP_TEXTUI_CLIENT_PATH),
                                    hints);
    }
}
