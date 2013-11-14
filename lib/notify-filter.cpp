/*
    Copyright (C) 2010 by David Edmundson <kde@davidedmundson.co.uk>
    Copyright (C) 2013  Lasath Fernando <kde@lasath.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "notify-filter.h"

#include <KAboutData>
#include <KComponentData>
#include <KNotification>

NotifyFilter::NotifyFilter(ChatWidget *widget) :
    KTp::AbstractMessageFilter(widget),
    m_widget(widget)
{
}

// TODO: when d_ed makes this available in a libarary, like he
// said in chat-widget.cpp, replace this
static KComponentData telepathyComponentData() {
    KAboutData telepathySharedAboutData("ktelepathy",0,KLocalizedString(),0);
    return KComponentData(telepathySharedAboutData);
}

void NotifyFilter::filterMessage(KTp::Message &message, const KTp::MessageContext &context) {

    // don't notify of past messages
    if (message.isHistory()) {
        return;
    }
    // don't notify of outgoing messages
    if (message.direction() != KTp::Message::RemoteToLocal) {
        return;
    }
    // don't notify of messages sent by self from another location
    if (message.senderId() == context.channel()->groupSelfContact()->id()) {
        return;
    }

    // choose correct notification type
    QString notificationType;
    if(message.type() == Tp::ChannelTextMessageTypeNotice) {
        notificationType = QLatin1String("kde_telepathy_info_event");
    } else {
        if (m_widget->isGroupChat()) {
            if(message.property("highlight").toBool()) {
                notificationType = QLatin1String("kde_telepathy_group_chat_highlight");
            } else {
                notificationType = QLatin1String("kde_telepathy_group_chat_incoming");
            }
        } else {
            notificationType = QLatin1String("kde_telepathy_contact_incoming");
        }

        if (m_widget->isOnTop()) {
            notificationType += QLatin1String("_active_window");
        }
    }

    KNotification *notification = new KNotification(
                notificationType, m_widget,
                KNotification::RaiseWidgetOnActivation
                | KNotification::CloseWhenWidgetActivated
                | KNotification::CloseOnTimeout);

    notification->setComponentData(telepathyComponentData());
    notification->setTitle(i18n("%1 has sent you a message",
                                message.senderAlias()));

    QString senderAvatar = message.property("senderAvatar").toString();
    if (!senderAvatar.isNull()) {
        QPixmap notificationPixmap;
        if (notificationPixmap.load(senderAvatar)) {
            notification->setPixmap(notificationPixmap);
        }
    }

    notification->setText(message.mainMessagePart().simplified());

    notification->setActions(QStringList(i18n("View")));
    connect(notification, SIGNAL(activated(uint)), m_widget, SIGNAL(notificationClicked()));

    notification->sendEvent();
}
