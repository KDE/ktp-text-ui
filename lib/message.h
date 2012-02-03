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


#ifndef MESSAGE_H
#define MESSAGE_H

#include <TelepathyQt/Message>

class Message {

public:
    Message(const Tp::Message& original);

    QString mainMessagePart() const;
    void setMainMessagePart(const QString& message);
    void appendMessagePart(const QString& part);

    QString finalizedMessage() const;

    QVariant property(const char *name) const;
    void setProperty(const char *name, const QVariant &value);

private:
    Tp::Message m_originalMessage;
    QVariantMap m_properties;
    QString     m_mainPart;
    QStringList m_parts;
};


#endif // MESSAGE_H
