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


#include "conversation-que-manager.h"
#include <KDebug>

class ConversationQueManager::ConversationQueManagerPrivate {
public:
    QList<Queable*> que;
    KAction* gloablAction;
};

void Queable::enqueSelf()
{
    m_queManager->enque(this);
}

void Queable::removeSelfFromQue()
{
    m_queManager->remove(this);
}

Queable::~Queable()
{
}

Queable::Queable(ConversationQueManager* que)
    : m_queManager(que)
{
    if(!que) {
        m_queManager = ConversationQueManager::instance();
    }
}

ConversationQueManager* ConversationQueManager::instance()
{
    static ConversationQueManager* m_instance = 0;

    if(!m_instance) {
        m_instance = new ConversationQueManager();
    }

    return m_instance;
}

ConversationQueManager::ConversationQueManager(QObject* parent):
    QObject(parent),
    d(new ConversationQueManagerPrivate)
{
    kDebug();

    //FIXME: think of a good name for this. What did Kopete call it?
    d->gloablAction = new KAction(this);
    d->gloablAction->setObjectName(QLatin1String("next-unread-conversation"));
    d->gloablAction->setGlobalShortcut(KShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_I)/*, KAction::ActiveShortcut | KAction::DefaultShortcut, KAction::NoAutoloading*/);

    connect(d->gloablAction, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), SLOT(dequeNext()));
}

void ConversationQueManager::dequeNext()
{
    kDebug();

    if(!d->que.isEmpty()) {
        d->que.takeLast()->selfDequed();
    }
}

void ConversationQueManager::enque(Queable* item)
{
    if(!d->que.contains(item)) {
        d->que.append(item);
    }
}

void ConversationQueManager::remove(Queable* item)
{
    if(d->que.contains(item)) {
        d->que.removeAll(item);
    }
}

ConversationQueManager::~ConversationQueManager()
{
    delete d;
}
