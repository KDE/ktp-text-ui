#ifndef TELEPATHYCHATMESSAGEINFO_H
#define TELEPATHYCHATMESSAGEINFO_H
#include <QString>
#include <QDateTime>
#include <QUrl>


/** This class contains all the information that is needed for a message or status message*/

//FIXME enum type for incoming, outgoing or status. Set on construct.
//messageDirection only has getter based on this
//no value handling to be done in this class.

//rules for this class
//anything relating to telepathy should be set by whoever fills in this class
//anything relating to style only (i.e to show 12 or 24 hour time) should be handled by the chatview.

class TelepathyChatMessageInfo
{
public:
    TelepathyChatMessageInfo();

    //bother. I've documented the private stuff. I meant to do the getters + setters. Can't be bothered to move it now. I'm too sleepy.
    QString message() const {return m_message;}
    void setMessage(const QString message) {m_message = message;}

    QDateTime time() const {return m_time;}
    void setTime(const QDateTime time){m_time = time;}

    //FIXME add the rest..

    QString messageDirection() const {return m_messageDirection;}
    void setMessageDirection(const QString messageDirection) {m_messageDirection = messageDirection;}

    QString senderDisplayName() const {return m_senderDisplayName;}
    void setSenderDisplayName(const QString senderDisplayName) {m_senderDisplayName = senderDisplayName;}



private:
    //descriptions come from the data we need for Adium theme templates
    //http://trac.adium.im/wiki/CreatingMessageStyles

    //both status messages and regular messages

    /** The message itself of the message/status*/
    QString m_message;

    /** The time at which message/status occurred*/
    QDateTime m_time;

    /** A human readable description for the messaging service associated with this message, such as "AIM" or "MSN". */
    QString m_service;

    /** Will be replaced with "showIcons" if the "Show user icons" checkbox is selected, and will be replaced with "hideIcons" if the checkbox is deselected.*/
    //FIXME do above as an enum? replace in the filter.
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

    //FIXME enum this.
    QString m_messageDirection;

    /** The serverside (remotely set) name of the sender, such as an MSN display name.*/
    QString m_senderDisplayName;

    //FUTURE. This looks more confusing.
    //textbackgroundcolor{X}

};

#endif // TELEPATHYCHATMESSAGEINFO_H
