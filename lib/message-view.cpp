/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright 2013  David Edmundson <kde@davidedmundson.co.uk>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "message-view.h"

#include <KDebug>
#include <KIconLoader>

#include "notify-filter.h"
//#include "logmanager.h"

#include <KTp/message-processor.h>

MessageView::MessageView(QWidget *parent)
: AdiumThemeView(parent),
  m_chatViewInitialized(false),
  m_logsLoaded(false),
  m_exchangedMessagesCount(0)
//  m_logManager(new LogManager(this))
{
    // initialize LogManager
    KConfig config(QLatin1String("ktelepathyrc"));
    KConfigGroup tabConfig = config.group("Behavior");
//    m_logManager->setScrollbackLength(tabConfig.readEntry<int>("scrollbackLength", 4));

    //FIXME
//     m_previousConversationAvailable = m_logManager->exists();
//    connect(m_logManager, SIGNAL(fetched(QList<KTp::Message>)), SLOT(onHistoryFetched(QList<KTp::Message>)));

}

MessageView::~MessageView()
{

}

void MessageView::initChatArea()
{
    bool isGroupChat = m_channel->targetHandleType() != Tp::HandleTypeContact;
    connect(this, SIGNAL(loadFinished(bool)), SLOT(onChatViewReady()), Qt::QueuedConnection);

    this->load(isGroupChat ? AdiumThemeView::GroupChat : AdiumThemeView::SingleUserChat);

    AdiumThemeHeaderInfo info;

    info.setGroupChat(isGroupChat);
    //normal chat - self and one other person.
    if (isGroupChat) {
        // A temporary solution to display a roomname instead of a full jid
        // This should be reworked as soon as QtTp is offering the
        // room name property
        QString roomName = m_channel->targetId();
        roomName = roomName.left(roomName.indexOf(QLatin1Char('@')));
        info.setChatName(roomName);
    } else {
        Tp::ContactPtr otherContact = m_channel->targetContact();

        Q_ASSERT(otherContact);

//         m_contactName = otherContact->alias(); //TODO
        info.setDestinationDisplayName(otherContact->alias());
        info.setDestinationName(otherContact->id());
        info.setChatName(otherContact->alias());
        info.setIncomingIconPath(otherContact->avatarData().fileName);
    }

    info.setSourceName(m_channel->connection()->protocolName());

    //set up anything related to 'self'
    info.setOutgoingIconPath(m_channel->groupSelfContact()->avatarData().fileName);

    //set the message time
    if (!m_channel->messageQueue().isEmpty()) {
        info.setTimeOpened(m_channel->messageQueue().first().received());
    } else {
        info.setTimeOpened(QDateTime::currentDateTime());
    }

//     info.setServiceIconImage(KIconLoader::global()->iconPath(m_account->iconName(), KIconLoader::Panel));
//FIXME?
    initialise(info);

    //set the title of this chat.
    //TODO
//     m_title = info.chatName();
}

void MessageView::acknowledgeMessages()
{
    kDebug();
    //if we're not initialised we can't have shown anything, even if we are on top, therefore ignore all requests to do so
    if (m_chatViewInitialized) {
        //acknowledge everything in the message queue.
        m_channel->acknowledge(m_channel->messageQueue());
    }
}

void MessageView::setTextChannel(const Tp::AccountPtr &account, const Tp::TextChannelPtr &textChannel)
{
    bool isFirstRun = m_channel.isNull();
    m_channel = textChannel;
    m_account = account;

    //if the UI is ready process any messages in queue
    if (m_chatViewInitialized) {
        Q_FOREACH (const Tp::ReceivedMessage &message, m_channel->messageQueue()) {
            handleIncomingMessage(message);
        }
    }

    connect(textChannel.data(), SIGNAL(messageReceived(Tp::ReceivedMessage)),
            SLOT(handleIncomingMessage(Tp::ReceivedMessage)));
    connect(textChannel.data(), SIGNAL(messageSent(Tp::Message,Tp::MessageSendingFlags,QString)),
            SLOT(handleMessageSent(Tp::Message,Tp::MessageSendingFlags,QString)));

    if (isFirstRun) {
        //m_logManager->setTextChannel(m_account, m_channel);
        initChatArea();
    }
}


void MessageView::handleIncomingMessage(const Tp::ReceivedMessage& message)
{
    if (m_chatViewInitialized) {
        if (message.isDeliveryReport()) {
            QString text;
            Tp::ReceivedMessage::DeliveryDetails reportDetails = message.deliveryDetails();

            if (reportDetails.hasDebugMessage()) {
                kDebug() << "delivery report debug message: " << reportDetails.debugMessage();
            }

            if (reportDetails.isError()) {
                switch (reportDetails.error()) {
                case Tp::ChannelTextSendErrorOffline:
                    if (reportDetails.hasEchoedMessage()) {
                        if(message.sender() && message.sender()->isBlocked()) {
                            text = i18n("Delivery of the message \"%1\" "
                                        "failed because the remote contact is blocked",
                                        reportDetails.echoedMessage().text());
                         } else {
                            text = i18n("Delivery of the message \"%1\" "
                                        "failed because the remote contact is offline",
                                        reportDetails.echoedMessage().text());
                         }
                    } else {
                        if(message.sender() && message.sender()->isBlocked()) {
                            text = i18n("Delivery of a message failed "
                                        "because the remote contact is blocked");
                        } else {
                            text = i18n("Delivery of a message failed "
                                        "because the remote contact is offline");
                        }
                    }
                    break;
                case Tp::ChannelTextSendErrorInvalidContact:
                    if (reportDetails.hasEchoedMessage()) {
                        text = i18n("Delivery of the message \"%1\" "
                                    "failed because the remote contact is not valid",
                                    reportDetails.echoedMessage().text());
                    } else {
                        text = i18n("Delivery of a message failed "
                                    "because the remote contact is not valid");
                    }
                    break;
                case Tp::ChannelTextSendErrorPermissionDenied:
                    if (reportDetails.hasEchoedMessage()) {
                        text = i18n("Delivery of the message \"%1\" failed because "
                                    "you do not have permission to speak in this room",
                                    reportDetails.echoedMessage().text());
                    } else {
                        text = i18n("Delivery of a message failed because "
                                    "you do not have permission to speak in this room");
                    }
                    break;
                case Tp::ChannelTextSendErrorTooLong:
                    if (reportDetails.hasEchoedMessage()) {
                        text = i18n("Delivery of the message \"%1\" "
                                    "failed because it was too long",
                                    reportDetails.echoedMessage().text());
                    } else {
                        text = i18n("Delivery of a message failed "
                                    "because it was too long");
                    }
                    break;
                default:
                    if (reportDetails.hasEchoedMessage()) {
                        text = i18n("Delivery of the message \"%1\" failed: %2",
                                    reportDetails.echoedMessage().text(),
                                    message.text());
                    } else {
                        text = i18n("Delivery of a message failed: %1", message.text());
                    }
                    break;
                }
            } else {
                //TODO: handle delivery reports properly
                kWarning() << "Ignoring delivery report";
                m_channel->acknowledge(QList<Tp::ReceivedMessage>() << message);
                return;
            }

            addStatusMessage(text, message.received());
        } else {
            m_exchangedMessagesCount++;

            KTp::Message processedMessage(KTp::MessageProcessor::instance()->processIncomingMessage(message, m_account, m_channel));

            addMessage(processedMessage);
            Q_EMIT newMessage(processedMessage);
        }

        //if the window is on top, ack str.channel()aight away. Otherwise they stay in the message queue for acking when activated..
        if (isActiveWindow() && isVisible()) {
            m_channel->acknowledge(QList<Tp::ReceivedMessage>() << message);
        }
    }
}

void MessageView::handleMessageSent(const Tp::Message& message, Tp::MessageSendingFlags flags, const QString& sentMessageToken)
{
    Q_UNUSED(sentMessageToken)
    KTp::Message processedMessage(KTp::MessageProcessor::instance()->processIncomingMessage(message, m_account, m_channel));
    addMessage(processedMessage);
    m_exchangedMessagesCount++;
    Q_EMIT newMessage(processedMessage);
}


void MessageView::onHistoryFetched(const QList< KTp::Message >& messages)
{
    m_chatViewInitialized = true;

    kDebug() << "found" << messages.count() << "messages in history";
    if (!messages.isEmpty()) {
        QDate date = messages.first().time().date();
        Q_FOREACH(const KTp::Message &message, messages) {
            if (message.time().date() != date) {
                date = message.time().date();
                addStatusMessage(date.toString(Qt::LocaleDate));
            }

            addMessage(message);
        }

        if (date != QDate::currentDate()) {
            addStatusMessage(QDate::currentDate().toString(Qt::LocaleDate));
        }
    }

    //process any messages we've 'missed' whilst initialising.
    Q_FOREACH(const Tp::ReceivedMessage &message, m_channel->messageQueue()) {
        handleIncomingMessage(message);
    }
}

void MessageView::onChatViewReady()
{
    disconnect(this, SIGNAL(loadFinished(bool)), this, SLOT(onChatViewReady()));

    if (!m_logsLoaded || m_exchangedMessagesCount > 0) {
        if (m_exchangedMessagesCount == 0) {
            //m_logManager->fetchScrollback();
        } else {
            //m_logManager->fetchHistory(m_exchangedMessagesCount + m_logManager->scrollbackLength());
        }
    }
    m_chatViewInitialized = true;
    m_logsLoaded = true;
}


void MessageView::reloadTheme()
{
    m_logsLoaded = false;
    m_chatViewInitialized = false;

    initChatArea();
}

void MessageView::clear()
{
    m_logsLoaded = true;
    initChatArea();
}

#include "message-view.moc"
