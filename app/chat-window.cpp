/*
    Copyright (C) 2010  David Edmundson   <kde@davidedmundson.co.uk>
    Copyright (C) 2011  Dominik Schmidt   <dev@dominik-schmidt.de>
    Copyright (C) 2011  Francesco Nwokeka <francesco.nwokeka@gmail.com>
    Copyright (C) 2014  Marcin Ziemiński   <zieminn@gmail.com>

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
#include <KTp/actions.h>
#include <KTp/contact.h>

#include <KStandardAction>
#include <KLocalizedString>
#include <KActionCollection>
#include <KColorScheme>
#include <ksettings/dialog.h>
#include <kcmodulecontainer.h>
#include <KNotification>
#include <KNotifyConfigWidget>
#include <KToolBar>
#include <KToolInvocation>
#include <KCModuleProxy>
#include <KIconLoader>

#include <QApplication>
#include <QFileDialog>
#include <QUrl>
#include <QMenu>
#include <QMenuBar>
#include <QTabBar>
#include <QAction>
#include <QEvent>
#include <QWidgetAction>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>

#include <TelepathyQt/Account>
#include <TelepathyQt/ContactCapabilities>
#include <TelepathyQt/PendingChannelRequest>
#include <TelepathyQt/TextChannel>

#include <Sonnet/DictionaryComboBox>

#include "emoticon-text-edit-action.h"
#include "invite-contact-dialog.h"
#include "telepathy-chat-ui.h"
#include "text-chat-config.h"

#ifdef HAVE_KPEOPLE
#include <kpeople/widgets/persondetailsdialog.h>
#include <kpeople/global.h>
#include <kpeople/persondata.h>
#endif

#include <KTp/Widgets/contact-info-dialog.h>
#include <KTp/OTR/constants.h>

#define PREFERRED_RFB_HANDLER "org.freedesktop.Telepathy.Client.krfb_rfb_handler"

// FIXME
// As of Qt 5.4 there's no way to get middle mouse click
// event other than reimplementing mouseReleaseEvent
// Remove this class once Qt has something for this
class MiddleMouseButtonHandler : public QTabWidget
{
    Q_OBJECT
public:
    MiddleMouseButtonHandler(QWidget *parent);

Q_SIGNALS:
    void mouseMiddleClick(int index);

protected:
    virtual void mouseReleaseEvent(QMouseEvent *event);
};

MiddleMouseButtonHandler::MiddleMouseButtonHandler(QWidget *parent)
    : QTabWidget(parent)
{
}

void MiddleMouseButtonHandler::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton) {
        Q_EMIT mouseMiddleClick(tabBar()->tabAt(event->pos()));
    }

    QTabWidget::mouseReleaseEvent(event);
}

//------------------------------------------------------------------------------

Q_GLOBAL_STATIC_WITH_ARGS(KTp::ServiceAvailabilityChecker, s_krfbAvailableChecker,
                          (QLatin1String(PREFERRED_RFB_HANDLER)));

ChatWindow::ChatWindow()
    : m_sendMessage(0),
      m_tabWidget(0),
      m_keyboardLayoutInterface(0),
      m_otrActionMenu(0),
      m_proxyService(
              new ProxyService(QDBusConnection::sessionBus(),
                  KTP_PROXY_BUS_NAME,
                  KTP_PROXY_SERVICE_OBJECT_PATH,
                  this))
{
    //This effectively constructs the s_krfbAvailableChecker object the first
    //time that this code is executed. This is to start the d-bus query early, so
    //that data are available when we need them later in desktopSharingCapability()
    (void) s_krfbAvailableChecker.operator->();

    KConfig config(QLatin1String("ktelepathyrc"));
    KConfigGroup group = config.group("Appearance");
    m_zoomFactor = group.readEntry("zoomFactor", (qreal) 1.0);

    //setup actions
    KStandardAction::close(this,SLOT(closeCurrentTab()),actionCollection());
    KStandardAction::quit(qApp, SLOT(quit()), actionCollection());
    KStandardAction::preferences(this, SLOT(showSettingsDialog()), actionCollection());
    KStandardAction::configureNotifications(this, SLOT(showNotificationsDialog()), actionCollection());
    KStandardAction::showMenubar(this->menuBar(), SLOT(setVisible(bool)), actionCollection());

    // keyboard shortcuts for the search bar
    KStandardAction::find(this, SLOT(onSearchActionToggled()), actionCollection());

    // start disabled
    KStandardAction::findNext(this, SLOT(onFindNextText()), actionCollection())->setEnabled(false);
    KStandardAction::findPrev(this, SLOT(onFindPreviousText()), actionCollection())->setEnabled(false);

    KStandardAction::zoomIn(this, SLOT(onZoomIn()), actionCollection());
    KStandardAction::zoomOut(this, SLOT(onZoomOut()), actionCollection());

    m_keyboardLayoutInterface = new QDBusInterface(QLatin1String("org.kde.keyboard"), QLatin1String("/Layouts"),
                                                   QLatin1String("org.kde.KeyboardLayouts"), QDBusConnection::sessionBus(), this);

    connect(m_keyboardLayoutInterface, SIGNAL(currentLayoutChanged(QString)), this, SLOT(onKeyboardLayoutChange(QString)));

    // set up m_tabWidget
    m_tabWidget = new MiddleMouseButtonHandler(this);
    //clicking on the close button can result in the tab bar getting focus, this works round that
    m_tabWidget->setFocusPolicy(Qt::TabFocus);
    m_tabWidget->setMovable(true);
    m_tabWidget->setDocumentMode(true);
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    m_tabWidget->tabBar()->hide();
    m_tabWidget->setElideMode(Qt::ElideRight);

    connect(m_tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(destroyTab(int)));
    connect(m_tabWidget, SIGNAL(currentChanged(int)), this, SLOT(onCurrentIndexChanged(int)));

    connect(m_tabWidget, SIGNAL(mouseMiddleClick(int)), this, SLOT(onTabMiddleClicked(int)));
    connect(qobject_cast<QTabBar*>(m_tabWidget->tabBar()), SIGNAL(customContextMenuRequested(QPoint)), SLOT(tabBarContextMenu(QPoint)));

    setCentralWidget(m_tabWidget);

    // create custom actions
    // we must do it AFTER m_tabWidget is set up
    setupCustomActions();

    // start otr service and create otr actions for otr popup menu
    setupOTR();

    setupGUI(QSize(460, 440), static_cast<StandardWindowOptions>(Default^StatusBar), QLatin1String("chatwindow.rc"));

    // Connects the toolbars iconSizeChanged to the custom toolbar item
    // NOTE Must be called after setupGUI or the toolbars won't be created
    Q_FOREACH(KToolBar *toolbar, toolBars()) {
        connect(toolbar, SIGNAL(iconSizeChanged(const QSize&)), SLOT(updateAccountIcon()));
    }
}

ChatWindow::~ChatWindow()
{
    Q_EMIT aboutToClose(this);
}

void ChatWindow::tabBarContextMenu(const QPoint &globalPos)
{
    int index = m_tabWidget->tabBar()->tabAt(globalPos);
    QAction close(QIcon::fromTheme(QStringLiteral("tab-close")), i18n("Close"), this);
    QAction dettach(QIcon::fromTheme(QStringLiteral("tab-detach")), i18n("Detach Tab"), this);
    QAction moveLeft(QIcon::fromTheme(QStringLiteral("arrow-left")), i18n("Move Tab Left"), this);
    QAction moveRight(QIcon::fromTheme(QStringLiteral("arrow-right")), i18n("Move Tab Right"), this);

    QMenu *menu = new QMenu(this);

    menu->addAction(&moveLeft);
    menu->addAction(&moveRight);
    menu->addAction(&dettach);
    menu->addAction(&close);

    if (index == 0) {
        moveLeft.setEnabled(false);
    } else if (index == (m_tabWidget->count() - 1)) {
        moveRight.setEnabled(false);
    }

    QAction *result = qobject_cast<QAction*>(menu->exec(m_tabWidget->tabBar()->mapToGlobal(globalPos)));

    if(result == &close) {
        destroyTab(index);
    } else if (result == &dettach) {
        Q_EMIT detachRequested(qobject_cast<ChatTab*>(m_tabWidget->widget(index)));
    } else if (result == &moveLeft) {
        m_tabWidget->tabBar()->moveTab(index, index - 1);
    } else if (result == &moveRight) {
        m_tabWidget->tabBar()->moveTab(index, index + 1);
    }
}

void ChatWindow::focusChat(ChatTab *tab)
{
    m_tabWidget->setCurrentWidget(tab);
}

ChatTab* ChatWindow::getTab(const Tp::AccountPtr &account, const Tp::TextChannelPtr &incomingTextChannel)
{
    ChatTab *match = 0;

    // if targetHandle is None, targetId is also "", therefore we won't be able to find it.
    if (incomingTextChannel->targetHandleType() != Tp::HandleTypeNone) {

        //loop through all tabs checking for matches
        for (int index = 0; index < m_tabWidget->count() && !match; ++index) {

            // get chatWidget object
            ChatTab *auxChatTab = qobject_cast<ChatTab*>(m_tabWidget->widget(index));

            Q_ASSERT(auxChatTab);

            // check for duplicate chat
            if (auxChatTab->account() == account
                && auxChatTab->textChannel()->targetId() == incomingTextChannel->targetId()
                && auxChatTab->textChannel()->targetHandleType() == incomingTextChannel->targetHandleType()) {
                match = auxChatTab;
            }
        }
    }
    return match;
}

ChatTab *ChatWindow::getCurrentTab()
{
    return qobject_cast<ChatTab*>(m_tabWidget->currentWidget());

}

QList<ChatTab*> ChatWindow::tabs() const
{
    QList<ChatTab*> tabs;
    tabs.reserve(m_tabWidget->count());
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        tabs << qobject_cast<ChatTab*>(m_tabWidget->widget(i));
    }
    return tabs;
}

void ChatWindow::removeTab(ChatTab *tab)
{
    tab->stopOtrSession();
    removeChatTabSignals(tab);

    m_tabWidget->removeTab(m_tabWidget->indexOf(tab));

    if (m_tabWidget->tabBar()->isVisible() && m_tabWidget->count() <= 1) {
        m_tabWidget->tabBar()->hide();
    }
}

void ChatWindow::onTabMiddleClicked(int index)
{
    destroyTab(index);
}

void ChatWindow::addTab(ChatTab *tab)
{
    setupChatTabSignals(tab);
    tab->setZoomFactor(m_zoomFactor);

    QDBusPendingCall dbusPendingCall = m_keyboardLayoutInterface->asyncCall(QLatin1String("getCurrentLayout"));
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(dbusPendingCall, this);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(onGetCurrentKeyboardLayoutFinished(QDBusPendingCallWatcher*)));

    m_tabWidget->addTab(tab, tab->icon(), tab->title());
    m_tabWidget->setCurrentWidget(tab);
    m_tabWidget->setTabToolTip(m_tabWidget->indexOf(tab), tab->title());

    if (!m_tabWidget->tabBar()->isVisible() && m_tabWidget->count() > 1) {
        m_tabWidget->tabBar()->show();
    }

    tab->setFocus();
    tab->updateSendMessageShortcuts(m_sendMessage->shortcut());
    // block text input if key is being generated for this account
    if(m_proxyService->isOngoingGeneration(QDBusObjectPath(tab->account()->objectPath()))) {
        tab->blockTextInput(true);
    }
}

void ChatWindow::destroyTab(int index)
{
    ChatTab *tab = qobject_cast<ChatTab*>(m_tabWidget->widget(index));
    Q_ASSERT(tab);

    tab->setChatWindow(0);
    tab->deleteLater();
}

void ChatWindow::setTabText(int index, const QString &newTitle)
{
    m_tabWidget->setTabText(index, newTitle);

    // this updates the window title and icon if the updated tab is the current one
    if (index == m_tabWidget->currentIndex()) {
        onCurrentIndexChanged(index);
    }
}

void ChatWindow::setTabIcon(int index, const QIcon & newIcon)
{
    m_tabWidget->setTabIcon(index, newIcon);

    // this updates the window title and icon if the updated tab is the current one
    if (index == m_tabWidget->currentIndex()) {
        onCurrentIndexChanged(index);
    }
}

void ChatWindow::setTabTextColor(int index, const QColor& color)
{
    m_tabWidget->tabBar()->setTabTextColor(index, color);
}

void ChatWindow::closeCurrentTab()
{
    destroyTab(m_tabWidget->currentIndex());
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
    if (index == -1) {
        close();
        return;
    }

    ChatTab *currentChatTab = qobject_cast<ChatTab*>(m_tabWidget->widget(index));
    currentChatTab->acknowledgeMessages();
    setWindowTitle(currentChatTab->title());
    if (hasUnreadMessages()) {
        setWindowIcon(QIcon::fromTheme(QStringLiteral("mail-mark-unread-new")));
    } else {
        setWindowIcon(currentChatTab->icon());
    }

    m_spellDictCombo->setCurrentByDictionary(currentChatTab->spellDictionary());

    if (currentChatTab->isActiveWindow()) {
	restoreKeyboardLayout(currentChatTab);
    }

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
        KTp::ContactPtr targetContact = KTp::ContactPtr::qObjectCast(currentChatTab->textChannel()->targetContact());

        setAudioCallEnabled(targetContact->audioCallCapability());
        setFileTransferEnabled(targetContact->fileTransferCapability());
        setVideoCallEnabled(targetContact->videoCallCapability());
        setShareDesktopEnabled(targetContact->streamTubeServicesCapability().contains(QLatin1String("rfb")));
        setInviteToChatEnabled(currentChatTab->account()->capabilities().textChatrooms());
        setShowInfoEnabled(true);
        toggleBlockButton(targetContact->isBlocked());
    } else {
        setAudioCallEnabled(false);
        setFileTransferEnabled(false);
        setVideoCallEnabled(false);
        setShareDesktopEnabled(false);
        setInviteToChatEnabled(currentChatTab->account()->capabilities().textChatrooms());
        setBlockEnabled(false);
        setShowInfoEnabled(false);
    }

    onOtrStatusChanged(currentChatTab->otrStatus());

    // Allow "Leaving" rooms only in group chat, and when persistent rooms are enabled
    actionCollection()->action(QLatin1String("leave-chat"))->setEnabled(currentChatTab->isGroupChat() && TextChatConfig::instance()->dontLeaveGroupChats());
    // No point having "Close" action with only one tab, it behaves exactly like "Quit"
    actionCollection()->action(QLatin1String("file_close"))->setVisible(m_tabWidget->count() > 1);

    if ( currentChatTab->account()->connection() ) {
        const QString collab(QLatin1String("infinote"));
        bool selfCanShare = currentChatTab->account()->connection()->selfContact()->capabilities().streamTubes(collab);
        if (currentChatTab->isGroupChat()) {
            // We can always share documents with a group chat if we support the service
            setCollaborateDocumentEnabled(selfCanShare);
        }
        else {
            bool otherCanShare = currentChatTab->textChannel()->targetContact()->capabilities().streamTubes(collab);
            setCollaborateDocumentEnabled(selfCanShare && otherCanShare);
        }
    }

    // only show enable the action if there are actually previous converstations
    setPreviousConversationsEnabled(currentChatTab->previousConversationAvailable());

    updateAccountIcon();
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

void ChatWindow::onGetCurrentKeyboardLayoutFinished(QDBusPendingCallWatcher* watcher)
{
    if (!watcher->isError()) {
	QDBusMessage replyMessage = watcher->reply();
	ChatTab *chatTab = getCurrentTab();
	if (chatTab) {
	    QString layout = replyMessage.arguments().first().toString();
	    chatTab->setCurrentKeyboardLayoutLanguage(layout);
	}
    }
    watcher->deleteLater();
}

void ChatWindow::onInviteToChatTriggered()
{
    ChatTab *currChat = qobject_cast<ChatTab*>(m_tabWidget->currentWidget());

    InviteContactDialog *dialog = new InviteContactDialog(KTp::accountManager(), currChat->account(), currChat->textChannel(), this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void ChatWindow::onKeyboardLayoutChange(const QString& keyboardLayout)
{
    ChatTab *currChat = getCurrentTab();
    if (currChat) {
	// To prevent keyboard layout change when the ChatWindow is minimized or not active
	if (currChat->isActiveWindow()) {
	    currChat->setCurrentKeyboardLayoutLanguage(keyboardLayout);
	}
    }
}

void ChatWindow::onNextTabActionTriggered()
{
    if (m_tabWidget->count() == 1) {
        return;
    }

    int currIndex = m_tabWidget->currentIndex();

    if (currIndex < m_tabWidget->count()-1) {
        m_tabWidget->setCurrentIndex(++currIndex);
    } else {
        m_tabWidget->setCurrentIndex(0);
    }
}

void ChatWindow::onPreviousTabActionTriggered()
{
    if (m_tabWidget->count() == 1) {
        return;
    }

    int currIndex = m_tabWidget->currentIndex();

    if (currIndex > 0) {
        m_tabWidget->setCurrentIndex(--currIndex);
    } else {
        m_tabWidget->setCurrentIndex(m_tabWidget->count()-1);
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

void ChatWindow::onCollaborateDocumentTriggered()
{
    ChatTab *currChat = qobject_cast<ChatTab*>(m_tabWidget->currentWidget());

    if(!currChat) {
        return;
    }
    if (currChat->isGroupChat()) {
        offerDocumentToChatroom(currChat->account(), currChat->textChannel()->targetId());
    }
    else {
        offerDocumentToContact(currChat->account(), currChat->textChannel()->targetContact());
    }
}

void ChatWindow::onTabStateChanged()
{
    QIcon windowIcon;
    ChatTab *sender = qobject_cast<ChatTab*>(QObject::sender());
    if (sender) {
        int tabIndex = m_tabWidget->indexOf(sender);
        setTabTextColor(tabIndex, sender->titleColor());

        if (TextChatConfig::instance()->showOthersTyping() && (sender->remoteChatState() == Tp::ChannelChatStateComposing)) {
            setTabIcon(tabIndex, QIcon::fromTheme(QStringLiteral("document-edit")));
            if (sender == m_tabWidget->currentWidget()) {
                windowIcon = QIcon::fromTheme(QStringLiteral("document-edit"));
            } else {
                windowIcon = qobject_cast<ChatTab*>(m_tabWidget->currentWidget())->icon();
            }
        } else {
            setTabIcon(tabIndex, sender->icon());
            windowIcon = qobject_cast<ChatTab*>(m_tabWidget->currentWidget())->icon();
        }

        if (sender->unreadMessageCount() > 0) {
            setTabIcon(tabIndex, QIcon::fromTheme(QStringLiteral("mail-mark-unread-new")));
        }
    }

    if (hasUnreadMessages()) {
        windowIcon = QIcon::fromTheme(QStringLiteral("mail-mark-unread-new"));
    }

    setWindowIcon(windowIcon);
}

void ChatWindow::onTabIconChanged(const QIcon & newIcon)
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

void ChatWindow::onShowInfoTriggered()
{
    ChatWidget *currChat =  qobject_cast<ChatWidget*>(m_tabWidget->currentWidget());
    const Tp::ContactPtr contact = currChat->textChannel()->targetContact();

    if(!currChat || !currChat->account() || !contact) {
        return;
    }

    if (KTp::kpeopleEnabled()) {
        #ifdef HAVE_KPEOPLE
        QString personId(QLatin1String("ktp://"));
        personId.append(currChat->account()->uniqueIdentifier());
        personId.append(QLatin1String("?"));
        personId.append(contact->id());

        if (!personId.isEmpty()) {
            KPeople::PersonDetailsDialog *view = new KPeople::PersonDetailsDialog(currChat);
            KPeople::PersonData *person = new KPeople::PersonData(personId, view);
            view->setPerson(person);
            view->setAttribute(Qt::WA_DeleteOnClose);
            view->show();
        }
        #endif
    } else {
        KTp::ContactInfoDialog* contactInfoDialog = new KTp::ContactInfoDialog(currChat->account(), contact, currChat);
        contactInfoDialog->setAttribute(Qt::WA_DeleteOnClose);
        contactInfoDialog->show();
    }
}

void ChatWindow::onOpenContactListTriggered()
{
    KToolInvocation::kdeinitExec(QLatin1String("ktp-contactlist"));
}

void ChatWindow::onOpenLogTriggered()
{
    int index = m_tabWidget->currentIndex();
    ChatTab *currentChatTab = qobject_cast<ChatTab*>(m_tabWidget->widget(index));
    Q_ASSERT(currentChatTab);

    Tp::AccountPtr account = currentChatTab->account();
    Tp::ContactPtr contact = currentChatTab->textChannel()->targetContact();

    /* Add "--" before the UIDs so that KCmdLineArgs in ktp-log-viewer does not try to parse
     * UIDs starting with "-" as arguments */
    if (!contact.isNull()) {
        KToolInvocation::kdeinitExec(QLatin1String("ktp-log-viewer"),
                                     QStringList() << QLatin1String("--") << account->uniqueIdentifier() << contact->id());
    } else {
        KToolInvocation::kdeinitExec(QLatin1String("ktp-log-viewer"),
                                     QStringList() << QLatin1String("--") << account->uniqueIdentifier() << currentChatTab->textChannel()->targetId());
    }
}

void ChatWindow::onClearViewTriggered()
{
    ChatWidget *chatWidget = qobject_cast<ChatWidget*>(m_tabWidget->currentWidget());

    if (!chatWidget) {
        return;
    }

    chatWidget->clear();
}

void ChatWindow::showSettingsDialog()
{
    KSettings::Dialog *dialog = new KSettings::Dialog(this);

    KPageWidgetItem *configPage = dialog->addModule(QLatin1String("kcm_ktp_chat_appearance"));
    KCModuleProxy *proxy = qobject_cast<KCModuleProxy*>(configPage->widget());
    Q_ASSERT(proxy);

    connect(proxy->realModule(), SIGNAL(reloadTheme()),
            this, SLOT(onReloadTheme()));

    dialog->addModule(QLatin1String("kcm_ktp_chat_behavior"));
    dialog->addModule(QLatin1String("kcm_ktp_chat_messages"));

    KPageWidgetItem *otrConfigPage = dialog->addModule(QLatin1String("kcm_ktp_chat_otr"));
    proxy = qobject_cast<KCModuleProxy*>(otrConfigPage->widget());
    Q_ASSERT(proxy);
    QVariant value;
    value.setValue(m_proxyService);
    proxy->realModule()->setProperty("proxyService", value);

    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void ChatWindow::showNotificationsDialog()
{
    KNotifyConfigWidget::configure(this, QLatin1String("ktelepathy"));
}

void ChatWindow::removeChatTabSignals(ChatTab *chatTab)
{
    disconnect(chatTab, SIGNAL(titleChanged(QString)), this, SLOT(onTabTextChanged(QString)));
    disconnect(chatTab, SIGNAL(iconChanged(QIcon)), this, SLOT(onTabIconChanged(QIcon)));
    disconnect(chatTab, SIGNAL(unreadMessagesChanged()), this, SLOT(onTabStateChanged()));
    disconnect(chatTab, SIGNAL(contactPresenceChanged(KTp::Presence)), this, SLOT(onTabStateChanged()));
    disconnect(chatTab->chatSearchBar(), SIGNAL(enableSearchButtonsSignal(bool)), this, SLOT(onEnableSearchActions(bool)));
    disconnect(chatTab, SIGNAL(contactBlockStatusChanged(bool)), this, SLOT(toggleBlockButton(bool)));
    if(chatTab->otrStatus())
        disconnect(chatTab, SIGNAL(otrStatusChanged(OtrStatus)), this, SLOT(onOtrStatusChanged(OtrStatus)));
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
    connect(chatTab, SIGNAL(iconChanged(QIcon)), this, SLOT(onTabIconChanged(QIcon)));
    connect(chatTab, SIGNAL(userTypingChanged(Tp::ChannelChatState)), this, SLOT(onTabStateChanged()));
    connect(chatTab, SIGNAL(unreadMessagesChanged()), this, SLOT(onTabStateChanged()));
    connect(chatTab, SIGNAL(contactPresenceChanged(KTp::Presence)), this, SLOT(onTabStateChanged()));
    connect(chatTab->chatSearchBar(), SIGNAL(enableSearchButtonsSignal(bool)), this, SLOT(onEnableSearchActions(bool)));
    connect(chatTab, SIGNAL(contactBlockStatusChanged(bool)), this, SLOT(toggleBlockButton(bool)));
    connect(chatTab, SIGNAL(zoomFactorChanged(qreal)), this, SLOT(onZoomFactorChanged(qreal)));
    if(chatTab->otrStatus())
        connect(chatTab, SIGNAL(otrStatusChanged(OtrStatus)), this, SLOT(onOtrStatusChanged(OtrStatus)));
}

void ChatWindow::setupCustomActions()
{
    KStandardAction::close(this, SLOT(closeCurrentTab()), this);
    KStandardAction::quit(this, SLOT(close()), this);

    QAction *nextTabAction = new QAction(QIcon::fromTheme(QStringLiteral("go-next-view")), i18n("&Next Tab"), this);
    nextTabAction->setShortcuts(KStandardShortcut::tabNext());
    connect(nextTabAction, SIGNAL(triggered()), this, SLOT(onNextTabActionTriggered()));

    QAction *previousTabAction = new QAction(QIcon::fromTheme(QStringLiteral("go-previous-view")), i18n("&Previous Tab"), this);
    previousTabAction->setShortcuts(KStandardShortcut::tabPrev());
    connect(previousTabAction, SIGNAL(triggered()), this, SLOT(onPreviousTabActionTriggered()));

    QAction *audioCallAction = new QAction(QIcon::fromTheme(QStringLiteral("audio-headset")), i18n("&Audio Call"), this);
    audioCallAction->setToolTip(i18nc("Toolbar icon tooltip", "Start an audio call with this contact"));
    connect(audioCallAction, SIGNAL(triggered()), this, SLOT(onAudioCallTriggered()));

    QAction *blockContactAction = new QAction(QIcon::fromTheme(QStringLiteral("im-ban-kick-user")), i18n("&Block Contact"), this);
    blockContactAction->setToolTip(i18nc("Toolbar icon tooltip",
                                         "Blocking means that this contact will not see you online and you will not receive any messages from this contact"));
    connect(blockContactAction, SIGNAL(triggered()), this, SLOT(onBlockContactTriggered()));

    QAction *fileTransferAction = new QAction(QIcon::fromTheme(QStringLiteral("mail-attachment")), i18n("&Send File"), this);
    fileTransferAction->setToolTip(i18nc("Toolbar icon tooltip", "Send a file to this contact"));
    connect(fileTransferAction, SIGNAL(triggered()), this, SLOT(onFileTransferTriggered()));

    QAction *inviteToChat = new QAction(QIcon::fromTheme(QStringLiteral("user-group-new")), i18n("&Invite to Chat"), this);
    inviteToChat->setToolTip(i18nc("Toolbar icon tooltip", "Invite another contact to join this chat"));
    connect(inviteToChat, SIGNAL(triggered()), this, SLOT(onInviteToChatTriggered()));

    QAction *videoCallAction = new QAction(QIcon::fromTheme(QStringLiteral("camera-web")), i18n("&Video Call"), this);
    videoCallAction->setToolTip(i18nc("Toolbar icon tooltip", "Start a video call with this contact"));
    connect(videoCallAction, SIGNAL(triggered()), this, SLOT(onVideoCallTriggered()));

    QAction *shareDesktopAction = new QAction(QIcon::fromTheme(QStringLiteral("krfb")), i18n("Share My &Desktop"), this);
    shareDesktopAction->setToolTip(i18nc("Toolbar icon tooltip", "Start an application that allows this contact to see your desktop"));
    connect(shareDesktopAction, SIGNAL(triggered()), this, SLOT(onShareDesktopTriggered()));

    QAction* collaborateDocumentAction = new QAction(QIcon::fromTheme(QStringLiteral("document-share")), i18n("&Collaboratively edit a document"), this);
    connect(collaborateDocumentAction, SIGNAL(triggered()), this, SLOT(onCollaborateDocumentTriggered()));

    QAction* showInfoAction = new QAction(QIcon::fromTheme(QStringLiteral("view-pim-contacts")), i18n("&Contact info"), this);
    connect(showInfoAction, SIGNAL(triggered()), this, SLOT(onShowInfoTriggered()));

    QAction* leaveAction = new QAction(QIcon::fromTheme(QStringLiteral("irc-close-channel")), i18n("&Leave room"), this);
    connect(leaveAction, SIGNAL(triggered()), this, SLOT(onLeaveChannelTriggered()));

    m_spellDictCombo = new Sonnet::DictionaryComboBox();
    connect(m_spellDictCombo, SIGNAL(dictionaryChanged(QString)),
            this, SLOT(setTabSpellDictionary(QString)));

    QWidgetAction *spellDictComboAction = new QWidgetAction(this);
    spellDictComboAction->setDefaultWidget(m_spellDictCombo);
    spellDictComboAction->defaultWidget()->setFocusPolicy(Qt::ClickFocus);
    spellDictComboAction->setIcon(QIcon::fromTheme(QStringLiteral("tools-check-spelling")));
    spellDictComboAction->setIconText(i18n("Choose Spelling Language"));

    QAction *openLogAction = new QAction(QIcon::fromTheme(QStringLiteral("view-pim-journal")), i18nc("Action to open the log viewer with a specified contact","&Previous Conversations"), this);
    connect(openLogAction, SIGNAL(triggered()), SLOT(onOpenLogTriggered()));

    QAction *openContactListAction = new QAction(QIcon::fromTheme(QStringLiteral("telepathy-kde")), i18nc("Action to open the contact list","Contact &List"), this);
    connect(openContactListAction, SIGNAL(triggered()), SLOT(onOpenContactListTriggered()));

    QWidgetAction *accountIconAction = new QWidgetAction(this);
    accountIconAction->setIcon(QIcon::fromTheme(QStringLiteral("telepathy-kde")));
    accountIconAction->setText(i18n("Account Icon"));
    m_accountIconLabel = new QLabel(this);
    accountIconAction->setDefaultWidget(m_accountIconLabel);

    QAction *clearViewAction = new QAction(QIcon::fromTheme(QStringLiteral("edit-clear-history")), i18n("&Clear View"), this);
    clearViewAction->setToolTip(i18nc("Toolbar icon tooltip", "Clear all messages from current chat tab"));
    connect(clearViewAction, SIGNAL(triggered()), this, SLOT(onClearViewTriggered()));

    EmoticonTextEditAction *addEmoticonAction = new EmoticonTextEditAction(this);
    connect(addEmoticonAction, SIGNAL(emoticonActivated(QString)), this, SLOT(onAddEmoticon(QString)) );

    m_sendMessage = new QAction(i18n("Send message"), this);

    actionCollection()->setDefaultShortcuts(m_sendMessage,
                // Setting default shortcuts. Return will be a primary one, and Enter (on keypad) - alternate.
                QList<QKeySequence>() << QKeySequence(Qt::Key_Return) << QKeySequence(Qt::Key_Enter));
    connect(m_sendMessage, SIGNAL(changed()), SLOT(updateSendMessageShortcuts()));

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
    actionCollection()->addAction(QLatin1String("open-log"), openLogAction);
    actionCollection()->addAction(QLatin1String("open-contact-list"), openContactListAction);
    actionCollection()->addAction(QLatin1String("clear-chat-view"), clearViewAction);
    actionCollection()->addAction(QLatin1String("emoticons"), addEmoticonAction);
    actionCollection()->addAction(QLatin1String("send-message"), m_sendMessage);
    actionCollection()->addAction(QLatin1String("collaborate-document"), collaborateDocumentAction);
    actionCollection()->addAction(QLatin1String("contact-info"), showInfoAction);
    actionCollection()->addAction(QLatin1String("leave-chat"), leaveAction);
}


void ChatWindow::setupOTR()
{
    m_otrActionMenu = new KActionMenu(QIcon::fromTheme(QStringLiteral("object-unlocked")), i18n("&OTR"), this);
    m_otrActionMenu->setDelayed(false);

    QAction *startRestartOtrAction = new QAction(QIcon::fromTheme(QStringLiteral("object-locked")), i18n("&Start session"), this);
    startRestartOtrAction->setEnabled(false);
    connect(startRestartOtrAction, SIGNAL(triggered()), this, SLOT(onStartRestartOtrTriggered()));

    QAction *stopOtrAction = new QAction(QIcon::fromTheme(QStringLiteral("object-unlocked")), i18n("&Stop session"), this);
    stopOtrAction->setEnabled(false);
    connect(stopOtrAction, SIGNAL(triggered()), this, SLOT(onStopOtrTriggered()));

    QAction *authenticateBuddyAction = new QAction(QIcon::fromTheme(QStringLiteral("application-pgp-signature")), i18n("&Authenticate contact"), this);
    authenticateBuddyAction->setEnabled(false);
    connect(authenticateBuddyAction, SIGNAL(triggered()), this, SLOT(onAuthenticateBuddyTriggered()));

    m_otrActionMenu->addAction(startRestartOtrAction);
    m_otrActionMenu->addAction(stopOtrAction);
    m_otrActionMenu->addAction(authenticateBuddyAction);
    m_otrActionMenu->setEnabled(false);

    actionCollection()->addAction(QLatin1String("start-restart-otr"), startRestartOtrAction);
    actionCollection()->addAction(QLatin1String("stop-otr"), stopOtrAction);
    actionCollection()->addAction(QLatin1String("authenticate-otr"), authenticateBuddyAction);
    actionCollection()->addAction(QLatin1String("otr-actions"), m_otrActionMenu);

    // private key generation
    connect(m_proxyService.data(), SIGNAL(keyGenerationStarted(Tp::AccountPtr)),
            SLOT(onKeyGenerationStarted(Tp::AccountPtr)));
    connect(m_proxyService.data(), SIGNAL(keyGenerationFinished(Tp::AccountPtr, bool)),
            SLOT(onKeyGenerationFinished(Tp::AccountPtr, bool)));
}

void ChatWindow::onOtrStatusChanged(OtrStatus status)
{

    ChatWidget *chatTab = dynamic_cast<ChatWidget*>(QObject::sender());
    // in case if this slot is called directly, not by the signal
    if(!chatTab) {
        chatTab = getCurrentTab();
    }

    if(chatTab != getCurrentTab()) {
        return;
    }

    // OTR is disabled for this channel
    if(!status) {
        m_otrActionMenu->setEnabled(false);
        m_otrActionMenu->menu()->setIcon(QIcon::fromTheme(QStringLiteral("object-unlocked")));
        return;
    }

    QAction* srAction = actionCollection()->action(QLatin1String("start-restart-otr"));
    QAction* stopAction = actionCollection()->action(QLatin1String("stop-otr"));
    QAction* authenticateBuddyAction = actionCollection()->action(QLatin1String("authenticate-otr"));

    m_otrActionMenu->setEnabled(true);

    switch(status.otrTrustLevel()) {

        case KTp::OTRTrustLevelNotPrivate:
            m_otrActionMenu->setIcon(QIcon::fromTheme(QStringLiteral("object-unlocked")));
            m_otrActionMenu->setToolTip(i18n("Not private"));
            srAction->setEnabled(true);
            srAction->setText(i18n("&Start session"));
            stopAction->setEnabled(false);
            authenticateBuddyAction->setEnabled(false);
            return;

        case KTp::OTRTrustLevelUnverified:
            m_otrActionMenu->setIcon(QIcon::fromTheme(QStringLiteral("object-locked-unverified")));
            m_otrActionMenu->setToolTip(i18n("Unverified"));
            srAction->setEnabled(true);
            srAction->setText(i18n("&Restart session"));
            stopAction->setEnabled(true);
            authenticateBuddyAction->setEnabled(true);
            return;

        case KTp::OTRTrustLevelPrivate:
            m_otrActionMenu->setIcon(QIcon::fromTheme(QStringLiteral("object-locked-verified")));
            m_otrActionMenu->setToolTip(i18n("Private"));
            srAction->setEnabled(true);
            srAction->setText(i18n("&Restart session"));
            stopAction->setEnabled(true);
            authenticateBuddyAction->setEnabled(true);
            return;

        case KTp::OTRTrustLevelFinished:
            m_otrActionMenu->setIcon(QIcon::fromTheme(QStringLiteral("object-locked-finished")));
            m_otrActionMenu->setToolTip(i18n("Finished"));
            srAction->setEnabled(true);
            srAction->setText(i18n("&Restart session"));
            stopAction->setEnabled(true);
            authenticateBuddyAction->setEnabled(false);
            return;

        default: return;
    }
}

void ChatWindow::onStartRestartOtrTriggered()
{

    ChatTab* chat = getCurrentTab();
    chat->startOtrSession();
}

void ChatWindow::onStopOtrTriggered() {

    ChatTab* chat = getCurrentTab();
    chat->stopOtrSession();
}

void ChatWindow::onAuthenticateBuddyTriggered()
{

    ChatTab* chat = getCurrentTab();
    chat->authenticateBuddy();
}

void ChatWindow::onKeyGenerationStarted(Tp::AccountPtr account)
{
    // block user text input for these tabs
    QList<ChatTab*> allTabs = tabs();
    Q_FOREACH(ChatTab *ct, allTabs) {
        if(ct->account()->objectPath() == account->objectPath()) {
            ct->blockTextInput(true);
        }
    }
}

void ChatWindow::onKeyGenerationFinished(Tp::AccountPtr account, bool error)
{
    Q_UNUSED(error);
    // unblock user text input for these tabs
    QList<ChatTab*> allTabs = tabs();
    Q_FOREACH(ChatTab *ct, allTabs) {
        if(ct->account()->objectPath() == account->objectPath()) {
            ct->blockTextInput(false);
        }
    }
}

void ChatWindow::setCollaborateDocumentEnabled(bool enable)
{
    QAction* action = actionCollection()->action(QLatin1String("collaborate-document"));

    if (action) {
        action->setEnabled(enable);
        if ( enable ) {
            action->setToolTip(i18nc("Toolbar icon tooltip", "Edit a plain-text document with this contact in real-time"));
        }
        else {
            action->setToolTip(i18nc("Toolbar icon tooltip for a disabled action", "<p>Both you and the target contact "
                                     "need to have the <i>kte-collaborative</i> package installed to share documents</p>"));
        }
    }
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

void ChatWindow::setShowInfoEnabled ( bool enable )
{
    QAction *action = actionCollection()->action(QLatin1String("contact-info"));

    if (action) {
        action->setEnabled(enable);
    }
}

void ChatWindow::updateAccountIcon()
{
    int index = m_tabWidget->currentIndex();
    ChatTab *currentChatTab = qobject_cast<ChatTab*>(m_tabWidget->widget(index));
    m_accountIconLabel->setPixmap(currentChatTab->accountIcon().pixmap(toolBar()->iconSize()));
}

void ChatWindow::startAudioCall(const Tp::AccountPtr& account, const Tp::ContactPtr& contact)
{
    Tp::PendingChannelRequest *channelRequest = KTp::Actions::startAudioCall(account, contact);
    connect(channelRequest, SIGNAL(finished(Tp::PendingOperation*)), this, SLOT(onGenericOperationFinished(Tp::PendingOperation*)));
}

void ChatWindow::startFileTransfer(const Tp::AccountPtr& account, const Tp::ContactPtr& contact)
{
    // check for existence of ContactPtr
    Q_ASSERT(contact);

    // use the keyword "FileTransferLastDirectory" for setting last used dir for file transfer
    QStringList fileNames = QFileDialog::getOpenFileNames(this,
                                                          i18n("Choose files to send to %1", contact->alias()),
                                                          QStringLiteral("kfiledialog:///FileTransferLastDirectory"),
                                                          QString()
                                                          );

    // User hit cancel button
    if (fileNames.isEmpty()) {
        return;
    }

    QDateTime now = QDateTime::currentDateTime();
    Q_FOREACH(const QString& fileName, fileNames) {
        Tp::PendingChannelRequest* channelRequest = KTp::Actions::startFileTransfer(account, contact, fileName);
        connect(channelRequest, SIGNAL(finished(Tp::PendingOperation*)), SLOT(onGenericOperationFinished(Tp::PendingOperation*)));
    }
}

void ChatWindow::offerDocumentToContact(const Tp::AccountPtr& account, const Tp::ContactPtr& targetContact)
{
    const QUrl url = QFileDialog::getOpenFileName();
    if ( ! url.isEmpty() ) {
        KTp::Actions::startCollaborativeEditing(account, targetContact, QList<QUrl>() << url, true);
    }
}

void ChatWindow::offerDocumentToChatroom(const Tp::AccountPtr& account, const QString& roomName)
{
   const QUrl url = QFileDialog::getOpenFileName();
    if ( ! url.isEmpty() ) {
        KTp::Actions::startCollaborativeEditing(account, roomName, QList<QUrl>() << url, true);
    }
}

void ChatWindow::restoreKeyboardLayout(ChatTab *chatTab)
{
    if (!chatTab || !TextChatConfig::instance()->rememberTabKeyboardLayout()) {
        return;
    }

    QString currentKeyboardLayout = chatTab->currentKeyboardLayoutLanguage();
    if (!currentKeyboardLayout.isEmpty()) {
        m_keyboardLayoutInterface->asyncCall(QLatin1String("setLayout"), currentKeyboardLayout);
    }
}

void ChatWindow::startVideoCall(const Tp::AccountPtr& account, const Tp::ContactPtr& contact)
{
    Tp::PendingChannelRequest* channelRequest = KTp::Actions::startAudioVideoCall(account, contact);
    connect(channelRequest, SIGNAL(finished(Tp::PendingOperation*)), this, SLOT(onGenericOperationFinished(Tp::PendingOperation*)));
}

void ChatWindow::startShareDesktop(const Tp::AccountPtr& account, const Tp::ContactPtr& contact)
{
    Tp::PendingChannelRequest* channelRequest = KTp::Actions::startDesktopSharing(account, contact);
    connect(channelRequest, SIGNAL(finished(Tp::PendingOperation*)), this, SLOT(onGenericOperationFinished(Tp::PendingOperation*)));
}

bool ChatWindow::event(QEvent *e)
{
    if (e->type() == QEvent::WindowActivate) {
        //when the window is activated reset the message count on the active tab.
        ChatTab *currChat =  qobject_cast<ChatTab*>(m_tabWidget->currentWidget());
        //it is (apparently) possible to get a window activation event whilst we're closing down and have no tabs
        //see https://bugs.kde.org/show_bug.cgi?id=322135
        if (currChat) {
            currChat->acknowledgeMessages();
	    restoreKeyboardLayout(currChat);
        }
    }

    return KXmlGuiWindow::event(e);
}

void ChatWindow::setTabSpellDictionary(const QString &dict)
{
    int index = m_tabWidget->currentIndex();
    ChatTab *currentChatTab=qobject_cast<ChatTab*>(m_tabWidget->widget(index));
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

void ChatWindow::onAddEmoticon(const QString& emoticon)
{
    int index = m_tabWidget->currentIndex();
    ChatTab *currentChatTab = qobject_cast<ChatTab*>(m_tabWidget->widget(index));
    currentChatTab->addEmoticonToChat(emoticon);
}

bool ChatWindow::hasUnreadMessages() const
{
    for (int i = 0; i < m_tabWidget->count(); i++) {
        ChatTab *tab = qobject_cast<ChatTab*>(m_tabWidget->widget(i));
        if (tab && tab->unreadMessageCount() > 0) {
            return true;
        }
    }

    return false;
}

void ChatWindow::onZoomIn()
{
    onZoomFactorChanged(m_zoomFactor + 0.1);
}

void ChatWindow::onZoomOut()
{
    onZoomFactorChanged(m_zoomFactor - 0.1);
}

void ChatWindow::onZoomFactorChanged(qreal zoom)
{
    m_zoomFactor = zoom;

    for (int i = 0; i < m_tabWidget->count(); i++) {
        ChatWidget *widget = qobject_cast<ChatWidget*>(m_tabWidget->widget(i));
        if (!widget) {
            continue;
        }

        widget->setZoomFactor(zoom);
    }

    KConfig config(QLatin1String("ktelepathyrc"));
    KConfigGroup group = config.group("Appearance");
    group.writeEntry("zoomFactor", m_zoomFactor);
}

void ChatWindow::updateSendMessageShortcuts()
{
    QKeySequence newSendMessageShortcuts = m_sendMessage->shortcut();
    for (int i = 0; i < m_tabWidget->count(); i++) {
        ChatTab* tab = qobject_cast<ChatTab*>(m_tabWidget->widget(i));
        tab->updateSendMessageShortcuts(newSendMessageShortcuts);
    }
}

void ChatWindow::onReloadTheme()
{
    for (int i = 0; i < m_tabWidget->count(); i++) {
        ChatTab *tab = qobject_cast<ChatTab*>(m_tabWidget->widget(i));
        tab->reloadTheme();
    }
}

void ChatWindow::onLeaveChannelTriggered()
{
    ChatTab *tab = getCurrentTab();
    tab->stopOtrSession();
    tab->textChannel()->requestLeave();
    closeCurrentTab();
}

#include "chat-window.moc"
