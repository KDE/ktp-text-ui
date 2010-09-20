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

#ifndef TELEPATHYCHATMESSAGEINFO_H
#define TELEPATHYCHATMESSAGEINFO_H
#include <QString>
#include <QDateTime>
#include <QUrl>
#include <QStringList>

/** This class contains all the information that is needed for a message or status message*/

//no value handling to be done in this class.

//rules for this class
//anything relating to telepathy should be set by whoever fills in this class
//anything relating to style only (i.e to show 12 or 24 hour time) should be handled by the chatview.

class TelepathyChatMessageInfo
{


public:
    enum MessageType {
        RemoteToLocal,
        LocalToRemote,
        Status
    };

    TelepathyChatMessageInfo(MessageType);


    //FIXME HTML escape all QString returns.

    //bother. I've documented the private stuff. I meant to do the getters + setters. Can't be bothered to move it now. I'm too sleepy.

    MessageType type() const {
        return m_type;
    }

    QString message() const {
        return m_message;
    }
    void setMessage(const QString message) {
        m_message = message;
    }

    QDateTime time() const {
        return m_time;
    }
    void setTime(const QDateTime time) {
        m_time = time;
    }

    QString service() const {
        return m_service;
    }
    void setService(const QString service) {
        m_service = service;
    }



    //FIXME add the rest..
    QString senderScreenName() const {
        return m_senderScreenName;
    }
    void setSenderScreenName(const QString senderScreenName) {
        m_senderScreenName = senderScreenName;
    }

    /** The text direction of the message (either rtl or ltr)  */
    QString messageDirection() const {
        if (m_type == LocalToRemote) {
            return "ltr";
        } else {
            return "rtl";
        }
    }

    QString senderDisplayName() const {
        return m_senderDisplayName;
    }

    void setSenderDisplayName(const QString senderDisplayName) {
        m_senderDisplayName = senderDisplayName;
    }

    QString messageClasses() const {
        //in the future this will also contain history, consecutive, autoreply, status, event
        //these will be stored internally as flags

        QStringList classes;

        if (m_type == RemoteToLocal) {
            classes.append("incoming");
            classes.append("message");
        }

        if (m_type == LocalToRemote) {
            classes.append("outgoing");
            classes.append("message");
        }

        if (m_type == Status) {
            classes.append("status");
        }

        return classes.join(" ");
    }

    /** Will be replaced with "showIcons" if the "Show user icons" checkbox is selected, and will be replaced with "hideIcons" if the checkbox is deselected.*/
    //This is pure style - do in the chat view instead?
    QString userIcons() const;

private:
    //descriptions come from the data we need for Adium theme templates
    //http://trac.adium.im/wiki/CreatingMessageStyles

    //both status messages and regular messages

    MessageType m_type;

    /** The message itself of the message/status*/
    QString m_message;

    /** The time at which message/status occurred*/
    QDateTime m_time;

    /** A human readable description for the messaging service associated with this message, such as "AIM" or "MSN". */
    QString m_service;


    QString m_userIcons;

    /** A space separated list of type information for messages, suitable for use as a class attribute. Currently available types are listed below. */
    QString m_messageClasses;

    /** A description of the status event. This is neither in the user's local language nor expected to be displayed; it may be useful to use a different div class to present different types of status messages. The following is a list of some of the more important status messages; your message style should be able to handle being shown a status message not in this list, as even at present the list is incomplete and is certain to become out of date in the future: */
    QString m_status;

    /** main message Path to the user icon associated with this message. If the user does not have an icon available, a path to the buddy_icon.png file in the Incoming or Outgoing directory (whichever is appropriate for the message) will be substituted instead, so if your message style uses this tag those files should be supplied as defaults. */
    QUrl m_userIconPath;

    /** The screen name (UID, ID, member name, etc.) of the sender of this message. See %sender%. */
    QString m_senderScreenName;

    /** The name of the sender of this message as the user's preferences request it. This is the recommended keyword to use for displaying the sender of a message, as it allows proper user customization within Adium. */
    //FIXME this shouldn't be here maybe? only create in filter code and set to senderScreenName or senderDisplayName appropriately
    QString m_sender;

    /** The path to the status icon of the sender (available, away, etc...) */
    QUrl m_senderStatusIcon;

    /** The serverside (remotely set) name of the sender, such as an MSN display name.*/
    QString m_senderDisplayName;

    //FUTURE. This looks more confusing.
    //textbackgroundcolor{X}

};

#endif // TELEPATHYCHATMESSAGEINFO_H
