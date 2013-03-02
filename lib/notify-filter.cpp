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
    if(message.property("highlight").toBool()) {
        notificationType = QLatin1String("kde_telepathy_contact_highlight");
    } else if(message.type() == Tp::ChannelTextMessageTypeNotice) {
        notificationType = QLatin1String("kde_telepathy_info_event");
    } else {
        if (m_widget->isOnTop()) {
            notificationType = QLatin1String("kde_telepathy_contact_incoming_active_window");
        } else {
            notificationType = QLatin1String("kde_telepathy_contact_incoming");
        }
    }

    KNotification *notification = new KNotification(
                notificationType, m_widget,
                KNotification::RaiseWidgetOnActivation
                | KNotification::CloseWhenWidgetActivated
                | KNotification::Persistent);

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

    //truncate message if necessary
    QString notifyText = message.mainMessagePart().simplified();
    if (notifyText.length() > 170) {
        //search for the closest space in text
        notifyText.truncate(notifyText.indexOf(QLatin1Char(' '), 150));
        notifyText.append(QLatin1String("..."));
    }
    notification->setText(notifyText);


    notification->setActions(QStringList(i18n("View")));
    connect(notification, SIGNAL(activated(uint)), m_widget, SIGNAL(notificationClicked()));

    notification->sendEvent();
}
