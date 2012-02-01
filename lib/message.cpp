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

#include <QtGui/QTextDocument> //needed for Qt::escape


Message::Message(const Tp::Message &original)
    : m_originalMessage(original)
{
    QString htmlMessage= Qt::escape(m_originalMessage.text());
    htmlMessage.replace(QLatin1String("\n "), QLatin1String("<br/>&nbsp;")); //keep leading whitespaces
    htmlMessage.replace(QLatin1Char('\n'), QLatin1String("<br/>"));
    htmlMessage.replace(QLatin1Char('\t'), QLatin1String("&nbsp; &nbsp; ")); // replace tabs by 4 spaces
    htmlMessage.replace(QLatin1String("  "), QLatin1String(" &nbsp;")); // keep multiple whitespaces
    htmlMessage.replace(QLatin1Char('\\'), QLatin1String("\\\\")); //replace a single backslash with two backslashes.

    setMainMessagePart(htmlMessage);
}

QString Message::mainMessagePart() const
{
    return m_content[Message::MainMessage];
}

void Message::setMainMessagePart(const QString& message)
{
    //FIXME there must be a better way to do this.
    if (m_content.size() > 0) {
        m_content[Message::MainMessage] = message;
    }
    else {
        m_content.append(message);
    }
}

void Message::appendMessagePart(const QString& part)
{
    m_content << part;
}

QString Message::finalizedMessage() const
{
    QString msg = m_content.join(QLatin1String("\n"));
    kDebug() << msg;
    return msg;
}

QVariant Message::property(const QString &name) const
{
    return m_properties[name];
}

void Message::setProperty(const QString &name, const QVariant &value)
{
    m_properties[name]=value;
}
