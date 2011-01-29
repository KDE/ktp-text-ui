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

#ifndef CHATSTYLEPLISTFILEREADER_H
#define CHATSTYLEPLISTFILEREADER_H

#include <QtCore/QMap>

class QString;
class QFile;
class QVariant;
class QDomDocument;


class ChatStylePlistFileReader
{
public:
    enum Status { Ok = 0, CannotOpenFileError, ParseError, UnknownError };
    ChatStylePlistFileReader(const QString &fileName);
    ChatStylePlistFileReader(const QByteArray &file);
    virtual ~ChatStylePlistFileReader();

    QString CFBundleGetInfoString() const;
    QString CFBundleIdentifier() const;
    QString CFBundleName() const;
    QString defaultFontFamily() const;
    int defaultFontSize() const;
    QString defaultVariant() const;
    int messageViewVersion() const;
    bool showUserIcons() const;
    bool showUserIcons(const QString &variantName) const;
    bool disableCombineConsecutive() const;
    bool defaultBackgroundIsTransparent() const;
    bool disableCustomBackground() const;
    QString defaultBackgroundColor() const;
    QString defaultBackgroundColor(const QString &variantName) const;
    bool allowTextColors() const;
    bool allowTextColors(const QString &variantName) const;
    QString imageMask() const;

    Status status() const;


private:
    class Private;
    Private * const d;
    ChatStylePlistFileReader::Status readAndParseFile(QFile &file);
    ChatStylePlistFileReader::Status parse(const QDomDocument &document);
};

#endif // CHATSTYLEPLISTFILEREADER_H
