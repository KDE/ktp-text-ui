/***************************************************************************
 *   Copyright (C) 2012 by David Edmundson <kde@davidedmundson.co.uk>      *
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

#include <KUniqueApplication>
#include <KCmdLineArgs>
#include <k4aboutdata.h>

#include "log-viewer.h"
#include "../ktptextui_version.h"
#include "debug.h"

#include <TelepathyQt/AccountManager>

#include <KTp/contact-factory.h>

Q_LOGGING_CATEGORY(KTP_LOGVIEWER, "ktp-logviewer")

int main(int argc, char *argv[])
{
    K4AboutData aboutData("ktp-log-viewer",
                         0,
                         ki18n("KDE IM Log Viewer"),
                         KTP_TEXT_UI_VERSION_STRING);
    aboutData.addAuthor(ki18n("David Edmundson"), ki18n("Developer"), "kde@davidedmundson.co.uk");
    aboutData.addAuthor(ki18n("Daniele E. Domenichelli"), ki18n("Developer"), "daniele.domenichelli@gmail.com");
    aboutData.addAuthor(ki18n("Dan Vrátil"), ki18n("Developer"), "dvratil@redhat.com");
    aboutData.setProductName("telepathy/log-viewer"); //set the correct name for bug reporting
    aboutData.setProgramIconName(QLatin1String("documentation"));
    aboutData.setLicense(K4AboutData::License_GPL_V2);

    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineOptions options;
    options.add("+accountID", ki18n("The UID of the account to preselect"));
    options.add("+contactID", ki18n("The UID of the contact to preselect"));

    KCmdLineArgs::addCmdLineOptions(options);

    KApplication app;

    Tp::registerTypes();

    Tp::AccountFactoryPtr  accountFactory = Tp::AccountFactory::create(
                                                QDBusConnection::sessionBus(),
                                                Tp::Features() << Tp::Account::FeatureCore
                                                    << Tp::Account::FeatureAvatar
                                                    << Tp::Account::FeatureProfile);

    Tp::ConnectionFactoryPtr connectionFactory = Tp::ConnectionFactory::create(
                                                QDBusConnection::sessionBus(),
                                                Tp::Features() << Tp::Connection::FeatureCore
                                                    << Tp::Connection::FeatureSelfContact
                                                    << Tp::Connection::FeatureRoster);

    Tp::ContactFactoryPtr contactFactory = KTp::ContactFactory::create(
                                                Tp::Features()  << Tp::Contact::FeatureAlias
                                                    << Tp::Contact::FeatureAvatarData
                                                    << Tp::Contact::FeatureSimplePresence
                                                    << Tp::Contact::FeatureCapabilities);

    Tp::ChannelFactoryPtr channelFactory = Tp::ChannelFactory::create(QDBusConnection::sessionBus());

    LogViewer *logViewer = new LogViewer(accountFactory, connectionFactory, channelFactory, contactFactory);
    logViewer->show();

    return app.exec();
}
