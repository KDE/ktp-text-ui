/*
    Copyright (C) 2010  David Edmundson    <kde@davidedmundson.co.uk>
    Copyright (C) 2011  Dominik Schmidt    <dev@dominik-schmidt.de>
    Copyright (C) 2011  Francesco Nwokeka  <francesco.nwokeka@gmail.com>
    Copyright (C) 2013  Daniel Cohen  <analoguecolour@gmail.com>

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
#include "KTpTextChatPart.h"

#include <kmainwindow.h>
#include <KTp/types.h>
#include <KDebug>
#include <KConfigGroup>
#include <KWindowSystem>
#include <kservice.h>
 #include <QStackedWidget>

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
    KMainWindow* window = createWindow();
    window->show();
}

//FIX ME does this behave correctly?
void TelepathyChatUi::removeWindow(ChatWindow *window)
{
    Q_ASSERT(window);
    m_chatWindows.removeOne(window);
}

KMainWindow* TelepathyChatUi::createWindow()
{
    KMainWindow* window = new KMainWindow();  
    KTabWidget* partTabWidget = new KTabWidget;
    partTabWidget->setDocumentMode(true);
    window->setCentralWidget(partTabWidget);
    window->show();
    m_chatWindows.push_back(window);
    return window;
}

//FIX ME this totally doesn't work yet
void TelepathyChatUi::dettachTab(ChatTab* tab)
{
    KMainWindow* window = createWindow();
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
        KMainWindow* window = m_chatWindows.at(i);
        KTabWidget* partTabWidget = window->findChild<KTabWidget*>();
        for (int j = 0; j < partTabWidget->count() ; ++j){
            ChatWidget* auxChatTab = qobject_cast<ChatWidget*>(partTabWidget->widget(j));
            if (auxChatTab->account() == account
                && auxChatTab->textChannel()->targetId() == textChannel->targetId()
                && auxChatTab->textChannel()->targetHandleType() == textChannel->targetHandleType()) {
                tabFound = true;
                partTabWidget->setCurrentIndex(j);

                KWindowSystem::forceActiveWindow(window->winId());
            }
        }
    }

    //if it's a group chat, we've been invited to. Join it
    if (textChannel->groupLocalPendingContacts().contains(textChannel->groupSelfContact())) {
        textChannel->groupAddContacts(QList<Tp::ContactPtr>() << textChannel->groupSelfContact());
    }

    //if there is currently no tab containing the incoming channel.

    if (!tabFound) {
        KService::Ptr service = KService::serviceByDesktopPath(QString::fromLatin1("KTpTextChatPart.desktop"));
        QVariantList args;
        QVariant storeChan, storeAcc;
        storeChan.setValue(textChannel);
        storeAcc.setValue(account);
        args << storeAcc << storeChan;
        KMainWindow* window;
        KParts::Part* m_part = service->createInstance<KParts::Part>(0,  args);
        if (m_chatWindows.count() == 1){
            window = m_chatWindows.at(0);
        }
        else {
            window = createWindow();
        }
        KTabWidget* partTabWidget = window->findChild<KTabWidget*>();
        partTabWidget->addTab(m_part->widget(),  textChannel->targetContact()->alias());


    }
//      TODO make this work
//        Q_ASSERT(window);
//
//         if (windowRaise) {
//             KWindowSystem::forceActiveWindow(window->winId());
//         }
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

