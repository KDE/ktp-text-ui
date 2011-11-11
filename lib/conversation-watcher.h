/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) 2011  <copyright holder> <email>

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


#ifndef CONVERSATION_WATCHER_H
#define CONVERSATION_WATCHER_H

#include <TelepathyQt4/AbstractClient>

class Conversation;

class ConversationWatcher : public Tp::AbstractClientObserver , public QObject
{
Q_OBJECT

public:
    virtual void observeChannels(const Tp::MethodInvocationContextPtr<>& context,
								 const Tp::AccountPtr& account,
								 const Tp::ConnectionPtr& connection,
								 const QList< Tp::ChannelPtr >& channels,
								 const Tp::ChannelDispatchOperationPtr& dispatchOperation,
								 const QList< Tp::ChannelRequestPtr >& requestsSatisfied,
								 const Tp::AbstractClientObserver::ObserverInfo& observerInfo
								);
    ConversationWatcher();
    virtual ~ConversationWatcher();

Q_SIGNALS:
	void newConversation(Conversation&);
};

#endif // CONVERSATION_WATCHER_H
