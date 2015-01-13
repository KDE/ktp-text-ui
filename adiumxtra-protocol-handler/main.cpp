/*
    KDE Telepathy AdiumxtraProtocolHandler - Install Adiumxtra packages through adiumxtra://-pseudo protocol
    Copyright (C) 2010 Dominik Schmidt <domme@rautelinux.org>

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

#include "adiumxtra-protocol-handler.h"
#include "../ktptextui_version.h"

#include <KAboutData>
#include <KLocalizedString>

#include <QApplication>
#include <QCommandLineParser>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    KAboutData aboutData("ktp-adiumxtra-protocol-handler",
                         i18n("AdiumXtra Protocol Handler"),
                         QStringLiteral(KTP_TEXT_UI_VERSION_STRING));
    aboutData.addAuthor(i18n("Dominik Schmidt"), i18n("Developer"), "kde@dominik-schmidt.de");
    aboutData.addAuthor(i18n("Daniele E. Domenichelli"), i18n("Developer"), "daniele.domenichelli@gmail.com");
    aboutData.setProductName("telepathy/text-ui"); //set the correct name for bug reporting
    aboutData.setLicense(KAboutLicense::GPL_V2);

    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    QCommandLineOption adiumBundleOption(QStringLiteral("adium-bundle"), i18n("Adium package to install"));
    parser.addOption(adiumBundleOption);
    parser.process(app);


    if (parser.positionalArguments().count() == 0) {
        return -1;
    }

    AdiumxtraProtocolHandler handler;
    handler.setUrl(parser.value(adiumBundleOption));

    app.connect(&handler, SIGNAL(finished()), SLOT(quit()));
    QTimer::singleShot(0, &handler, SLOT(install()));

    return app.exec();
}
