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

#ifndef ADIUMXTRAPROTOCOLHANDLER_H
#define ADIUMXTRAPROTOCOLHANDLER_H

#include "bundle-installer.h"

#include <QObject>

class AdiumxtraProtocolHandler : public QObject
{
    Q_OBJECT
public:
    AdiumxtraProtocolHandler();
    virtual ~AdiumxtraProtocolHandler();

    void setUrl(const QString &url);

public Q_SLOTS:
    void install();

Q_SIGNALS:
    void finished();

private:
    QString m_url;
};

#endif // ADIUMXTRAPROTOCOLHANDLER_H
