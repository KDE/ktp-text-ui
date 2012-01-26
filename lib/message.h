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


#ifndef KTP_MESSAGE_H
#define KTP_MESSAGE_H

#include <TelepathyQt/Message>

namespace KTp {

class Message {

public:
    Message(Tp::Message& original);

    QString mainMessagePart() const;
    void setMainMessagePart(const QString& message);
    void appendMessagePart(const QString& part);

    QString finalizedMessage() const;

    QVariantMap& miscData();
private:
    Tp::Message originalMessage;
    QVariantMap m_miscData;
    QStringList content;

    enum MessageParts {
        MainMessage = 0
    };
};

}

#endif // KTP_MESSAGE_H
