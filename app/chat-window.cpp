/*
    Copyright (C) 2010  David Edmundson   <kde@davidedmundson.co.uk>
    Copyright (C) 2011  Dominik Schmidt   <dev@dominik-schmidt.de>
    Copyright (C) 2011  Francesco Nwokeka <francesco.nwokeka@gmail.com>

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


#include "chat-window.h"

#include "chat-search-bar.h"
#include "chat-tab.h"

#include <KStandardAction>
#include <KIcon>
#include <KLocale>
#include <KApplication>
#include <KAction>
#include <KActionCollection>
#include <KDebug>
#include <KFileDialog>
#include <KIcon>
#include <KColorScheme>
#include <KTabBar>
#include <KSettings/Dialog>
#include <KNotification>
#include <KNotifyConfigWidget>
#include <KMenuBar>
#include <KLineEdit>

#include <TelepathyQt4/Account>
#include <TelepathyQt4/ContactCapabilities>
#include <TelepathyQt4/PendingChannelRequest>
#include <TelepathyQt4/TextChannel>
#include <KMenu>

#define PREFERRED_TEXTCHAT_HANDLER "org.freedesktop.Telepathy.Client.KDE.TextUi"
#define PREFERRED_FILETRANSFER_HANDLER "org.freedesktop.Telepathy.Client.KDE.FileTransfer"
#define PREFERRED_AUDIO_VIDEO_HANDLER "org.freedesktop.Telepathy.Client.KDE.CallUi"

ChatWindow::ChatWindow()
{
    //setup actions
    KStandardAction::close(this,SLOT(closeCurrentTab()),actionCollection());
    KStandardAction::quit(KApplication::instance(), SLOT(quit()), actionCollection());
    KStandardAction::preferences(this, SLOT(showSettingsDialog()), actionCollection());
    KStandardAction::configureNotifications(this, SLOT(showNotificationsDialog()), actionCollection());
    KStandardAction::showMenubar(this->menuBar(), SLOT(setVisible(bool)), actionCollection());

    // keyboard shortcuts for the search bar
    KStandardAction::find(this, SLOT(onSearchActionToggled()), actionCollection());
    // start disabled
    KStandardAction::findNext(this, SLOT(onFindNextText()), actionCollection())->setEnabled(false);
    KStandardAction::findPrev(this, SLOT(onFindPreviousText()), actionCollection())->setEnabled(false);

    // create custom actions
    setupCustomActions();

    // set up m_tabWidget
    m_tabWidget = new KTabWidget(this);
    m_tabWidget->setMovable(true);
    m_tabWidget->setDocumentMode(true);
    m_tabWidget->setCloseButtonEnabled(true);
    m_tabWidget->setHoverCloseButtonDelayed(true);
    m_tabWidget->setTabBarHidden(true);

    connect(m_tabWidget, SIGNAL(closeRequest(QWidget*)), this, SLOT(destroyTab(QWidget*)));
    connect(m_tabWidget, SIGNAL(currentChanged(int)), this, SLOT(onCurrentIndexChanged(int)));
    connect(qobject_cast<KTabBar*>(m_tabWidget->tabBar()), SIGNAL(mouseMiddleClick(int)),m_tabWidget, SLOT(removeTab(int)));
    connect(qobject_cast<KTabBar*>(m_tabWidget->tabBar()), SIGNAL(contextMenu(int, QPoint)), SLOT(tabBarContextMenu(int, QPoint)));

    setCentralWidget(m_tabWidget);

    setupGUI(QSize(460, 440), static_cast<StandardWindowOptions>(Default^StatusBar), "chatwindow.rc");
}

ChatWindow::~ChatWindow()
{
    emit aboutToClose(this);
}

void ChatWindow::tabBarContextMenu(int index, const QPoint& globalPos)
{
    KAction close(KIcon("tab-close", KIconLoader::global()), i18n("Close"), this);
    KAction dettach(KIcon("tab-detach", KIconLoader::global()), i18n("Detach Tab"), this);
    KAction moveLeft(KIcon("arrow-left", KIconLoader::global()), i18n("Move Tab Left"), this);
    KAction moveRight(KIcon("arrow-right", KIconLoader::global()), i18n("Move Tab Right"), this);

    KMenu* menu = new KMenu(this);

    menu->addAction(&moveLeft);
    menu->addAction(&moveRight);
    menu->addAction(&dettach);
    menu->addAction(&close);

    if (index == 0) {
        moveLeft.setEnabled(false);
    } else if (index == (m_tabWidget->count() - 1)) {
        moveRight.setEnabled(false);
    }

    KAction* result = qobject_cast<KAction*>(menu->exec(globalPos));

    if(result == &close) {
        destroyTab(m_tabWidget->widget(index));
    } else if (result == &dettach) {
        emit detachRequested(qobject_cast<ChatTab*>(m_tabWidget->widget(index)));
    } else if (result == &moveLeft) {
        m_tabWidget->moveTab(index, index - 1);
    } else if (result == &moveRight) {
        m_tabWidget->moveTab(index, index + 1);
    }
}

void ChatWindow::focusChat(ChatTab* tab)
{
    kDebug();
    m_tabWidget->setCurrentWidget(tab);
}

ChatTab* ChatWindow::getTab(const Tp::TextChannelPtr& incomingTextChannel)
{
    ChatTab* match = 0;

    // if targetHandle is None, targetId is also ""
    if (!incomingTextChannel->targetHandleType() == Tp::HandleTypeNone) {

        // check that the tab requested isn't already open
        for (int index = 0; index < m_tabWidget->count() && !match; ++index) {

            // get chatWidget object
            ChatTab *auxChatTab = qobject_cast<ChatTab*>(m_tabWidget->widget(index));

            Q_ASSERT(auxChatTab);

            // check for 1on1 duplicate chat
            if (auxChatTab->textChannel()->targetId() == incomingTextChannel->targetId()
            && auxChatTab->textChannel()->targetHandleType() == incomingTextChannel->targetHandleType()) {
                match = auxChatTab;
            }
        }
    }
    return match;
}

void ChatWindow::removeTab(ChatTab* tab)
{
    kDebug();

    removeChatTabSignals(tab);

    m_tabWidget->removePage(tab);

    if (!m_tabWidget->isTabBarHidden()){
        if (m_tabWidget->count() <= 1) {
            m_tabWidget->setTabBarHidden(true);
        }
    }
}

void ChatWindow::addTab(ChatTab* tab)
{
    kDebug();

    setupChatTabSignals(tab);

    m_tabWidget->addTab(tab, tab->icon(), tab->title());
    m_tabWidget->setCurrentWidget(tab);

    if (m_tabWidget->isTabBarHidden()) {
        if (m_tabWidget->count() > 1) {
            m_tabWidget->setTabBarHidden(false);
        }
    }
}

void ChatWindow::destroyTab(QWidget* chatWidget)
{
    kDebug();

    ChatTab* tab = qobject_cast<ChatTab*>(chatWidget);
    Q_ASSERT(tab);

    tab->setChatWindow(0);
    chatWidget->deleteLater();
}

void ChatWindow::setTabText(int index, const QString &newTitle)
{
    m_tabWidget->setTabText(index, newTitle);

    // this updates the window title and icon if the updated tab is the current one
    if (index == m_tabWidget->currentIndex()) {
        onCurrentIndexChanged(index);
    }
}

void ChatWindow::setTabIcon(int index, const KIcon & newIcon)
{
    m_tabWidget->setTabIcon(index, newIcon);

    // this updates the window title and icon if the updated tab is the current one
    if (index == m_tabWidget->currentIndex()) {
        onCurrentIndexChanged(index);
    }
}

void ChatWindow::setTabTextColor(int index, const QColor& color)
{
    m_tabWidget->setTabTextColor(index, color);
}

void ChatWindow::closeCurrentTab()
{
    destroyTab(m_tabWidget->currentWidget());
}

void ChatWindow::onAudioCallTriggered()
{
    ChatWidget *currChat =  qobject_cast<ChatWidget*>(m_tabWidget->currentWidget());

    // a check. Should never happen
    if (!currChat) {
        return;
    }

    startAudioCall(currChat->account(), currChat->textChannel()->targetContact());
}

void ChatWindow::onCurrentIndexChanged(int index)
{
    kDebug() << index;

    if (index == -1) {
        close();
        return;
    }

    ChatTab* currentChatTab = qobject_cast<ChatTab*>(m_tabWidget->widget(index));
    setWindowTitle(currentChatTab->title());
    setWindowIcon(currentChatTab->icon());

    // when the tab changes I need to "refresh" the window's findNext and findPrev actions
    if (currentChatTab->chatSearchBar()->searchBar()->text().isEmpty()) {
        onEnableSearchActions(false);
    } else {
        onEnableSearchActions(true);
    }

    //enable/disable  send file/start webcam buttons
    //always disabled for group chats and offline accounts.
    if (!currentChatTab->isGroupChat() && currentChatTab->account()->connection()) {
        // check which capabilities the contact and user supports
        Tp::ContactCapabilities contactCapabilites = currentChatTab->textChannel()->targetContact()->capabilities();
        Tp::ContactCapabilities selfCapabilities = currentChatTab->textChannel()->groupSelfContact()->capabilities();

        setAudioCallEnabled(selfCapabilities.streamedMediaAudioCalls() && contactCapabilites.streamedMediaAudioCalls());
        setFileTransferEnabled(selfCapabilities.fileTransfers() && contactCapabilites.fileTransfers());
        setVideoCallEnabled(selfCapabilities.streamedMediaVideoCalls() && contactCapabilites.streamedMediaVideoCalls());
        /// TODO re-activate check when invitation to chat has been sorted out
        setInviteToChatEnabled(false);
    } else {
        setAudioCallEnabled(false);
        setFileTransferEnabled(false);
        setVideoCallEnabled(false);
        /// TODO re-activate check when invitation to chat has been sorted out
        setInviteToChatEnabled(false);
    }
}

void ChatWindow::onEnableSearchActions(bool enable)
{
    actionCollection()->action(KStandardAction::name(KStandardAction::FindNext))->setEnabled(enable);
    actionCollection()->action(KStandardAction::name(KStandardAction::FindPrev))->setEnabled(enable);
}

void ChatWindow::onFileTransferTriggered()
{
    // This feature should be used only in 1on1 chats!
    ChatWidget *currChat =  qobject_cast<ChatWidget*>(m_tabWidget->currentWidget());

    // This should never happen
    if (!currChat) {
        return;
    }

    startFileTransfer(currChat->account(), currChat->textChannel()->targetContact());
}

void ChatWindow::onFindNextText()
{
    ChatTab *currChat = qobject_cast<ChatTab*>(m_tabWidget->currentWidget());

    // This should never happen
    if(!currChat) {
        return;
    }
    currChat->chatSearchBar()->onNextButtonClicked();
}

void ChatWindow::onFindPreviousText()
{
    ChatTab *currChat = qobject_cast<ChatTab*>(m_tabWidget->currentWidget());

    // This should never happen
    if(!currChat) {
        return;
    }
    currChat->chatSearchBar()->onPreviousButtonClicked();
}

void ChatWindow::onGenericOperationFinished(Tp::PendingOperation* op)
{
    // send notification via plasma like the contactlist does
    if (op->isError()) {
        QString errorMsg(op->errorName() + ": " + op->errorMessage());
        sendNotificationToUser(SystemErrorMessage, errorMsg);
    }
}

void ChatWindow::onInviteToChatTriggered()
{
    /// TODO
}

void ChatWindow::onNextTabActionTriggered()
{
    int currIndex = m_tabWidget->currentIndex();

    if (currIndex < m_tabWidget->count() && m_tabWidget->count() != 1) {
        m_tabWidget->setCurrentIndex(++currIndex);
    }
}

void ChatWindow::onPreviousTabActionTriggered()
{
    int currIndex = m_tabWidget->currentIndex();

    if (currIndex > 0) {
        m_tabWidget->setCurrentIndex(--currIndex);
    }
}

void ChatWindow::onSearchActionToggled()
{
    ChatTab *currChat = qobject_cast<ChatTab*>(m_tabWidget->currentWidget());

    // This should never happen
    if(!currChat) {
        return;
    }
    currChat->toggleSearchBar();
}

void ChatWindow::onTabStateChanged()
{
    kDebug();

    ChatTab* sender = qobject_cast<ChatTab*>(QObject::sender());
    if (sender) {
        int tabIndex = m_tabWidget->indexOf(sender);
        setTabTextColor(tabIndex, sender->titleColor());
    }
}

void ChatWindow::onTabIconChanged(const KIcon & newIcon)
{
    //find out which widget made the call, and update the correct tab.
    QWidget* sender = qobject_cast<QWidget*>(QObject::sender());
    if (sender) {
        int tabIndexToChange = m_tabWidget->indexOf(sender);
        setTabIcon(tabIndexToChange, newIcon);
    }
}

void ChatWindow::onTabTextChanged(const QString &newTitle)
{
    //find out which widget made the call, and update the correct tab.
    QWidget* sender = qobject_cast<QWidget*>(QObject::sender());
    if (sender) {
        int tabIndexToChange = m_tabWidget->indexOf(sender);
        setTabText(tabIndexToChange, newTitle);
    }
}

void ChatWindow::onVideoCallTriggered()
{
    ChatWidget *currChat =  qobject_cast<ChatWidget*>(m_tabWidget->currentWidget());

    // This should never happen
    if (!currChat) {
        return;
    }

    startVideoCall(currChat->account(), currChat->textChannel()->targetContact());
}

void ChatWindow::showSettingsDialog()
{
    kDebug();

    KSettings::Dialog *dialog = new KSettings::Dialog(this);

    dialog->addModule("kcm_telepathy_chat_appearance_config");
    dialog->addModule("kcm_telepathy_chat_behavior_config");

    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void ChatWindow::showNotificationsDialog()
{
    KNotifyConfigWidget::configure(this, "ktelepathy");
}

void ChatWindow::removeChatTabSignals(ChatTab* chatTab)
{
    disconnect(chatTab, SIGNAL(titleChanged(QString)), this, SLOT(onTabTextChanged(QString)));
    disconnect(chatTab, SIGNAL(iconChanged(KIcon)), this, SLOT(onTabIconChanged(KIcon)));
    disconnect(chatTab, SIGNAL(userTypingChanged(bool)), this, SLOT(onTabStateChanged()));
    disconnect(chatTab, SIGNAL(unreadMessagesChanged(int)), this, SLOT(onTabStateChanged()));
    disconnect(chatTab, SIGNAL(contactPresenceChanged(Tp::Presence)), this, SLOT(onTabStateChanged()));
    disconnect(chatTab->chatSearchBar(), SIGNAL(enableSearchButtonsSignal(bool)), this, SLOT(onEnableSearchActions(bool)));
}

void ChatWindow::sendNotificationToUser(ChatWindow::NotificationType type, const QString& errorMsg)
{
    //The pointer is automatically deleted when the event is closed
    KNotification *notification;

    if (type == SystemInfoMessage) {
        notification = new KNotification("telepathyInfo", this);
    } else {
        notification = new KNotification("telepathyError", this);
    }

    notification->setText(errorMsg);
    notification->sendEvent();
}

void ChatWindow::setupChatTabSignals(ChatTab *chatTab)
{
    connect(chatTab, SIGNAL(titleChanged(QString)), this, SLOT(onTabTextChanged(QString)));
    connect(chatTab, SIGNAL(iconChanged(KIcon)), this, SLOT(onTabIconChanged(KIcon)));
    connect(chatTab, SIGNAL(userTypingChanged(bool)), this, SLOT(onTabStateChanged()));
    connect(chatTab, SIGNAL(unreadMessagesChanged(int)), this, SLOT(onTabStateChanged()));
    connect(chatTab, SIGNAL(contactPresenceChanged(Tp::Presence)), this, SLOT(onTabStateChanged()));
    connect(chatTab->chatSearchBar(), SIGNAL(enableSearchButtonsSignal(bool)), this, SLOT(onEnableSearchActions(bool)));
}

void ChatWindow::setupCustomActions()
{
    KAction *separator = new KAction(this);
    separator->setSeparator(true);

    KAction *nextTabAction = new KAction(KIcon("go-next-view"), i18n("&Next Tab"), this);
    nextTabAction->setShortcuts(KStandardShortcut::tabNext());
    connect(nextTabAction, SIGNAL(triggered()), this, SLOT(onNextTabActionTriggered()));

    KAction *previousTabAction = new KAction(KIcon("go-previous-view"), i18n("&Previous Tab"), this);
    previousTabAction->setShortcuts(KStandardShortcut::tabPrev());
    connect(previousTabAction, SIGNAL(triggered()), this, SLOT(onPreviousTabActionTriggered()));

    KAction *audioCallAction = new KAction(KIcon("voicecall"), i18n("&Audio Call"), this);
    connect(audioCallAction, SIGNAL(triggered()), this, SLOT(onAudioCallTriggered()));

    KAction *fileTransferAction = new KAction(KIcon("mail-attachment"), i18n("&Send File"), this);
    connect(fileTransferAction, SIGNAL(triggered()), this, SLOT(onFileTransferTriggered()));

    KAction *inviteToChat = new KAction(KIcon("user-group-new"), i18n("&Invite to chat"), this);
    connect(inviteToChat, SIGNAL(triggered()), this, SLOT(onInviteToChatTriggered()));

    KAction *videoCallAction = new KAction(KIcon("webcamsend"), i18n("&Video Call"), this);
    connect(videoCallAction, SIGNAL(triggered()), this, SLOT(onVideoCallTriggered()));

    // add custom actions to the collection
    actionCollection()->addAction("separator", separator);
    actionCollection()->addAction("next-tab", nextTabAction);
    actionCollection()->addAction("previous-tab", previousTabAction);
    actionCollection()->addAction("audio-call", audioCallAction);
    actionCollection()->addAction("send-file", fileTransferAction);
    actionCollection()->addAction("video-call", videoCallAction);
    actionCollection()->addAction("invite-to-chat", inviteToChat);
}

void ChatWindow::setAudioCallEnabled(bool enable)
{
    QAction *action = actionCollection()->action("audio-call");

    // don't want to segfault. Should never happen
    if (action) {
        action->setEnabled(enable);
    }
}

void ChatWindow::setFileTransferEnabled(bool enable)
{
    QAction *action = actionCollection()->action("send-file");

    if (action) {
        action->setEnabled(enable);
    }
}

void ChatWindow::setInviteToChatEnabled(bool enable)
{
    QAction *action = actionCollection()->action("invite-to-chat");

    if (action) {
        action->setEnabled(enable);
    }
}

void ChatWindow::setVideoCallEnabled(bool enable)
{
    QAction *action = actionCollection()->action("video-call");

    if (action) {
        action->setEnabled(enable);
    }
}

void ChatWindow::startAudioCall(const Tp::AccountPtr& account, const Tp::ContactPtr& contact)
{
    Tp::PendingChannelRequest* channelRequest = account->ensureStreamedMediaAudioCall(contact,
                                                                                      QDateTime::currentDateTime(),
                                                                                      PREFERRED_AUDIO_VIDEO_HANDLER);
    connect(channelRequest, SIGNAL(finished(Tp::PendingOperation*)), this, SLOT(onGenericOperationFinished(Tp::PendingOperation*)));
}

void ChatWindow::startFileTransfer(const Tp::AccountPtr& account, const Tp::ContactPtr& contact)
{
    // check for existence of ContactPtr
    Q_ASSERT(contact);

    // use the keyword "FileTransferLastDirectory" for setting last used dir for file transfer
    QString fileName = KFileDialog::getOpenFileName(KUrl("kfiledialog:///FileTransferLastDirectory"),
                                                    QString(),
                                                    this,
                                                    i18n("Choose a file"));

    // User hit cancel button
    if (fileName.isEmpty()) {
        return;
    }

    QFileInfo fileinfo(fileName);

    kDebug() << "Filename:" << fileName;
    kDebug() << "Content type:" << KMimeType::findByFileContent(fileName)->name();
    // TODO Let the user set a description?

    Tp::FileTransferChannelCreationProperties fileTransferProperties(fileName, KMimeType::findByFileContent(fileName)->name());

    Tp::PendingChannelRequest* channelRequest = account->createFileTransfer(contact,
                                                                            fileTransferProperties,
                                                                            QDateTime::currentDateTime(),
                                                                            PREFERRED_FILETRANSFER_HANDLER);

    connect(channelRequest, SIGNAL(finished(Tp::PendingOperation*)), SLOT(onGenericOperationFinished(Tp::PendingOperation*)));
}

void ChatWindow::startVideoCall(const Tp::AccountPtr& account, const Tp::ContactPtr& contact)
{
    Tp::PendingChannelRequest* channelRequest = account->ensureStreamedMediaVideoCall(contact,
                                                                                      true,
                                                                                      QDateTime::currentDateTime(),
                                                                                      PREFERRED_AUDIO_VIDEO_HANDLER);
    connect(channelRequest, SIGNAL(finished(Tp::PendingOperation*)), this, SLOT(onGenericOperationFinished(Tp::PendingOperation*)));
}




#include "chat-window.moc"
