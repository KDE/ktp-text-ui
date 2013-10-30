 /*
    Copyright (C) 2013  Daniel Cohen    <analoguecolour@gmail.com>

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
#include <KDebug>
#include <KFileDialog>
#include <ktoolinvocation.h>
#include <KTp/actions.h>
#include <KNotification>
#include <TelepathyQt/PendingChannelRequest>

ChatTabWidget::ChatTabWidget(const Tp::TextChannelPtr& channel, const Tp::AccountPtr& account, KXmlGuiWindow* parent): ChatWidget(channel, account, parent), KXMLGUIClient()
{
    chatAccount = account;
    chatChannel = channel;
    SetupActions();
}

ChatTabWidget::~ChatTabWidget()
{
}

void ChatTabWidget::SetupActions()
{
    KAction *openLogAction = new KAction(KIcon(QLatin1String("view-pim-journal")), i18nc("Action to open the log viewer with a specified contact","&Previous Conversations"), this);
    connect(openLogAction, SIGNAL(triggered()), this, SLOT(onOpenLogTriggered()));
    actionCollection()->addAction(QLatin1String("open-log"), openLogAction);
    KAction *fileTransferAction = new KAction(KIcon(QLatin1String("mail-attachment")), i18n("&Send File"), this);
    fileTransferAction->setToolTip(i18nc("Toolbar icon tooltip", "Send a file to this contact"));
    connect(fileTransferAction, SIGNAL(triggered()), this, SLOT(onFileTransferTriggered()));
    actionCollection()->addAction(QLatin1String("send-file"), fileTransferAction);
    setXMLFile(QLatin1String("chatTabWidget.rc"));
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

#include "chatTabWidget.moc"
