/*
    Copyright (C) 2010  David Edmundson <kde@davidedmundson.co.uk>
    Copyright (C) 2011  Dominik Schmidt <dev@dominik-schmidt.de>

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
#include <TelepathyQt4/ChannelClassSpec>
#include <TelepathyQt4/TextChannel>
#include <KConfigGroup>

inline Tp::ChannelClassSpecList channelClassList()
{
    return Tp::ChannelClassSpecList() << Tp::ChannelClassSpec::textChat()
                                      << Tp::ChannelClassSpec::unnamedTextChat()
                                      << Tp::ChannelClassSpec::textChatroom();
}


TelepathyChatUi::TelepathyChatUi()
    : KApplication(), AbstractClientHandler(channelClassList())
{
    kDebug();

    KSharedConfigPtr config = KSharedConfig::openConfig("ktelepathyrc");
    KConfigGroup tabConfig = config->group("Behavior");

    QString mode = tabConfig.readEntry("tabOpenMode", "NewWindow");
    if(mode == "NewWindow"){
        openMode = NewWindow;
    } else if (mode == "FirstWindow"){
        openMode = FirstWindow;
    } else if (mode == "LastWindow"){
        openMode = LastWindow;
    }
}

void TelepathyChatUi::removeWindow()
{
    ChatWindow* window = qobject_cast<ChatWindow*>(sender());
    Q_ASSERT(window);
    m_chatWindows.removeOne(window);
}

ChatWindow* TelepathyChatUi::createWindow()
{
    kDebug();

    ChatWindow* window = new ChatWindow();
    connect(window, SIGNAL(aboutToClose()), SLOT(removeWindow()));
    connect(window, SIGNAL(dettachRequested(ChatTab*)), SLOT(dettachTab(ChatTab*)));
    m_chatWindows.push_back(window);

    return window;
}

void TelepathyChatUi::dettachTab(ChatTab* tab)
{
    ChatWindow* window = createWindow();
    tab->setWindow(window);
    window->show();
}

TelepathyChatUi::~TelepathyChatUi()
{
    kDebug();
}

void TelepathyChatUi::handleChannels(const Tp::MethodInvocationContextPtr<> & context,
        const Tp::AccountPtr & account,
        const Tp::ConnectionPtr & connection,
        const QList<Tp::ChannelPtr> & channels,
        const QList<Tp::ChannelRequestPtr> & requestsSatisfied,
        const QDateTime & userActionTime,
        const Tp::AbstractClientHandler::HandlerInfo & handlerInfo)
{
    kDebug();
    Q_UNUSED(connection);
    Q_UNUSED(requestsSatisfied);
    Q_UNUSED(userActionTime);
    Q_UNUSED(handlerInfo);

    Tp::TextChannelPtr textChannel;
    foreach(const Tp::ChannelPtr & channel, channels) {
        textChannel = Tp::TextChannelPtr::dynamicCast(channel);
        if (textChannel) {
            break;
        }
    }

    Q_ASSERT(textChannel);

    bool tabFound = false;
    foreach(ChatWindow* window, m_chatWindows) {
        ChatTab* tab = window->getTab(textChannel);
        if(tab){
            tabFound = true;

            tab->showOnTop();                                       // set focus on selected tab

            // check if channel is invalid. Replace only if invalid
            // You get this status if user goes offline and then back on without closing the chat
            if (!tab->textChannel()->isValid()) {
                tab->setTextChannel(textChannel);    // replace with new one
                tab->setChatEnabled(true);                   // re-enable chat
            }
        }
    }

    if(!tabFound) {
        ChatWindow* window = NULL;
        switch (openMode) {
            case FirstWindow:
                window = m_chatWindows.count()?m_chatWindows[0]:createWindow();
                break;
            case LastWindow:
            case NewWindow:
                window = createWindow();
                break;
        }
        ChatTab* tab = new ChatTab(textChannel, account);
        tab->setWindow(window);
        window->show();
    }

    context->setFinished();
}

bool TelepathyChatUi::bypassApproval() const
{
    return false;
}

