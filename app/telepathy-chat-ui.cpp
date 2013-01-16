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

#include "telepathy-chat-ui.h"
#include "chat-tab.h"
#include "chat-window.h"

#include <KDebug>
#include <KConfigGroup>
#include <KWindowSystem>

#include <TelepathyQt/ChannelClassSpec>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/ChannelRequest>
#include <TelepathyQt/ChannelRequestHints>


inline Tp::ChannelClassSpecList channelClassList()
{
    return Tp::ChannelClassSpecList() << Tp::ChannelClassSpec::textChat()
                                      << Tp::ChannelClassSpec::unnamedTextChat()
                                      << Tp::ChannelClassSpec::textChatroom();
}


TelepathyChatUi::TelepathyChatUi(const Tp::AccountManagerPtr &accountManager)
    : KTp::TelepathyHandlerApplication(true, -1, -1), AbstractClientHandler(channelClassList()),
      m_accountManager(accountManager)
{
    kDebug();

    KSharedConfigPtr config = KSharedConfig::openConfig(QLatin1String("ktelepathyrc"));
    KConfigGroup tabConfig = config->group("Behavior");

    // load the settings for new tab "open mode"
    QString mode = tabConfig.readEntry("tabOpenMode", "FirstWindow");
    if (mode == QLatin1String("NewWindow")) {
        m_openMode = NewWindow;
    } else if (mode == QLatin1String("FirstWindow")) {
        m_openMode = FirstWindow;
    } else if (mode == QLatin1String("LastWindow")) {
        m_openMode = LastWindow;
    }
}

void TelepathyChatUi::removeWindow(ChatWindow *window)
{
    Q_ASSERT(window);
    m_chatWindows.removeOne(window);
}

ChatWindow* TelepathyChatUi::createWindow()
{
    ChatWindow* window = new ChatWindow();

    connect(window, SIGNAL(detachRequested(ChatTab*)), this, SLOT(dettachTab(ChatTab*)));
    connect(window, SIGNAL(aboutToClose(ChatWindow*)), this, SLOT(removeWindow(ChatWindow*)));

    m_chatWindows.push_back(window);

    return window;
}

void TelepathyChatUi::dettachTab(ChatTab* tab)
{
    ChatWindow* window = createWindow();
    tab->setChatWindow(window);
    window->show();
}

TelepathyChatUi::~TelepathyChatUi()
{
}

void TelepathyChatUi::handleChannels(const Tp::MethodInvocationContextPtr<> & context,
        const Tp::AccountPtr &account,
        const Tp::ConnectionPtr &connection,
        const QList<Tp::ChannelPtr> &channels,
        const QList<Tp::ChannelRequestPtr> &channelRequests,
        const QDateTime &userActionTime,
        const Tp::AbstractClientHandler::HandlerInfo &handlerInfo)
{
    kDebug();
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
        kDebug() << channelRequest->hints().allHints();
        windowRaise = !channelRequest->hints().hint(QLatin1String("org.kde.telepathy"), QLatin1String("suppressWindowRaise")).toBool();
    }

    kDebug() << "raise window hint set to: " << windowRaise;

    bool tabFound = false;

    //search for any tabs which are already handling this channel.
    for (int i = 0; i < m_chatWindows.count() && !tabFound; ++i) {
        ChatWindow *window = m_chatWindows.at(i);
        ChatTab* tab = window->getTab(textChannel);

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
        switch (m_openMode) {
            case FirstWindow:
                window = m_chatWindows.count()?m_chatWindows[0]:createWindow();
                break;
            case LastWindow:
            case NewWindow:
                window = createWindow();
                break;
        }

        Q_ASSERT(window);

        ChatTab* tab = new ChatTab(textChannel, account);
        tab->setChatWindow(window);
        window->show();

        if (windowRaise) {
            KWindowSystem::forceActiveWindow(window->winId());
        }
    }

    context->setFinished();
}

bool TelepathyChatUi::bypassApproval() const
{
    return false;
}

Tp::AccountManagerPtr TelepathyChatUi::accountManager() const
{
    return m_accountManager;
}

