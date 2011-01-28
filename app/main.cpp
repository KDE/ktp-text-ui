/***************************************************************************
 *   Copyright (C) 2010 by David Edmundson <kde@davidedmundson.co.uk>      *
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

#include "mainwindow.h"

#include <TelepathyQt4/Channel>
#include <TelepathyQt4/ClientRegistrar>
#include <TelepathyQt4/AccountFactory>
#include <TelepathyQt4/ConnectionFactory>


#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>

int main(int argc, char *argv[])
{

    KAboutData aboutData("telepathy-chat-handler",
                         0,
                         ki18n("Telepathy Chat Handler"),
                         "0.1");

    KCmdLineArgs::init(argc, argv, &aboutData);
    KApplication app;

    registerTypes();


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

    Tp::ClientRegistrarPtr registrar = Tp::ClientRegistrar::create(accountFactory, connectionFactory,
                                                                   channelFactory, contactFactory);
    MainWindow* mainWindow = new MainWindow();

    AbstractClientPtr handler = AbstractClientPtr::dynamicCast(SharedPtr<MainWindow>(mainWindow));
    registrar->registerClient(handler, "KDEChatHandler");

    mainWindow->show();

    return app.exec();
    delete mainWindow;
}
