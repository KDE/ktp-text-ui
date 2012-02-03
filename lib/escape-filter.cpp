/*
    Copyright (C) 2012  Lasath Fernando <kde@lasath.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "filters.h"

#include <QtGui/QTextDocument> //needed for Qt::escape

EscapeFilter::EscapeFilter(QObject *parent)
    : AbstractMessageFilter(parent)
{
}

void EscapeFilter::filterMessage(Message& message)
{
    QString escapedMessage = Qt::escape(message.mainMessagePart());

    escapedMessage.replace(QLatin1String("\n "), QLatin1String("<br/>&nbsp;")); //keep leading whitespaces
    escapedMessage.replace(QLatin1Char('\n'), QLatin1String("<br/>"));
    escapedMessage.replace(QLatin1Char('\t'), QLatin1String("&nbsp; &nbsp; ")); // replace tabs by 4 spaces
    escapedMessage.replace(QLatin1String("  "), QLatin1String(" &nbsp;")); // keep multiple whitespaces
    escapedMessage.replace(QLatin1Char('\\'), QLatin1String("\\\\")); //replace a single backslash with two backslashes.

    message.setMainMessagePart(escapedMessage);
}