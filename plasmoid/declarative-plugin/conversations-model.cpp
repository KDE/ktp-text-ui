/*
    Copyright (C) 2011  Lasath Fernando <kde@lasath.org>

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


#include "conversations-model.h"
#include "conversation.h"
#include "conversation-target.h"
#include "messages-model.h"
#include "channel-delegator.h"

#include <KDebug>

#include <TelepathyQt/ChannelClassSpec>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/ClientRegistrar>

static inline Tp::ChannelClassSpecList channelClassList()
{
    return Tp::ChannelClassSpecList() << Tp::ChannelClassSpec::textChat();
}

class ConversationsModel::ConversationsModelPrivate
{
public:
    QList<Conversation*> conversations;
};

ConversationsModel::ConversationsModel() :
        QAbstractListModel(),
        Tp::AbstractClientHandler(channelClassList()),
        d(new ConversationsModelPrivate)
{
    QHash<int, QByteArray> roles;
    roles[ConversationRole] = "conversation";
    setRoleNames(roles);
}

QVariant ConversationsModel::data(const QModelIndex& index, int role) const
{
    QVariant result;
    if (index.isValid()) {
        if (role == ConversationRole) {
            result = QVariant::fromValue<Conversation*>(d->conversations[index.row()]);
            kDebug() << "returning value " << result;
        }
    }
    return result;
}

int ConversationsModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return d->conversations.count();
}

void ConversationsModel::handleChannels(const Tp::MethodInvocationContextPtr<> &context,
                                        const Tp::AccountPtr &account,
                                        const Tp::ConnectionPtr &connection,
                                        const QList<Tp::ChannelPtr> &channels,
                                        const QList<Tp::ChannelRequestPtr> &channelRequests,
                                        const QDateTime &userActionTime,
                                        const HandlerInfo &handlerInfo)
{
    Q_UNUSED(connection);
    Q_UNUSED(handlerInfo);

    bool handled = false;
    bool shouldDelegate = false;

    //check that the channel is of type text
    Tp::TextChannelPtr textChannel;
    Q_FOREACH(const Tp::ChannelPtr & channel, channels) {
        textChannel = Tp::TextChannelPtr::dynamicCast(channel);
        if (textChannel) {
            break;
        }
    }

    Q_ASSERT(textChannel);


    //find the relevant channelRequest
    Q_FOREACH(const Tp::ChannelRequestPtr channelRequest, channelRequests) {
        kDebug() << channelRequest->hints().allHints();
        shouldDelegate = channelRequest->hints().hint(QLatin1String("org.freedesktop.Telepathy.ChannelRequest"), QLatin1String("DelegateToPreferredHandler")).toBool();
    }


    //loop through all conversations checking for matches

    //if we are handling and we're not told to delegate it, update the text channel
    //if we are handling but should delegate, call delegate channel
    Q_FOREACH(Conversation *convo, d->conversations) {
        if (convo->target()->id() == textChannel->targetId() &&
                convo->messages()->textChannel()->targetHandleType() == textChannel->targetHandleType())
        {
            if (!shouldDelegate) {
                convo->messages()->setTextChannel(textChannel);
            } else {
                if (convo->messages()->textChannel() == textChannel) {
                    ChannelDelegator::delegateChannel(account, textChannel, userActionTime);
                }
            }
            handled = true;
            break;
        }
    }

    //if we are not handling channel already and should not delegate, add the conversation
    //if we not handling the channel but should delegate it, do nothing.

    if (!handled && !shouldDelegate) {
        beginInsertRows(QModelIndex(), rowCount(), rowCount());
        Conversation* newConvo = new Conversation(textChannel, account, this);
        d->conversations.append(newConvo);
        connect(newConvo, SIGNAL(validityChanged(bool)), SLOT(handleValidityChange(bool)));
        endInsertRows();
        context->setFinished();
    }

}

bool ConversationsModel::bypassApproval() const
{
    return true;
}

void ConversationsModel::handleValidityChange(bool valid)
{
    if(!valid) {
        Conversation* sender = qobject_cast<Conversation*>(QObject::sender());
        int index = d->conversations.indexOf(sender);
        if(index != -1) {
            beginRemoveRows(QModelIndex(), index, index);

            d->conversations.removeAt(index);
            sender->deleteLater();
            endRemoveRows();
        } else {
            kError() << "attempting to delete non-existent conversation";
        }
    }
}

ConversationsModel::~ConversationsModel()
{
    delete d;
}
