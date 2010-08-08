/***************************************************************************
 *   Copyright (C) 2010 by David Edmundson <kde@davidedmundson.co.uk>      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#ifndef TELEPATHYCHATINFO_H
#define TELEPATHYCHATINFO_H

#include <QString>
#include <QUrl>
#include <QDateTime>

/** Containts all the information needed for the header generation from the Adium/Kopete templates */

class TelepathyChatInfo
{
public:
    TelepathyChatInfo();

    /** A name for the chat. For a one-on-one chat, this will be the display name of the remote user. For a group chat, it is the group chat name or topic, as appropriate. */
    QString chatName() const {
        return m_chatName;
    };
    void setChatName(const QString chatName) {
        m_chatName = chatName;
    };

    /** The name of the source account for this chat. */
    QString sourceName() const {
        return m_sourceName;
    };
    void setSourceName(const QString sourceName) {
        m_sourceName = sourceName;
    };

    /** The name of the chat's destination.*/
    QString destinationName() const {
        return m_destinationName;
    }
    void setDestinationName(const QString destinationName) {
        m_destinationName = destinationName;
    };

    /** The serverside (remotely set) name of the chat's destination, such as an MSN display name. */
    QString destinationDisplayName() const {
        return m_destinationDisplayName;
    }
    void setDestinationDisplayName(const QString destinationDisplayName) {
        m_destinationDisplayName = destinationDisplayName;
    }

    //FIXME this documentation should say (optional) - then the handing of not setting it should be handled in the getter method - or even the constructor.
    /** A full path to the image of the remote user. If the remote user does not have an image, or this is a group chat so no image is appropriate, this tag will be replaced by incoming_icon.png, so you should have that file available if this tag is used. */
    QUrl incomingIconPath() const {
        return m_incomingIconPath;
    };
    void setIncomingIconPath(const QUrl incomingIconPath) {
        m_incomingIconPath = incomingIconPath;
    };

    /** A full path to the image for the local user. If the local user does not have an image this tag will be replaced by outgoing_icon.png, so you should have that file available if this tag is used. */
    QUrl outgoingIconPath() const {
        return m_outgoingIconPath;
    };
    void setOutgoingIconPath(const QUrl outgoingIconPath) {
        m_outgoingIconPath = outgoingIconPath;
    };

    /** The time at which the chat was opened. */
    QDateTime timeOpened() const {
        return m_timeOpened;
    };
    void setTimeOpened(const QDateTime timeOpened) {
        m_timeOpened = timeOpened;
    };

private:
    QString m_chatName;
    QString m_sourceName;
    QString m_destinationName;
    QString m_destinationDisplayName;
    QUrl m_incomingIconPath;
    QUrl m_outgoingIconPath;
    QDateTime m_timeOpened;

};

#endif // TELEPATHYCHATINFO_H
