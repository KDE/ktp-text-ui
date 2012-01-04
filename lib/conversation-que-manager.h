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


#ifndef CONVERSATION_QUE_MANAGER_H
#define CONVERSATION_QUE_MANAGER_H

#include <QtCore/QObject>
#include <KAction>

class ConversationQueManager;
class Queable
{
friend class ConversationQueManager;

protected:
    Queable(ConversationQueManager* que = 0);
    virtual ~Queable();

    void enqueSelf();
    void removeSelfFromQue();
    virtual void selfDequed() = 0;

private:
    ConversationQueManager* m_queManager;
};


class ConversationQueManager : public QObject
{
Q_OBJECT

public:
    static ConversationQueManager* instance();
    void enque(Queable* item);
    void remove(Queable* item);

public Q_SLOTS:
    void dequeNext();

private:
    explicit ConversationQueManager(QObject* parent = 0);
    virtual ~ConversationQueManager();

    class ConversationQueManagerPrivate;
    ConversationQueManagerPrivate *d;
};

#endif // CONVERSATION_QUE_MANAGER_H
