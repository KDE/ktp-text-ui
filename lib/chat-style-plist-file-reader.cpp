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

#include "chat-style-plist-file-reader.h"

#include <QtCore/QFile>
#include <QtCore/QVariant>
#include <QtXml/QDomDocument>

class ChatStylePlistFileReader::Private
{
public:
    Private() {}
    ~Private() {}

    QMap<QString, QVariant> data;
    Status m_status;
};

ChatStylePlistFileReader::ChatStylePlistFileReader(const QString &fileName)
    : d(new Private)
{
    QFile file(fileName);

    d->m_status = readAndParseFile(file);
}

ChatStylePlistFileReader::ChatStylePlistFileReader(const QByteArray& fileContent)
    : d(new Private)
{
    QDomDocument document;
    document.setContent(fileContent);

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
    QDomNodeList keyElements = document.elementsByTagName(QLatin1String("key"));

    for (int i = 0; i < keyElements.size(); i++) {
        if (keyElements.at(i).nextSibling().toElement().tagName() != QLatin1String("key")) {
            key = keyElements.at(i).toElement().text();
            QDomElement nextElement= keyElements.at(i).nextSibling().toElement();
            if(nextElement.tagName() == QLatin1String("true") || nextElement.tagName() == QLatin1String("false")) {
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

QString ChatStylePlistFileReader::CFBundleGetInfoString() const
{
    return d->data.value(QLatin1String("CFBundleGetInfoString")).toString();
}

QString ChatStylePlistFileReader::CFBundleName() const
{
    return d->data.value(QLatin1String("CFBundleName")).toString();
}

QString ChatStylePlistFileReader::CFBundleIdentifier() const
{
    return d->data.value(QLatin1String("CFBundleIdentifier")).toString();
}

QString ChatStylePlistFileReader::defaultFontFamily() const
{
    return d->data.value(QLatin1String("DefaultFontFamily")).toString();
}

int ChatStylePlistFileReader::defaultFontSize() const
{
    return d->data.value(QLatin1String("DefaultFontSize")).toInt();
}

QString ChatStylePlistFileReader::defaultVariant() const
{
    return d->data.value(QLatin1String("DefaultVariant")).toString();
}

QString ChatStylePlistFileReader::displayNameForNoVariant() const
{
    return d->data.value(QLatin1String("DisplayNameForNoVariant")).toString();
}

int ChatStylePlistFileReader::messageViewVersion() const
{
    return d->data.value(QLatin1String("MessageViewVersion")).toInt();
}

ChatStylePlistFileReader::Status ChatStylePlistFileReader::status() const
{
    return d->m_status;
}

bool ChatStylePlistFileReader::showUserIcons() const
{
    return d->data.value(QLatin1String("ShowUserIcons")).toBool();
}

bool ChatStylePlistFileReader::showUserIcons(const QString& variantName) const
{
    return d->data.value(QString(QLatin1String("ShowUserIcons:%1")).arg(variantName)).toBool();
}

bool ChatStylePlistFileReader::disableCombineConsecutive() const
{
    return d->data.value(QLatin1String("DisableCombineConsecutive")).toBool();
}

bool ChatStylePlistFileReader::defaultBackgroundIsTransparent() const
{
    return d->data.value(QLatin1String("DefaultBackgroundIsTransparent")).toBool();
}

bool ChatStylePlistFileReader::disableCustomBackground() const
{
    return d->data.value(QLatin1String("DisableCustomBackground")).toBool();
}

QString ChatStylePlistFileReader::defaultBackgroundColor() const
{
    return d->data.value(QLatin1String("DefaultBackgroundColor")).toString();
}

QString ChatStylePlistFileReader::defaultBackgroundColor(const QString& variantName) const
{
    return d->data.value(QString(QLatin1String("DefaultBackgroundColor:%1")).arg(variantName)).toString();
}

bool ChatStylePlistFileReader::allowTextColors() const
{
    return d->data.value(QLatin1String("AllowTextColors")).toBool();
}

bool ChatStylePlistFileReader::allowTextColors(const QString& variantName) const
{
    return d->data.value(QString(QLatin1String("AllowTextColors")).arg(variantName)).toBool();
}

QString ChatStylePlistFileReader::imageMask() const
{
    return d->data.value(QLatin1String("ImageMask")).toString();
}
