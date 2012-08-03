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
#include "chat-widget.h"

#include <KTp/service-availability-checker.h>

#include <KStandardAction>
#include <KIcon>
#include <KLocale>
#include <KApplication>
#include <KAction>
#include <KActionCollection>
#include <KDebug>
#include <KFileDialog>
#include <KColorScheme>
#include <KTabBar>
#include <KSettings/Dialog>
#include <KNotification>
#include <KNotifyConfigWidget>
#include <KMenuBar>
#include <KLineEdit>
#include <KMenu>
#include <KToolBar>
#include <KToolInvocation>

#include <QEvent>
#include <QWidgetAction>
#include <QLabel>

#include <TelepathyQt/Account>
#include <TelepathyQt/ContactCapabilities>
#include <TelepathyQt/PendingChannelRequest>
#include <TelepathyQt/TextChannel>

#include <Sonnet/DictionaryComboBox>

#include "invite-contact-dialog.h"

#define PREFERRED_TEXTCHAT_HANDLER "org.freedesktop.Telepathy.Client.KTp.TextUi"
#define PREFERRED_FILETRANSFER_HANDLER "org.freedesktop.Telepathy.Client.KTp.FileTransfer"
#define PREFERRED_AUDIO_VIDEO_HANDLER "org.freedesktop.Telepathy.Client.KTp.CallUi"
#define PREFERRED_RFB_HANDLER "org.freedesktop.Telepathy.Client.krfb_rfb_handler"

K_GLOBAL_STATIC_WITH_ARGS(KTp::ServiceAvailabilityChecker, s_krfbAvailableChecker,
                          (QLatin1String(PREFERRED_RFB_HANDLER)));

ChatWindow::ChatWindow()
{
    //This effectively constructs the s_krfbAvailableChecker object the first
    //time that this code is executed. This is to start the d-bus query early, so
    //that data are available when we need them later in desktopSharingCapability()
    (void) s_krfbAvailableChecker.operator->();

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
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setTabBarHidden(true);

    connect(m_tabWidget, SIGNAL(closeRequest(QWidget*)), this, SLOT(destroyTab(QWidget*)));
    connect(m_tabWidget, SIGNAL(currentChanged(int)), this, SLOT(onCurrentIndexChanged(int)));
    connect(qobject_cast<KTabBar*>(m_tabWidget->tabBar()), SIGNAL(mouseMiddleClick(int)),m_tabWidget, SLOT(removeTab(int)));
    connect(qobject_cast<KTabBar*>(m_tabWidget->tabBar()), SIGNAL(contextMenu(int,QPoint)), SLOT(tabBarContextMenu(int,QPoint)));

    setCentralWidget(m_tabWidget);

    setupGUI(QSize(460, 440), static_cast<StandardWindowOptions>(Default^StatusBar), QLatin1String("chatwindow.rc"));
}

ChatWindow::~ChatWindow()
{
    Q_EMIT aboutToClose(this);
}

void ChatWindow::tabBarContextMenu(int index, const QPoint& globalPos)
{
    KAction close(KIcon(QLatin1String("tab-close"), KIconLoader::global()), i18n("Close"), this);
    KAction dettach(KIcon(QLatin1String("tab-detach"), KIconLoader::global()), i18n("Detach Tab"), this);
    KAction moveLeft(KIcon(QLatin1String("arrow-left"), KIconLoader::global()), i18n("Move Tab Left"), this);
    KAction moveRight(KIcon(QLatin1String("arrow-right"), KIconLoader::global()), i18n("Move Tab Right"), this);

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
        Q_EMIT detachRequested(qobject_cast<ChatTab*>(m_tabWidget->widget(index)));
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

    // if targetHandle is None, targetId is also "", therefore we won't be able to find it.
    if (!incomingTextChannel->targetHandleType() == Tp::HandleTypeNone) {

        //loop through all tabs checking for matches
        for (int index = 0; index < m_tabWidget->count() && !match; ++index) {

            // get chatWidget object
            ChatTab *auxChatTab = qobject_cast<ChatTab*>(m_tabWidget->widget(index));

            Q_ASSERT(auxChatTab);

            // check for duplicate chat
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

void ChatWindow::onBlockContactTriggered()
{
    ChatWidget *currChat = qobject_cast<ChatWidget*>(m_tabWidget->currentWidget());

    if (!currChat) {
        return;
    }

    Tp::ContactPtr contact = currChat->textChannel()->targetContact();
    if(!contact.isNull()) {
        contact->block();
    }
}

void ChatWindow::onCurrentIndexChanged(int index)
{
    kDebug() << index;

    if (index == -1) {
        close();
        return;
    }

    ChatTab* currentChatTab = qobject_cast<ChatTab*>(m_tabWidget->widget(index));
    currentChatTab->acknowledgeMessages();
    setWindowTitle(currentChatTab->title());
    setWindowIcon(currentChatTab->icon());

    //call this to update the "Typing.." in window title
    onUserTypingChanged(currentChatTab->remoteChatState());

    kDebug() << "Current spell dictionary is" << currentChatTab->spellDictionary();
    m_spellDictCombo->setCurrentByDictionary(currentChatTab->spellDictionary());

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
        setShareDesktopEnabled(s_krfbAvailableChecker->isAvailable() && contactCapabilites.streamTubes(QLatin1String("rfb")));
        setInviteToChatEnabled(true);

        toggleBlockButton(currentChatTab->textChannel()->targetContact()->isBlocked());

    } else {
        setAudioCallEnabled(false);
        setFileTransferEnabled(false);
        setVideoCallEnabled(false);
        setShareDesktopEnabled(false);
        setInviteToChatEnabled(true);
        setBlockEnabled(false);

    }

    // only show enable the action if there are actually previous converstations
#ifdef TELEPATHY_LOGGER_QT4_FOUND
    setPreviousConversationsEnabled(currentChatTab->previousConversationAvailable());
#endif

    setAccountIcon(currentChatTab->accountIcon());
}

void ChatWindow::onEnableSearchActions(bool enable)
{
    actionCollection()->action(QLatin1String(KStandardAction::name(KStandardAction::FindNext)))->setEnabled(enable);
    actionCollection()->action(QLatin1String(KStandardAction::name(KStandardAction::FindPrev)))->setEnabled(enable);
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
        QString errorMsg(op->errorName() + QLatin1String(": ") + op->errorMessage());
        sendNotificationToUser(SystemErrorMessage, errorMsg);
    }
}

void ChatWindow::onInviteToChatTriggered()
{
    ChatTab *currChat = qobject_cast<ChatTab*>(m_tabWidget->currentWidget());
    InviteContactDialog *dialog = new InviteContactDialog(currChat->account(), currChat->textChannel(), this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
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

void ChatWindow::onUnblockContactTriggered()
{
    ChatWidget *currChat = qobject_cast<ChatWidget*>(m_tabWidget->currentWidget());

    if(!currChat) {
        return;
    }

    Tp::ContactPtr contact = currChat->textChannel()->targetContact();
    contact->unblock();
}

void ChatWindow::onShareDesktopTriggered()
{
    ChatWidget *currChat =  qobject_cast<ChatWidget*>(m_tabWidget->currentWidget());

    // This should never happen
    if (!currChat) {
        return;
    }

    startShareDesktop(currChat->account(), currChat->textChannel()->targetContact());
}

void ChatWindow::onOpenLogTriggered()
{
    int index = m_tabWidget->currentIndex();
    ChatTab* currentChatTab = qobject_cast<ChatTab*>(m_tabWidget->widget(index));
    Q_ASSERT(currentChatTab);

    Tp::AccountPtr account = currentChatTab->account();
    Tp::ContactPtr contact = currentChatTab->textChannel()->targetContact();

    if (!contact.isNull()) {
        KToolInvocation::kdeinitExec(QLatin1String("ktp-log-viewer"),
                                     QStringList() << account->uniqueIdentifier() << contact->id());
    } else {
        KToolInvocation::kdeinitExec(QLatin1String("ktp-log-viewer"),
                                     QStringList() << account->uniqueIdentifier() << currentChatTab->textChannel()->targetId());
    }
}


void ChatWindow::showSettingsDialog()
{
    kDebug();

    KSettings::Dialog *dialog = new KSettings::Dialog(this);

    dialog->addModule(QLatin1String("kcm_ktp_chat_appearance"));
    dialog->addModule(QLatin1String("kcm_ktp_chat_behavior"));

    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void ChatWindow::showNotificationsDialog()
{
    KNotifyConfigWidget::configure(this, QLatin1String("ktelepathy"));
}

void ChatWindow::removeChatTabSignals(ChatTab* chatTab)
{
    disconnect(chatTab, SIGNAL(titleChanged(QString)), this, SLOT(onTabTextChanged(QString)));
    disconnect(chatTab, SIGNAL(iconChanged(KIcon)), this, SLOT(onTabIconChanged(KIcon)));
    disconnect(chatTab, SIGNAL(userTypingChanged(Tp::ChannelChatState)), this, SLOT(onTabStateChanged()));
    disconnect(chatTab, SIGNAL(unreadMessagesChanged()), this, SLOT(onTabStateChanged()));
    disconnect(chatTab, SIGNAL(contactPresenceChanged(Tp::Presence)), this, SLOT(onTabStateChanged()));
    disconnect(chatTab->chatSearchBar(), SIGNAL(enableSearchButtonsSignal(bool)), this, SLOT(onEnableSearchActions(bool)));
    disconnect(chatTab, SIGNAL(contactBlockStatusChanged(bool)), this, SLOT(toggleBlockButton(bool)));
}

void ChatWindow::sendNotificationToUser(ChatWindow::NotificationType type, const QString& errorMsg)
{
    //The pointer is automatically deleted when the event is closed
    KNotification *notification;

    if (type == SystemInfoMessage) {
        notification = new KNotification(QLatin1String("telepathyInfo"), this);
    } else {
        notification = new KNotification(QLatin1String("telepathyError"), this);
    }

    notification->setText(errorMsg);
    notification->sendEvent();
}

void ChatWindow::setupChatTabSignals(ChatTab *chatTab)
{
    connect(chatTab, SIGNAL(titleChanged(QString)), this, SLOT(onTabTextChanged(QString)));
    connect(chatTab, SIGNAL(iconChanged(KIcon)), this, SLOT(onTabIconChanged(KIcon)));
    connect(chatTab, SIGNAL(userTypingChanged(Tp::ChannelChatState)), this, SLOT(onTabStateChanged()));
    connect(chatTab, SIGNAL(userTypingChanged(Tp::ChannelChatState)), this, SLOT(onUserTypingChanged(Tp::ChannelChatState)));
    connect(chatTab, SIGNAL(unreadMessagesChanged()), this, SLOT(onTabStateChanged()));
    connect(chatTab, SIGNAL(contactPresenceChanged(KTp::Presence)), this, SLOT(onTabStateChanged()));
    connect(chatTab->chatSearchBar(), SIGNAL(enableSearchButtonsSignal(bool)), this, SLOT(onEnableSearchActions(bool)));
    connect(chatTab, SIGNAL(contactBlockStatusChanged(bool)), this, SLOT(toggleBlockButton(bool)));
}

void ChatWindow::setupCustomActions()
{
    KAction *nextTabAction = new KAction(KIcon(QLatin1String("go-next-view")), i18n("&Next Tab"), this);
    nextTabAction->setShortcuts(KStandardShortcut::tabNext());
    connect(nextTabAction, SIGNAL(triggered()), this, SLOT(onNextTabActionTriggered()));

    KAction *previousTabAction = new KAction(KIcon(QLatin1String("go-previous-view")), i18n("&Previous Tab"), this);
    previousTabAction->setShortcuts(KStandardShortcut::tabPrev());
    connect(previousTabAction, SIGNAL(triggered()), this, SLOT(onPreviousTabActionTriggered()));

    KAction *audioCallAction = new KAction(KIcon(QLatin1String("audio-headset")), i18n("&Audio Call"), this);
    audioCallAction->setToolTip(i18nc("Toolbar icon tooltip", "Start an audio call with this contact"));
    connect(audioCallAction, SIGNAL(triggered()), this, SLOT(onAudioCallTriggered()));

    KAction *blockContactAction = new KAction(KIcon(QLatin1String("im-ban-kick-user")), i18n("&Block Contact"), this);
    blockContactAction->setToolTip(i18nc("Toolbar icon tooltip",
                                         "Blocking means that this contact will not see you online and you will not receive any messages from this contact"));
    connect(blockContactAction, SIGNAL(triggered()), this, SLOT(onBlockContactTriggered()));

    KAction *fileTransferAction = new KAction(KIcon(QLatin1String("mail-attachment")), i18n("&Send File"), this);
    fileTransferAction->setToolTip(i18nc("Toolbar icon tooltip", "Send a file to this contact"));
    connect(fileTransferAction, SIGNAL(triggered()), this, SLOT(onFileTransferTriggered()));

    KAction *inviteToChat = new KAction(KIcon(QLatin1String("user-group-new")), i18n("&Invite to Chat"), this);
    inviteToChat->setToolTip(i18nc("Toolbar icon tooltip", "Invite another contact to join this chat"));
    connect(inviteToChat, SIGNAL(triggered()), this, SLOT(onInviteToChatTriggered()));

    KAction *videoCallAction = new KAction(KIcon(QLatin1String("camera-web")), i18n("&Video Call"), this);
    videoCallAction->setToolTip(i18nc("Toolbar icon tooltip", "Start a video call with this contact"));
    connect(videoCallAction, SIGNAL(triggered()), this, SLOT(onVideoCallTriggered()));

    KAction *shareDesktopAction = new KAction(KIcon(QLatin1String("krfb")), i18n("Share My &Desktop"), this);
    shareDesktopAction->setToolTip(i18nc("Toolbar icon tooltip", "Start an application that allows this contact to see your desktop"));
    connect(shareDesktopAction, SIGNAL(triggered()), this, SLOT(onShareDesktopTriggered()));

    m_spellDictCombo = new Sonnet::DictionaryComboBox();
    connect(m_spellDictCombo, SIGNAL(dictionaryChanged(QString)),
            this, SLOT(setTabSpellDictionary(QString)));

    QWidgetAction *spellDictComboAction = new QWidgetAction(this);
    spellDictComboAction->setDefaultWidget(m_spellDictCombo);
    spellDictComboAction->defaultWidget()->setFocusPolicy(Qt::ClickFocus);
    spellDictComboAction->setIcon(KIcon(QLatin1String("tools-check-spelling")));
    spellDictComboAction->setIconText(i18n("Choose Spelling Language"));

#ifdef TELEPATHY_LOGGER_QT4_FOUND
    KAction *openLogAction = new KAction(KIcon(QLatin1String("view-pim-journal")), i18nc("Action to open the log viewer with a specified contact","&Previous Conversations"), this);
    connect(openLogAction, SIGNAL(triggered()), SLOT(onOpenLogTriggered()));
#endif

    KAction *accountIconAction = new KAction(KIcon(QLatin1String("telepathy-kde")), i18n("Account Icon"), this);
    m_accountIconLabel = new QLabel(this);
    accountIconAction->setDefaultWidget(m_accountIconLabel);

    // add custom actions to the collection
    actionCollection()->addAction(QLatin1String("next-tab"), nextTabAction);
    actionCollection()->addAction(QLatin1String("previous-tab"), previousTabAction);
    actionCollection()->addAction(QLatin1String("audio-call"), audioCallAction);
    actionCollection()->addAction(QLatin1String("send-file"), fileTransferAction);
    actionCollection()->addAction(QLatin1String("video-call"), videoCallAction);
    actionCollection()->addAction(QLatin1String("invite-to-chat"), inviteToChat);
    actionCollection()->addAction(QLatin1String("share-desktop"), shareDesktopAction);
    actionCollection()->addAction(QLatin1String("language"), spellDictComboAction);
    actionCollection()->addAction(QLatin1String("account-icon"), accountIconAction);
    actionCollection()->addAction(QLatin1String("block-contact"), blockContactAction);
#ifdef TELEPATHY_LOGGER_QT4_FOUND
    actionCollection()->addAction(QLatin1String("open-log"), openLogAction);
#endif
}

void ChatWindow::setAudioCallEnabled(bool enable)
{
    QAction *action = actionCollection()->action(QLatin1String("audio-call"));

    // don't want to segfault. Should never happen
    if (action) {
        action->setEnabled(enable);
    }
}

void ChatWindow::setBlockEnabled(bool enable)
{
    QAction *action = actionCollection()->action(QLatin1String("block-contact"));

    if (action) {
        action->setEnabled(enable);
    }
}

void ChatWindow::setFileTransferEnabled(bool enable)
{
    QAction *action = actionCollection()->action(QLatin1String("send-file"));

    if (action) {
        action->setEnabled(enable);
    }
}

void ChatWindow::setInviteToChatEnabled(bool enable)
{
    QAction *action = actionCollection()->action(QLatin1String("invite-to-chat"));

    if (action) {
        action->setEnabled(enable);
    }
}

void ChatWindow::setVideoCallEnabled(bool enable)
{
    QAction *action = actionCollection()->action(QLatin1String("video-call"));

    if (action) {
        action->setEnabled(enable);
    }
}

void ChatWindow::setShareDesktopEnabled(bool enable)
{
    QAction *action = actionCollection()->action(QLatin1String("share-desktop"));

    if (action) {
        action->setEnabled(enable);
    }
}

void ChatWindow::setPreviousConversationsEnabled ( bool enable )
{
    QAction *action = actionCollection()->action(QLatin1String("open-log"));

    if (action) {
        action->setEnabled(enable);
    }
}

void ChatWindow::setAccountIcon(const QIcon &icon)
{
    m_accountIconLabel->setPixmap(icon.pixmap(toolBar()->iconSize()));
}

void ChatWindow::startAudioCall(const Tp::AccountPtr& account, const Tp::ContactPtr& contact)
{
    Tp::PendingChannelRequest *channelRequest = account->ensureAudioCall(contact,
                                                                         QLatin1String("audio"),
                                                                         QDateTime::currentDateTime(),
                                                                         QLatin1String(PREFERRED_AUDIO_VIDEO_HANDLER));

    connect(channelRequest, SIGNAL(finished(Tp::PendingOperation*)), this, SLOT(onGenericOperationFinished(Tp::PendingOperation*)));
}

void ChatWindow::startFileTransfer(const Tp::AccountPtr& account, const Tp::ContactPtr& contact)
{
    // check for existence of ContactPtr
    Q_ASSERT(contact);

    // use the keyword "FileTransferLastDirectory" for setting last used dir for file transfer
    QStringList fileNames = KFileDialog::getOpenFileNames(KUrl("kfiledialog:///FileTransferLastDirectory"),
                                                          QString(),
                                                          this,
                                                          i18n("Choose files to send to %1", contact->alias()));

    // User hit cancel button
    if (fileNames.isEmpty()) {
        return;
    }

    QDateTime now = QDateTime::currentDateTime();
    Q_FOREACH(const QString& fileName, fileNames) {
        QFileInfo fileinfo(fileName);

        kDebug() << "Filename:" << fileName;
        kDebug() << "Content type:" << KMimeType::findByFileContent(fileName)->name();

        Tp::FileTransferChannelCreationProperties fileTransferProperties(fileName, KMimeType::findByFileContent(fileName)->name());

        Tp::PendingChannelRequest* channelRequest = account->createFileTransfer(contact,
                                                                                fileTransferProperties,
                                                                                QDateTime::currentDateTime(),
                                                                                QLatin1String(PREFERRED_FILETRANSFER_HANDLER));

        connect(channelRequest, SIGNAL(finished(Tp::PendingOperation*)), SLOT(onGenericOperationFinished(Tp::PendingOperation*)));
    }
}

void ChatWindow::startVideoCall(const Tp::AccountPtr& account, const Tp::ContactPtr& contact)
{
    Tp::PendingChannelRequest* channelRequest = account->ensureAudioVideoCall(contact,
                                                                              QLatin1String("audio"),
                                                                              QLatin1String("video"),
                                                                              QDateTime::currentDateTime(),
                                                                              QLatin1String(PREFERRED_AUDIO_VIDEO_HANDLER));

    connect(channelRequest, SIGNAL(finished(Tp::PendingOperation*)), this, SLOT(onGenericOperationFinished(Tp::PendingOperation*)));
}

void ChatWindow::startShareDesktop(const Tp::AccountPtr& account, const Tp::ContactPtr& contact)
{
    Tp::PendingChannelRequest* channelRequest = account->createStreamTube(contact,
                                                                          QLatin1String("rfb"),
                                                                          QDateTime::currentDateTime(),
                                                                          QLatin1String(PREFERRED_RFB_HANDLER));

    connect(channelRequest, SIGNAL(finished(Tp::PendingOperation*)), this, SLOT(onGenericOperationFinished(Tp::PendingOperation*)));
}

void ChatWindow::onUserTypingChanged(Tp::ChannelChatState state)
{
    ChatWidget *currChat =  qobject_cast<ChatWidget*>(m_tabWidget->currentWidget());
    Q_ASSERT(currChat);
    QString title = currChat->title();

    if (state == Tp::ChannelChatStateComposing) {
        setWindowTitle(i18nc("String prepended in window title, arg is contact's name", "Typing... %1", title));
    } else if (state == Tp::ChannelChatStatePaused) {
        setWindowTitle(i18nc("String appended in window title, arg is contact's name", "%1 has entered text", title));
    } else {
        setWindowTitle(title);
    }
}

bool ChatWindow::event(QEvent *e)
{
    if (e->type() == QEvent::WindowActivate) {
        //when the window is activated reset the message count on the active tab.
        ChatWidget *currChat =  qobject_cast<ChatWidget*>(m_tabWidget->currentWidget());
        Q_ASSERT(currChat);
        currChat->acknowledgeMessages();
    }

    return KXmlGuiWindow::event(e);
}

void ChatWindow::setTabSpellDictionary(const QString &dict)
{
    int index = m_tabWidget->currentIndex();
    ChatTab* currentChatTab=qobject_cast<ChatTab*>(m_tabWidget->widget(index));
    currentChatTab->setSpellDictionary(dict);
}

void ChatWindow::toggleBlockButton(bool contactIsBlocked)
{
    QAction *action = actionCollection()->action(QLatin1String("block-contact"));
    if(contactIsBlocked) {
        //Change the name of the action to "Unblock Contact"
        //and disconnect it with the block slot and reconnect it with unblock slot
        disconnect(action, SIGNAL(triggered()), this, SLOT(onBlockContactTriggered()));
        action->setText(i18n("&Unblock Contact"));

        connect(action, SIGNAL(triggered()), this, SLOT(onUnblockContactTriggered()));
    } else {
        //Change the name of the action to "Block Contact"
        //and disconnect it with the unblock slot and reconnect it with block slot
        disconnect(action, SIGNAL(triggered()), this, SLOT(onUnblockContactTriggered()));
        action->setText(i18n("&Block Contact"));

        connect(action, SIGNAL(triggered()), this, SLOT(onBlockContactTriggered()));
    }
    //Reset the WindowTitle
    ChatWidget *currChat =  qobject_cast<ChatWidget*>(m_tabWidget->currentWidget());
    Q_ASSERT(currChat);
    setWindowTitle(currChat->title());

    setBlockEnabled(true);
}


#include "chat-window.moc"
