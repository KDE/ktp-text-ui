 /***************************************************************************
 *   Copyright (C) 2013 by Daniel Cohen <analoguecolour@gmail.com>        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#ifndef _CHATTABWIDGET_H_
#define _CHATTABWIDGET_H_

#include <kparts/part.h>
#include <kparts/factory.h>
#include <KConfigGroup>
#include <chat-search-bar.h>

#include "chat-widget.h"
#include "kxmlguiclient.h"
#include "kxmlguiwindow.h"



class ChatTabWidget : public ChatWidget, public KXMLGUIClient
{
  Q_OBJECT

public:
    explicit ChatTabWidget(const Tp::TextChannelPtr& channel, const Tp::AccountPtr& account, KXmlGuiWindow* parent);
    void setupActions();

    enum NotificationType {
        SystemErrorMessage,
        SystemInfoMessage
    };

private:
    Tp::TextChannelPtr chatChannel;
    Tp::AccountPtr chatAccount;
    void setFileTransferEnabled(bool enable);
    void setAudioCallEnabled(bool enable);
    void setVideoCallEnabled(bool enable);
    void setBlockEnabled(bool enable);
    void setCollaborateDocumentEnabled(bool enable);
    void offerDocumentToContact(const Tp::AccountPtr& account, const Tp::ContactPtr& targetContact);
    void offerDocumentToChatroom(const Tp::AccountPtr& account, const QString& roomName);
    void sendNotificationToUser(ChatTabWidget::NotificationType type, const QString& errorMsg);
    void startShareDesktop(const Tp::AccountPtr& account, const Tp::ContactPtr& contact);

private Q_SLOTS:
//     void onEnableSearchActions(bool enable);
//     void onFindPreviousText();
//     void onFindNextText();
//     void onSearchActionToggled();
    void onOpenLogTriggered();
    void onFileTransferTriggered();
    void startFileTransfer(const Tp::AccountPtr& account, const Tp::ContactPtr& contact);
    void onGenericOperationFinished(Tp::PendingOperation* op);
    void onAudioCallTriggered();
    void startAudioCall(const Tp::AccountPtr& account, const Tp::ContactPtr& contact);
    void onVideoCallTriggered();
    void startVideoCall(const Tp::AccountPtr &account, const Tp::ContactPtr &contact);
    void onClearViewTriggered();
    void onCollaborateDocumentTriggered();
    void onShareDesktopTriggered();
    void onBlockContactTriggered();
    void onUnblockContactTriggered();
    void toggleBlockButton(bool contactIsBlocked);
};

#endif // CHATTABWIDGET_H