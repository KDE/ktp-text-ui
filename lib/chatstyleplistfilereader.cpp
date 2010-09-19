/***************************************************************************
 *   Copyright (C) 2010 by Dominik Schmidt <domme@rautelinux.org>      *
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


#include "chatstyleplistfilereader.h"

#include <QFile>
#include <QDomDocument>
#include <QVariant>
#include <QDebug>

ChatStylePlistFileReader::ChatStylePlistFileReader(QString fileName)
{
    readFile(fileName);
}

void ChatStylePlistFileReader::readFile(QString &fileName)
{
    QFile file(fileName);

    QDomDocument document = QDomDocument();
    if (!file.open(QIODevice::ReadOnly))
    {
        return;
    }if (!document.setContent(&file)) {
        file.close();
        return;
    }
    file.close();

    QString key, value;
    QDomNodeList keyElements = document.elementsByTagName("key");
    for (int i = 0; i < keyElements.size(); i++) {
        if (keyElements.at(i).nextSibling().toElement().tagName() != "key") {
            key = keyElements.at(i).toElement().text();
            value = keyElements.at(i).nextSibling().toElement().text();
            data.insert(key, value);
        }
    }
}

ChatStylePlistFileReader::~ChatStylePlistFileReader()
{
}

QString ChatStylePlistFileReader::CFBundleGetInfoString()
{
    return data.value("CFBundleGetInfoString").toString();
}

QString ChatStylePlistFileReader::CFBundleName()
{
    return data.value("CFBundleName").toString();
}

QString ChatStylePlistFileReader::CFBundleIdentifier()
{
    return data.value("CFBundleIdentifier").toString();
}

QString ChatStylePlistFileReader::defaultFontFamily()
{
    return data.value("DefaultFontFamily").toString();
}

int ChatStylePlistFileReader::defaultFontSize()
{
    return data.value("DefaultFontSize").toInt();
}

QString ChatStylePlistFileReader::defaultVariant()
{
    return data.value("DefaultVariant").toString();
}

int ChatStylePlistFileReader::messageViewVersion()
{
    return data.value("MessageViewVersion").toInt();
}
