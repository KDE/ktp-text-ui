/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

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



Queable::Queable(ConversationQueManager* que)
{
    if(que) {
        m_queManager = que;
    } else {
        m_queManager = ConversationQueManager::instance();
    }
}

void Queable::push()
{
    if(!m_queManager->que.contains(this)) {
        m_queManager->que.append(this);
    }
}

ConversationQueManager* ConversationQueManager::instance()
{
    static ConversationQueManager* m_instance = 0;

    if(!m_instance) {
        m_instance = ConversationQueManager();
    }

    return m_instance;
}

ConversationQueManager::ConversationQueManager(QObject* parent): QObject(parent)
{
    //FIXME: think of a good name for this. What did Kopete call it?
    m_gloablAction = new KAction("cycle-unread-conversations");
    m_gloablAction->setGlobalShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_I);

    connect(m_gloablAction, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), SLOT(popConversation()));
}

void ConversationQueManager::popConversation()
{
    if(!que.isEmpty()) {
        que.takeLast()->pop();
    }
}