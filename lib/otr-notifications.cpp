/***************************************************************************
 *   Copyright (C) 2014 by Marcin Ziemiński <zieminn@gmail.com>            *
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


#include "otr-notifications.h"

#include <TelepathyQt/AvatarData>

#include <QWidget>

#include <KAboutData>
#include <KComponentData>
#include <KNotification>

namespace OTRNotifications
{

    static KNotification* prepareNotification(QWidget *widget, const Tp::ContactPtr &contact)
    {
        const QString notificationType = QLatin1String("kde_telepathy_info_event");

        KNotification *notification = new KNotification(
                notificationType, widget,
                KNotification::RaiseWidgetOnActivation
                | KNotification::CloseWhenWidgetActivated
                | KNotification::CloseOnTimeout);

        KAboutData telepathySharedAboutData("ktelepathy", 0, KLocalizedString(), 0);
        notification->setComponentData(KComponentData(telepathySharedAboutData));

        QPixmap notificationPixmap;
        if(notificationPixmap.load(contact->avatarData().fileName)) {
            notification->setPixmap(notificationPixmap);
        }

        notification->setActions(QStringList(i18n("View")));

        return notification;
    }

    void otrSessionStarted(QWidget *widget, const Tp::ContactPtr &targetContact, bool verified)
    {
        KNotification *notification = prepareNotification(widget, targetContact);
        if(verified) {
            notification->setText(i18n("Private OTR session started with %1", targetContact->id()));
        } else {
            notification->setText(i18n("Unverified OTR session started with %1", targetContact->id()));
        }

        notification->sendEvent();
    }

    void otrSessionFinished(QWidget *widget, const Tp::ContactPtr &targetContact)
    {
        KNotification *notification = prepareNotification(widget, targetContact);
        notification->setText(i18n("Finished OTR session with %1", targetContact->id()));

        notification->sendEvent();
    }

    void authenticationRequested(QWidget *widget, const Tp::ContactPtr &targetContact)
    {
        KNotification *notification = prepareNotification(widget, targetContact);
        notification->setText(i18n("%1 has requested your authentication", targetContact->id()));

        notification->sendEvent();
    }

    void authenticationConcluded(QWidget *widget, const Tp::ContactPtr &targetContact, bool success)
    {
        KNotification *notification = prepareNotification(widget, targetContact);
        if(success) {
            notification->setText(i18n("Authentication with %1 completed successfully", targetContact->id()));
        } else {
            notification->setText(i18n("Authentication with %1 failed", targetContact->id()));
        }

        notification->sendEvent();
    }

    void authenticationAborted(QWidget *widget, const Tp::ContactPtr &targetContact)
    {
        KNotification *notification = prepareNotification(widget, targetContact);
        notification->setText(i18n("Authentication with %1 was aborted", targetContact->id()));

        notification->sendEvent();
    }

    void authenticationFailed(QWidget *widget, const Tp::ContactPtr &targetContact)
    {
        KNotification *notification = prepareNotification(widget, targetContact);
        notification->setText(i18n("Authentication with %1 failed", targetContact->id()));

        notification->sendEvent();
    }
}
