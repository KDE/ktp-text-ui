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
#include <KAboutData>
#include "log-viewer.h"

int main(int argc, char *argv[])
{
    KAboutData aboutData("ktp-log-viewer",
                         0,
                         ki18n("KDE IM Log Viewer"),
                         "0.3.60");
    aboutData.addAuthor(ki18n("David Edmundson"), ki18n("Developer"), "kde@kde@davidedmundson.co.uk");
    aboutData.addAuthor(ki18n("Daniele E. Domenichelli"), ki18n("Developer"), "daniele.domenichelli@gmail.com");
    aboutData.setProductName("telepathy/logger"); //set the correct name for bug reporting
    aboutData.setLicense(KAboutData::License_GPL_V2);
    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineArgs::parsedArgs();
    KUniqueApplication a;
    LogViewer w;
    w.show();

    return a.exec();
}
