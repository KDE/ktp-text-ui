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


#include "message.h"
#include <KDebug>

#ifdef TELEPATHY_LOGGER_QT4_FOUND
#include <TelepathyLoggerQt4/TextEvent>
#endif

Message::Message(const Tp::Message &original) :
      m_sentTime(original.sent()),
      m_token(original.messageToken()),
      m_messageType(original.messageType())
{
    setMainMessagePart(original.text());
}

#ifdef TELEPATHY_LOGGER_QT4_FOUND
Message::Message(const Tpl::TextEventPtr &original) :
    m_sentTime(original->timestamp()),
    m_token(original->messageToken()),
    m_messageType(original->messageType())
{
    setMainMessagePart(original->message());
}
#endif

QString Message::mainMessagePart() const
{
    return m_mainPart;
}

void Message::setMainMessagePart(const QString& message)
{
    m_mainPart = message;
}

void Message::appendMessagePart(const QString& part)
{
    m_parts << part;
}

QString Message::finalizedMessage() const
{
    QString msg = m_mainPart + QLatin1String("\n") +
        m_parts.join(QLatin1String("\n"));

//     kDebug() << msg;
    return msg;
}

QVariant Message::property(const char *name) const
{
    return m_properties[QLatin1String(name)];
}

void Message::setProperty(const char *name, const QVariant& value)
{
    m_properties[QLatin1String(name)] = value;
}

QDateTime Message::time() const
{
    return m_sentTime;
}

QString Message::token() const
{
    return m_token;
}

Tp::ChannelTextMessageType Message::type() const
{
    return m_messageType;
}
