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

class ChatStylePlistFileReader::Private
{
public:
    Private() {}

    ~Private() {}

    QMap<QString, QVariant> data;
    Status m_status;
};

ChatStylePlistFileReader::ChatStylePlistFileReader(const QString &fileName)
{
    QFile file(fileName);

    d = new Private();
    d->m_status = readAndParseFile(file);
}

ChatStylePlistFileReader::ChatStylePlistFileReader(const QByteArray& fileContent)
{
    QDomDocument document;
    document.setContent(fileContent);

    d = new Private();
    d->m_status = parse(document);
}

ChatStylePlistFileReader::Status ChatStylePlistFileReader::readAndParseFile(QFile& file)
{
    QDomDocument document;

    if (!file.open(QIODevice::ReadOnly)) {
        return CannotOpenFileError;
    } if (!document.setContent(&file)) {
        file.close();
        return UnknownError;
    }
    file.close();

     return parse(document);
}

ChatStylePlistFileReader::Status ChatStylePlistFileReader::parse(const QDomDocument& document)
{
    QString key, value;
    QDomNodeList keyElements = document.elementsByTagName("key");

    for (int i = 0; i < keyElements.size(); i++) {
        if (keyElements.at(i).nextSibling().toElement().tagName() != "key") {
            key = keyElements.at(i).toElement().text();
            QDomElement nextElement= keyElements.at(i).nextSibling().toElement();
            if(nextElement.tagName() == "true" || nextElement.tagName() == "false") {
                value = nextElement.tagName();
            } else {
                value = nextElement.text();
            }
            d->data.insert(key, value);
        }
    }

    return Ok;
}

ChatStylePlistFileReader::~ChatStylePlistFileReader()
{
    delete d;
}

QString ChatStylePlistFileReader::CFBundleGetInfoString()
{
    return d->data.value("CFBundleGetInfoString").toString();
}

QString ChatStylePlistFileReader::CFBundleName()
{
    return d->data.value("CFBundleName").toString();
}

QString ChatStylePlistFileReader::CFBundleIdentifier()
{
    return d->data.value("CFBundleIdentifier").toString();
}

QString ChatStylePlistFileReader::defaultFontFamily()
{
    return d->data.value("DefaultFontFamily").toString();
}

int ChatStylePlistFileReader::defaultFontSize()
{
    return d->data.value("DefaultFontSize").toInt();
}

QString ChatStylePlistFileReader::defaultVariant()
{
    return d->data.value("DefaultVariant").toString();
}

int ChatStylePlistFileReader::messageViewVersion()
{
    return d->data.value("MessageViewVersion").toInt();
}

ChatStylePlistFileReader::Status ChatStylePlistFileReader::status()
{
    return d->m_status;
}

bool ChatStylePlistFileReader::showUserIcons()
{
    return d->data.value("ShowUserIcons").toBool();
}

bool ChatStylePlistFileReader::showUserIcons(const QString& variantName)
{
    return d->data.value(QString("ShowUserIcons:%1").arg(variantName)).toBool();
}

bool ChatStylePlistFileReader::disableCombineConsecutive()
{
    return d->data.value("DisableCombineConsecutive").toBool();
}

bool ChatStylePlistFileReader::defaultBackgroundIsTransparent()
{
    return d->data.value("DefaultBackgroundIsTransparent").toBool();
}

bool ChatStylePlistFileReader::disableCustomBackground()
{
    return d->data.value("DisableCustomBackground").toBool();
}

QString ChatStylePlistFileReader::defaultBackgroundColor()
{
    return d->data.value("DefaultBackgroundColor").toString();
}

QString ChatStylePlistFileReader::defaultBackgroundColor(const QString& variantName)
{
    return d->data.value(QString("DefaultBackgroundColor:%1").arg(variantName)).toString();
}

bool ChatStylePlistFileReader::allowTextColors()
{
    return d->data.value("AllowTextColors").toBool();
}

bool ChatStylePlistFileReader::allowTextColors(const QString& variantName)
{
    return d->data.value(QString("AllowTextColors").arg(variantName)).toBool();
}

QString ChatStylePlistFileReader::imageMask()
{
    return d->data.value("ImageMask").toString();
}
