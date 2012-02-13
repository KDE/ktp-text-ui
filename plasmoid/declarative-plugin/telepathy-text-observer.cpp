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


#include "telepathy-text-observer.h"
#include "conversation.h"

#include <KDebug>

#include <TelepathyQt/AccountFactory>
#include <TelepathyQt/ConnectionFactory>
#include <TelepathyQt/ClientRegistrar>


TelepathyTextObserver::TelepathyTextObserver(QObject* parent) :
    QObject(parent),
    m_handler(new ConversationsModel())
{
    kDebug();
    Tp::registerTypes();
    Tp::AccountFactoryPtr accountFactory = Tp::AccountFactory::create(QDBusConnection::sessionBus(),
                                                                    Tp::Account::FeatureCore);

    Tp::ConnectionFactoryPtr  connectionFactory = Tp::ConnectionFactory::create(
        QDBusConnection::sessionBus(),
        Tp::Features() << Tp::Connection::FeatureSelfContact
                    << Tp::Connection::FeatureCore
    );

    Tp::ChannelFactoryPtr channelFactory = Tp::ChannelFactory::create(QDBusConnection::sessionBus());
    channelFactory->addCommonFeatures(Tp::Channel::FeatureCore);

    Tp::Features textFeatures = Tp::Features() << Tp::TextChannel::FeatureMessageQueue
                                            << Tp::TextChannel::FeatureMessageSentSignal
                                            << Tp::TextChannel::FeatureChatState
                                            << Tp::TextChannel::FeatureMessageCapabilities;
    channelFactory->addFeaturesForTextChats(textFeatures);
    channelFactory->addFeaturesForTextChatrooms(textFeatures);

    Tp::ContactFactoryPtr contactFactory = Tp::ContactFactory::create(
        Tp::Features() << Tp::Contact::FeatureAlias
                    << Tp::Contact::FeatureAvatarToken
                    << Tp::Contact::FeatureAvatarData
                    << Tp::Contact::FeatureCapabilities
                    << Tp::Contact::FeatureSimplePresence
    );

    //TODO: check these to make sure I'm only requesting features I actually use.
    m_registrar = Tp::ClientRegistrar::create(accountFactory, connectionFactory,
                                            channelFactory, contactFactory);
    m_registrar->registerClient(m_handler, QLatin1String("KDE.TextUi.ConversationWatcher")); //KTp.ChatPlasmoid
}

TelepathyTextObserver::~TelepathyTextObserver()
{
    qDebug() << "deleting text observer";
}

QAbstractListModel * TelepathyTextObserver::conversationModel()
{
    Q_ASSERT(!m_handler.isNull());
    return m_handler.data();
}
