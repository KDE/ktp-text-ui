/*
    Copyright (C) 2010  David Edmundson <kde@davidedmundson.co.uk>
    Copyright (C) 2011  Dominik Schmidt <dev@dominik-schmidt.de>

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

#include "chat-window.h"
#include "telepathy-chat-ui.h"

#include "defines.h"

#include <TelepathyQt/ClientRegistrar>
#include <TelepathyQt/AccountFactory>
#include <TelepathyQt/ConnectionFactory>
#include <TelepathyQt/ChannelFactory>
#include <TelepathyQt/ContactFactory>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/OutgoingFileTransferChannel>

#include <KAboutData>
#include <KCmdLineArgs>

int main(int argc, char *argv[])
{
    KAboutData aboutData("ktp-text-ui", 0,
                         ki18n("Telepathy Text Ui"),
                         "0.5");
    aboutData.addAuthor(ki18n("David Edmundson"), ki18n("Developer"), "david@davidedmundson.co.uk");
    aboutData.addAuthor(ki18n("Dominik Schmidt"), ki18n("Developer"), "kde@dominik-schmidt.de");
    aboutData.addAuthor(ki18n("Francesco Nwokeka"), ki18n("Developer"), "francesco.nwokeka@gmail.com");
    aboutData.setProductName("telepathy/text-ui"); //set the correct name for bug reporting
    aboutData.setLicense(KAboutData::License_GPL_V2);

    KCmdLineArgs::init(argc, argv, &aboutData);

    Tp::registerTypes();

    Tp::AccountFactoryPtr accountFactory = Tp::AccountFactory::create(QDBusConnection::sessionBus(),
                                                                      Tp::Features() << Tp::Account::FeatureCore
                                                                                     << Tp::Account::FeatureProfile);

    Tp::ConnectionFactoryPtr  connectionFactory = Tp::ConnectionFactory::create(
        QDBusConnection::sessionBus(),
        Tp::Features() << Tp::Connection::FeatureSelfContact
                       << Tp::Connection::FeatureCore
                       << Tp::Connection::FeatureRoster
    );

    Tp::ChannelFactoryPtr channelFactory = Tp::ChannelFactory::create(QDBusConnection::sessionBus());
    channelFactory->addCommonFeatures(Tp::Channel::FeatureCore);

    Tp::Features textFeatures = Tp::Features() << Tp::TextChannel::FeatureMessageQueue
                                               << Tp::TextChannel::FeatureMessageSentSignal
                                               << Tp::TextChannel::FeatureChatState
                                               << Tp::TextChannel::FeatureMessageCapabilities;
    channelFactory->addFeaturesForTextChats(textFeatures);
    channelFactory->addFeaturesForTextChatrooms(textFeatures);
    channelFactory->addFeaturesForOutgoingFileTransfers(Tp::OutgoingFileTransferChannel::FeatureCore);

    Tp::ContactFactoryPtr contactFactory = Tp::ContactFactory::create(
        Tp::Features() << Tp::Contact::FeatureAlias
                       << Tp::Contact::FeatureAvatarToken
                       << Tp::Contact::FeatureAvatarData
                       << Tp::Contact::FeatureCapabilities
                       << Tp::Contact::FeatureSimplePresence
    );

    Tp::ClientRegistrarPtr registrar = Tp::ClientRegistrar::create(accountFactory, connectionFactory,
                                                                   channelFactory, contactFactory);

    Tp::SharedPtr<TelepathyChatUi> app = Tp::SharedPtr<TelepathyChatUi>(new TelepathyChatUi);
    Tp::AbstractClientPtr handler = Tp::AbstractClientPtr(app);
    registrar->registerClient(handler, QLatin1String(KTP_TEXTUI_CLIENT_NAME));

    return app->exec();
}
