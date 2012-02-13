/*
    Copyright (C) 2011  Lasath Fernando <kde@lasath.org>

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


#ifndef CONVERSATION_H
#define CONVERSATION_H

#include "ktpchat_export.h"

#include <QObject>
#include <TelepathyQt/Account>
#include <TelepathyQt/TextChannel>

#include <KIcon>

#include "conversation-que-manager.h"
#include "conversation-target.h"
#include "messages-model.h"

class ConversationTarget;
class MessagesModel;
class KDE_TELEPATHY_CHAT_EXPORT Conversation : public QObject
{
Q_OBJECT

Q_PROPERTY(QObject* target READ target CONSTANT);
Q_PROPERTY(QObject* messages READ messages CONSTANT);
Q_PROPERTY(bool valid READ isValid NOTIFY validityChanged);

public:
    Conversation(const Tp::TextChannelPtr &channel, const Tp::AccountPtr &account, QObject *parent = 0);
    Conversation(QObject *parent = 0);
    virtual ~Conversation();

    MessagesModel* messages() const;
    ConversationTarget* target() const;

    bool isValid();

Q_SIGNALS:
    void validityChanged(bool isValid);

public Q_SLOTS:
    void delegateToProperClient();
    void requestClose();

private Q_SLOTS:
    void onChannelInvalidated(Tp::DBusProxy *proxy, const QString &errorName, const QString &errorMessage);

private:
    class ConversationPrivate;
    ConversationPrivate *d;
};

Q_DECLARE_METATYPE(Conversation*);

#endif // CONVERSATION_H
