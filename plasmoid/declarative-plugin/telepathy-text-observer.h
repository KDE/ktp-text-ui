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


#ifndef TELEPATHY_TEXT_OBSERVER_H
#define TELEPATHY_TEXT_OBSERVER_H

#include "ktpchat_export.h"

#include "conversations-model.h"

#include <TelepathyQt/AbstractClientHandler>


class KDE_TELEPATHY_CHAT_EXPORT TelepathyTextObserver : public QObject
{
Q_OBJECT

Q_PROPERTY(QObject* conversations READ conversationModel CONSTANT)

public:
    TelepathyTextObserver(QObject* parent=0);
    ~TelepathyTextObserver();

    QAbstractListModel* conversationModel();

private:
    Tp::SharedPtr<ConversationsModel> m_handler;
    Tp::ClientRegistrarPtr m_registrar;
};

#endif // CONVERSATION_WATCHER_H
