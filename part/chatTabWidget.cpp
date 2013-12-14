 /*
    Copyright (C) 2010 by David Edmundson <kde@davidedmundson.co.uk>
    Copyright (C) 2013  Daniel Cohen      <analoguecolour@gmail.com>

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

#include "chatTabWidget.h"

#include <KApplication>
#include <KAction>
#include <KLocale>
#include <KActionCollection>
#include <KStandardAction>
#include <KFileDialog>
#include <ktoolinvocation.h>
#include <KTp/actions.h>
#include <KTp/service-availability-checker.h>
#include <KNotification>
#include <TelepathyQt/PendingChannelRequest>

#define PREFERRED_RFB_HANDLER "org.freedesktop.Telepathy.Client.krfb_rfb_handler"

K_GLOBAL_STATIC_WITH_ARGS(KTp::ServiceAvailabilityChecker, s_krfbAvailableChecker,
                          (QLatin1String(PREFERRED_RFB_HANDLER)));

ChatTabWidget::ChatTabWidget(const Tp::TextChannelPtr& channel,
                             const Tp::AccountPtr& account,
                             KXmlGuiWindow* parent)
    : ChatWidget(channel, account, parent)
    , KXMLGUIClient()
{
    //This effectively constructs the s_krfbAvailableChecker object the first
    //time that this code is executed. This is to start the d-bus query early, so
    //that data are available when we need them later in desktopSharingCapability()
    (void) s_krfbAvailableChecker.operator->();

    chatAccount = account;
    chatChannel = channel;
    setupActions();
}

void ChatTabWidget::setupActions()
{
    KAction *openLogAction = new KAction(KIcon(QLatin1String("view-pim-journal")),
                                         i18nc("Action to open the log viewer with a "
                                               "specified contact", "&Previous Conversations"),
                                         this);
    KAction *fileTransferAction = new KAction(KIcon(QLatin1String("mail-attachment")), i18n("&Send File"), this);
    KAction *shareDesktopAction = new KAction(KIcon(QLatin1String("krfb")), i18n("Share My &Desktop"), this);
    KAction *blockContactAction = new KAction(KIcon(QLatin1String("im-ban-kick-user")), i18n("&Block Contact"), this);
    KAction *audioCallAction = new KAction(KIcon(QLatin1String("audio-headset")), i18n("&Audio Call"), this);
    KAction *videoCallAction = new KAction(KIcon(QLatin1String("camera-web")), i18n("&Video Call"), this);
    KAction *clearViewAction = new KAction(KIcon(QLatin1String("edit-clear-history")), i18n("&Clear View"), this);
    connect(openLogAction, SIGNAL(triggered()), this, SLOT(onOpenLogTriggered()));
    connect(fileTransferAction, SIGNAL(triggered()), this, SLOT(onFileTransferTriggered()));
    connect(shareDesktopAction, SIGNAL(triggered()), this, SLOT(onShareDesktopTriggered()));
    connect(blockContactAction, SIGNAL(triggered()), this, SLOT(onBlockContactTriggered()));
    connect(this, SIGNAL(contactBlockStatusChanged(bool)), this, SLOT(toggleBlockButton(bool)));
    connect(audioCallAction, SIGNAL(triggered()), this, SLOT(onAudioCallTriggered()));
    connect(videoCallAction, SIGNAL(triggered()), this, SLOT(onVideoCallTriggered()));
    connect(clearViewAction, SIGNAL(triggered()), this, SLOT(onClearViewTriggered()));
    fileTransferAction->setToolTip(i18nc("Toolbar icon tooltip", "Send a file to this contact"));
    shareDesktopAction->setToolTip(i18nc("Toolbar icon tooltip", "Start an application that "
                                                                 "allows this contact to see your desktop"));
    blockContactAction->setToolTip(i18nc("Toolbar icon tooltip", "Blocking means that this contact "
                                                                 "will not see you online and you will not"
                                                                 "receive any messages from this contact"));
    audioCallAction->setToolTip(i18nc("Toolbar icon tooltip", "Start an audio call with this contact"));
    videoCallAction->setToolTip(i18nc("Toolbar icon tooltip", "Start a video call with this contact"));
    clearViewAction->setToolTip(i18nc("Toolbar icon tooltip", "Clear all messages from current chat tab"));
    actionCollection()->addAction(QLatin1String("block-contact"), blockContactAction);
    actionCollection()->addAction(QLatin1String("share-desktop"), shareDesktopAction);
    actionCollection()->addAction(QLatin1String("send-file"), fileTransferAction);
    actionCollection()->addAction(QLatin1String("open-log"), openLogAction);
    actionCollection()->addAction(QLatin1String("audio-call"), audioCallAction);
    actionCollection()->addAction(QLatin1String("video-call"), videoCallAction);
    actionCollection()->addAction(QLatin1String("clear-chat-view"), clearViewAction);

    setXMLFile(QLatin1String("chatTabWidget.rc"));
}

void ChatTabWidget::onClearViewTriggered()
{
    clear();
}

void ChatTabWidget::onOpenLogTriggered()
{
    Tp::ContactPtr chatContact = chatChannel->targetContact();

    QString id;
    if (!chatContact.isNull()) {
        id = chatContact->id();
    } else {
        id = chatChannel->targetId();
    }
    KToolInvocation::kdeinitExec(QLatin1String("ktp-log-viewer"),
                                     QStringList() << QLatin1String("--") << chatAccount->uniqueIdentifier() << id);
}

void ChatTabWidget::onFileTransferTriggered()
{
    // This feature should be used only in 1on1 chats!
    startFileTransfer(chatAccount, chatChannel->targetContact());
}

void ChatTabWidget::setFileTransferEnabled(bool enable)
{
    QAction *action = actionCollection()->action(QLatin1String("send-file"));

    if (action) {
        action->setEnabled(enable);
    }
}

void ChatTabWidget::startFileTransfer(const Tp::AccountPtr& account, const Tp::ContactPtr& contact)
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
        Tp::PendingChannelRequest* channelRequest = KTp::Actions::startFileTransfer(account, contact, fileName);
        connect(channelRequest, SIGNAL(finished(Tp::PendingOperation*)), SLOT(onGenericOperationFinished(Tp::PendingOperation*)));
    }
}

void ChatTabWidget::onGenericOperationFinished(Tp::PendingOperation* op)
{
    // send notification via plasma like the contactlist does
    if (op->isError()) {
        QString errorMsg(op->errorName() + QLatin1String(": ") + op->errorMessage());
        sendNotificationToUser(SystemErrorMessage, errorMsg);
    }
}

void ChatTabWidget::onAudioCallTriggered()
{
    startAudioCall(chatAccount, chatChannel->targetContact());
}

void ChatTabWidget::setAudioCallEnabled(bool enable)
{
    QAction *action = actionCollection()->action(QLatin1String("audio-call"));

    // don't want to segfault. Should never happen
    if (action) {
        action->setEnabled(enable);
    }
}

void ChatTabWidget::startAudioCall(const Tp::AccountPtr& account, const Tp::ContactPtr& contact)
{
    Tp::PendingChannelRequest *channelRequest = KTp::Actions::startAudioCall(account, contact);
    connect(channelRequest, SIGNAL(finished(Tp::PendingOperation*)), this, SLOT(onGenericOperationFinished(Tp::PendingOperation*)));
}

void ChatTabWidget::setVideoCallEnabled(bool enable)
{
    QAction *action = actionCollection()->action(QLatin1String("video-call"));

    if (action) {
        action->setEnabled(enable);
    }
}

void ChatTabWidget::startVideoCall(const Tp::AccountPtr& account, const Tp::ContactPtr& contact)
{
    Tp::PendingChannelRequest* channelRequest = KTp::Actions::startAudioVideoCall(account, contact);
    connect(channelRequest, SIGNAL(finished(Tp::PendingOperation*)), this, SLOT(onGenericOperationFinished(Tp::PendingOperation*)));
}

void ChatTabWidget::onVideoCallTriggered()
{
    startVideoCall(chatAccount, chatChannel->targetContact());
}

void ChatTabWidget::sendNotificationToUser(ChatTabWidget::NotificationType type, const QString& errorMsg)
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

void ChatTabWidget::onShareDesktopTriggered()
{
    startShareDesktop(chatAccount, chatChannel->targetContact());
}

void ChatTabWidget::startShareDesktop(const Tp::AccountPtr& account, const Tp::ContactPtr& contact)
{
    Tp::PendingChannelRequest* channelRequest = KTp::Actions::startDesktopSharing(account, contact);
    connect(channelRequest, SIGNAL(finished(Tp::PendingOperation*)),
            this, SLOT(onGenericOperationFinished(Tp::PendingOperation*)));
}

void ChatTabWidget::setBlockEnabled(bool enable)
{
    QAction *action = actionCollection()->action(QLatin1String("block-contact"));

    if (action) {
        action->setEnabled(enable);
    }
}

void ChatTabWidget::onBlockContactTriggered()
{
    Tp::ContactPtr contact = chatChannel->targetContact();
    if(!contact.isNull()) {
        contact->block();
     }
}

void ChatTabWidget::onUnblockContactTriggered()
{
    Tp::ContactPtr contact = chatChannel->targetContact();
    contact->unblock();
}

void ChatTabWidget::toggleBlockButton(bool contactIsBlocked)
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
    setWindowTitle(this->title());

    setBlockEnabled(true);
}

#include "chatTabWidget.moc"