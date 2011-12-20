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


#ifndef CONVERSATION_TARGET_H
#define CONVERSATION_TARGET_H

#include <QObject>
#include <TelepathyQt/Contact>

#include "chat-widget.h"

#include "ktpchat_export.h"


class KDE_TELEPATHY_CHAT_EXPORT ConversationTarget : public QObject
{
Q_OBJECT

Q_PROPERTY(QIcon avatar READ avatar NOTIFY avatarChanged);
Q_PROPERTY(QString nick READ nick NOTIFY nickChanged);
Q_PROPERTY(QIcon presenceIcon READ presenceIcon NOTIFY presenceIconChanged);
Q_PROPERTY(QString presenceIconSource READ presenceIconSource NOTIFY presenceIconSourceChanged);
Q_PROPERTY(QString id READ id)

//turns out you can't have non QObjects as properties
// Q_PROPERTY(Tp::ContactPtr contact READ contact WRITE setContact NOTIFY contactChanged);

public:
    ConversationTarget( Tp::ContactPtr contact = Tp::ContactPtr());
    virtual ~ConversationTarget();

    QIcon   avatar() const;
    QString id() const;
    QString nick() const;
    QIcon   presenceIcon() const;
    QString presenceIconSource() const;

    Tp::ContactPtr contact() const;
    void setContact(Tp::ContactPtr contact);
  
    static QString iconSourceForPresence(Tp::ConnectionPresenceType presence);

    
Q_SIGNALS:
    void avatarChanged(QIcon avatar);
    void nickChanged(QString nick);
    void presenceIconChanged(QIcon icon);
    void presenceIconSourceChanged(QString source);

    void contactChanged(Tp::ContactPtr contact);

private Q_SLOTS:
    void onAvatarDataChanged ( Tp::AvatarData );
    void onPresenceChanged ( Tp::Presence );

private:
    void setupContactSignals(Tp::ContactPtr contact);
    void removeContactSignals(Tp::ContactPtr contact);

    class ConversationTargetPrivate;
    ConversationTargetPrivate *d;
};

#endif // CONVERSATION_TARGET_H
