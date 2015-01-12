/*
    Copyright (C) 2010  David Edmundson    <kde@davidedmundson.co.uk>
    Copyright (C) 2011  Dominik Schmidt    <dev@dominik-schmidt.de>
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

#include "chat-window.h"
#include "telepathy-chat-ui.h"

#include "defines.h"
#include "../ktptextui_version.h"

#include <TelepathyQt/ClientRegistrar>
#include <TelepathyQt/AccountFactory>
#include <TelepathyQt/ConnectionFactory>
#include <TelepathyQt/ChannelFactory>
#include <TelepathyQt/ContactFactory>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/OutgoingFileTransferChannel>
#include <TelepathyQt/AccountManager>

#include <KTp/core.h>
#include <KTp/OTR/types.h>

#include <KAboutData>
#include <KLocalizedString>

#include <Kdelibs4ConfigMigrator>

int main(int argc, char *argv[])
{

    Kdelibs4ConfigMigrator migrator(QLatin1String("ktp-text-ui"));
    migrator.setConfigFiles(QStringList() << QLatin1String("ktp-text-uirc"));
    migrator.setUiFiles(QStringList() << QLatin1String("chatwindow.rc"));
    migrator.migrate();

    KAboutData aboutData("ktp-text-ui", i18n("Chat Application"),
                         QStringLiteral(KTP_TEXT_UI_VERSION_STRING));
    aboutData.addAuthor(i18n("David Edmundson"), i18n("Developer"), "david@davidedmundson.co.uk");
    aboutData.addAuthor(i18n("Dominik Schmidt"), i18n("Developer"), "kde@dominik-schmidt.de");
    aboutData.addAuthor(i18n("Francesco Nwokeka"), i18n("Developer"), "francesco.nwokeka@gmail.com");
    aboutData.addAuthor(i18n("Marcin Ziemiński"), i18n("Developer"), "zieminn@gmail.com");
    aboutData.setProductName("telepathy/text-ui"); //set the correct name for bug reporting
    aboutData.setLicense(KAboutLicense::GPL_V2);

    Tp::registerTypes();
    KTp::registerOtrTypes();

    Tp::ChannelFactoryPtr channelFactory = Tp::ChannelFactory::create(QDBusConnection::sessionBus());
    channelFactory->addCommonFeatures(Tp::Channel::FeatureCore);

    Tp::Features textFeatures = Tp::Features() << Tp::TextChannel::FeatureMessageQueue
                                               << Tp::TextChannel::FeatureMessageSentSignal
                                               << Tp::TextChannel::FeatureChatState
                                               << Tp::TextChannel::FeatureMessageCapabilities;
    channelFactory->addFeaturesForTextChats(textFeatures);
    channelFactory->addFeaturesForTextChatrooms(textFeatures);
    channelFactory->addFeaturesForOutgoingFileTransfers(Tp::OutgoingFileTransferChannel::FeatureCore);

    Tp::ClientRegistrarPtr registrar = Tp::ClientRegistrar::create(KTp::accountFactory(), KTp::connectionFactory(), channelFactory, KTp::contactFactory());

    Tp::SharedPtr<TelepathyChatUi> app(new TelepathyChatUi(argc, argv));
    Tp::AbstractClientPtr handler = Tp::AbstractClientPtr(app);
    registrar->registerClient(handler, QLatin1String(KTP_TEXTUI_CLIENT_NAME));

    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("telepathy-kde")));
    KAboutData::setApplicationData(aboutData);

    return app->exec();
}
