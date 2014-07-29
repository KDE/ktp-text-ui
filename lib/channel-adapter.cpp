/*
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

#include "channel-adapter.h"
#include "chat-widget.h"
#include "otr-channel-proxy.h"
#include "otr-utils.h"
#include "otr-constants.h"

#include <KDebug>

#include <QSharedPointer>
#include <QMap>
#include <QGenericArgument>

typedef QSharedPointer<Tp::Client::ChannelProxyInterfaceOTRInterface> OTRProxyPtr;

    uint getId(const Tp::MessagePartList &message) {
        return message.first()[QLatin1String("pending-message-id")].variant().toUInt(NULL);
    }

class OTRMessage : public Tp::ReceivedMessage
{
    public:
        OTRMessage(const Tp::MessagePartList &message, const Tp::TextChannelPtr channel)
            : Tp::ReceivedMessage(message, channel)
        {
        }
};

struct ChannelAdapter::Private
{
    Private(ChatWidget *chat)
        : chat(chat),
        otrConnected(false)
    {
    }

    Tp::TextChannelPtr textChannel;
    ChatWidget *chat;
    OTRProxyPtr otrProxy;

    bool otrConnected;
    Tp::OTRTrustLevel trustLevel;

    QMap<uint, OTRMessage> messages;
};

ChannelAdapter::ChannelAdapter(const Tp::TextChannelPtr &textChannel, ChatWidget *parent)
    : QObject(parent),
    d(new Private(parent))
{
    setChannel(textChannel);
}

ChannelAdapter::~ChannelAdapter()
{
    delete d;
}

bool ChannelAdapter::isValid() const
{
    return d->textChannel->isValid();
}

void ChannelAdapter::reset()
{
    kDebug();

    d->messages.clear();

    if(isOTRsuppored()) {
        d->otrConnected = false;
        d->trustLevel = Tp::OTRTrustLevelNotPrivate;
        d->otrProxy.clear();
    } else {
        disconnect(d->textChannel.data(), SIGNAL(messageReceived(Tp::ReceivedMessage)),
                this, SIGNAL(messageReceived(Tp::ReceivedMessage)));
        disconnect(d->textChannel.data(), SIGNAL(pendingMessageRemoved(Tp::ReceivedMessage)),
                this, SIGNAL(pendingMessageRemoved(Tp::ReceivedMessage)));
        disconnect(d->textChannel.data(), SIGNAL(messageSent(Tp::Message,Tp::MessageSendingFlags,QString)),
                this, SIGNAL(messageSent(Tp::Message,Tp::MessageSendingFlags,QString)));
    }

    d->textChannel.reset();
}

void ChannelAdapter::setChannel(const Tp::TextChannelPtr &textChannel)
{
    if(d->textChannel) {
        // reset old settings
        reset();
    }

    d->textChannel = textChannel;
    QDBusConnection dbusConnection = textChannel->dbusConnection();
    if(textChannel->targetHandleType() != Tp::HandleTypeContact ||
            !dbusConnection.interface()->isServiceRegistered(KTP_PROXY_BUS_NAME))
    {
        setupTextChannel();
        return;
    }

    QString otrProxyPath = Tp::Utils::getOtrProxyObjectPathFor(textChannel);
    d->otrProxy = OTRProxyPtr(new Tp::Client::ChannelProxyInterfaceOTRInterface(KTP_PROXY_BUS_NAME, otrProxyPath, this));

    if(!d->otrProxy->isValid()) {
        kDebug() << "No OTR proxy available for channel: " << textChannel->objectPath();
        setupTextChannel();
        return;
    }

    kDebug() << "Connecting to the OTR proxy: " << d->otrProxy->path();
    QDBusPendingReply<> connectResult = d->otrProxy->ConnectProxy();
    connectResult.waitForFinished();
    if(connectResult.isValid()) {
        setupOTRChannel();
    } else {
        kWarning() << "Could not connect to the proxy";
        setupTextChannel();
    }
}

Tp::TextChannelPtr ChannelAdapter::textChannel()
{
    return d->textChannel;
}

void ChannelAdapter::setupTextChannel()
{
    kDebug();
    connect(d->textChannel.data(), SIGNAL(messageReceived(Tp::ReceivedMessage)),
            SIGNAL(messageReceived(Tp::ReceivedMessage)));
    connect(d->textChannel.data(), SIGNAL(pendingMessageRemoved(Tp::ReceivedMessage)),
            SIGNAL(pendingMessageRemoved(Tp::ReceivedMessage)));
    connect(d->textChannel.data(), SIGNAL(messageSent(Tp::Message,Tp::MessageSendingFlags,QString)),
            SIGNAL(messageSent(Tp::Message,Tp::MessageSendingFlags,QString)));
}

void ChannelAdapter::setupOTRChannel()
{
    kDebug();
    d->otrConnected = true;
    d->trustLevel = Tp::OTRTrustLevelNotPrivate;

    connect(d->otrProxy.data(), SIGNAL(SessionRefreshed()), SIGNAL(sessionRefreshed()));
    connect(d->otrProxy.data(), SIGNAL(MessageReceived(const Tp::MessagePartList&)),
            SLOT(onMessageReceived(const Tp::MessagePartList&)));
    connect(d->otrProxy.data(), SIGNAL(PendingMessagesRemoved(const Tp::UIntList&)),
            SLOT(onPendingMessagesRemoved(const Tp::UIntList&)));
    connect(d->otrProxy.data(), SIGNAL(MessageSent(const Tp::MessagePartList&, uint, const QString&)),
            SLOT(onMessageSent(const Tp::MessagePartList&, uint, const QString&)));
    connect(d->otrProxy.data(), SIGNAL(TrustLevelChanged(uint)), SLOT(onTrustLevelChanged(uint)));

    // initialize message queue;
    connect(d->otrProxy->requestPropertyPendingMessages(), SIGNAL(finished(Tp::PendingOperation*)),
                SLOT(onPendingMessagesPropertyGet(Tp::PendingOperation*)));
    // initialize trust level property
    connect(d->otrProxy->requestPropertyTrustLevel(), SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onTrustLevelPropertyGet(Tp::PendingOperation*)));
}

Tp::OTRTrustLevel ChannelAdapter::otrTrustLevel() const
{
    return d->trustLevel;
}

void ChannelAdapter::onTrustLevelPropertyGet(Tp::PendingOperation *op)
{
    if(op->isError()) {
        kWarning() << "Could not get property: TrustLevel";
        return;
    }
    Tp::PendingVariant *pv = dynamic_cast<Tp::PendingVariant*>(op);
    d->trustLevel = static_cast<Tp::OTRTrustLevel>(pv->result().toUInt(NULL));
}

bool ChannelAdapter::isOTRsuppored() const
{
    return d->otrConnected;
}

void ChannelAdapter::initializeOTR()
{
    kDebug() << "Initializing OTR session";
    d->otrProxy->Initialize();
}

void ChannelAdapter::stopOTR()
{
    kDebug() << "Stopping OTR session";
    d->otrProxy->Stop();
}

void ChannelAdapter::acknowledge(const QList<Tp::ReceivedMessage> &messages)
{
    if(messages.isEmpty()) {
        return;
    }

    kDebug();
    if(isOTRsuppored()) {
        d->otrProxy->AcknowledgePendingMessages(Tp::Utils::getPendingMessagesIDs(messages));
    } else {
        d->textChannel->acknowledge(messages);
    }
}

void ChannelAdapter::send(const QString& text, Tp::ChannelTextMessageType type, Tp::MessageSendingFlags flags)
{
    if(isOTRsuppored()) {
        Tp::MessagePartList parts;
        parts << Tp::MessagePart() << Tp::MessagePart();
        parts[0].insert(QLatin1String("message-type"),
                QDBusVariant(type));
        parts[1].insert(QLatin1String("content-type"),
                QDBusVariant(QLatin1String("text/plain")));
        parts[1].insert(QLatin1String("content"), QDBusVariant(text));

        d->otrProxy->SendMessage(parts, (uint) flags);
    } else {
        d->textChannel->send(text, type, flags);
    }
}

bool ChannelAdapter::supportsMessageType(Tp::ChannelTextMessageType messageType) const
{
    return d->textChannel->supportsMessageType(messageType);
}

QList<Tp::ChannelTextMessageType> ChannelAdapter::supportedMessageTypes() const
{
    return d->textChannel->supportedMessageTypes();
}

QStringList ChannelAdapter::supportedContentTypes() const
{
    return d->textChannel->supportedContentTypes();
}

Tp::MessagePartSupportFlags ChannelAdapter::messagePartSupport() const
{
    return d->textChannel->messagePartSupport();
}

Tp::DeliveryReportingSupportFlags ChannelAdapter::deliveryReportingSupport() const
{
    return d->textChannel->deliveryReportingSupport();
}

QList<Tp::ReceivedMessage> ChannelAdapter::messageQueue() const
{
    if(isOTRsuppored()) {
        QList<Tp::ReceivedMessage> messages;
        Q_FOREACH(const Tp::ReceivedMessage &m, d->messages) {
            messages << m;
        }
        return messages;
    } else {
        return d->textChannel->messageQueue();
    }
}

void ChannelAdapter::onMessageReceived(const Tp::MessagePartList &message)
{
    kDebug();
    const uint id = getId(message);
    OTRMessage recvMes(message, d->textChannel);
    if(!d->messages.contains(id)) {
        d->messages.insert(id, recvMes);
        Q_EMIT messageReceived(recvMes);
    } else {
        kWarning() << "Message already in the queue. Id: " << id;
    }
}

void ChannelAdapter::onPendingMessagesPropertyGet(Tp::PendingOperation *op)
{
    kDebug();
    Tp::PendingVariant *variant = dynamic_cast<Tp::PendingVariant*>(op);

    if(!variant->isError()) {
        QDBusArgument dbusArgument = variant->result().value<QDBusArgument>();
        Tp::MessagePartListList pendingMessages;
        dbusArgument >> pendingMessages;
        Q_FOREACH(const Tp::MessagePartList &message, pendingMessages) {
            onMessageReceived(message);
        }
    } else {
        kWarning() << "Could not initialize message queue: " << variant->errorName() << " - "
            << variant->errorMessage();
    }
}

void ChannelAdapter::onPendingMessagesRemoved(const Tp::UIntList &messageIDs)
{
    kDebug();
    Q_FOREACH(uint id, messageIDs) {
        const QMap<uint, OTRMessage>::Iterator mIt = d->messages.find(id);
        if(mIt != d->messages.end()) {
            OTRMessage message = *mIt;
            d->messages.erase(mIt);
            Q_EMIT pendingMessageRemoved(message);
        } else {
            kWarning() << "No message to remove with id: " << id;
        }
    }
}

void ChannelAdapter::onMessageSent(const Tp::MessagePartList &content, uint flags, const QString &messageToken)
{
    kDebug();
    OTRMessage message(content, d->textChannel);
    Q_EMIT messageSent(message, Tp::MessageSendingFlags(flags), messageToken);
}

void ChannelAdapter::onTrustLevelChanged(uint trustLevel)
{
    Tp::OTRTrustLevel oldLevel = d->trustLevel;
    d->trustLevel = static_cast<Tp::OTRTrustLevel>(trustLevel);

    Q_EMIT otrTrustLevelChanged(d->trustLevel, oldLevel);
}
