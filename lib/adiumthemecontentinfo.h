#ifndef ADIUMTHEMECONTENTINFO_H
#define ADIUMTHEMECONTENTINFO_H
#include "adiumthememessageinfo.h"

class QString;
class AdiumThemeContentInfoPrivate;



class AdiumThemeContentInfo : AdiumThemeMessageInfo
{
public:
    explicit AdiumThemeContentInfo(AdiumThemeMessageInfo::MessageType);

    /** Path to the user icon associated with this message */
    QString userIconPath() const;
    void setUserIconPath(const QString& userIconPath);

    /** The screen name (UID, ID, member name, etc.) of the sender of this message.*/
    QString senderScreenName() const;
    void setSenderScreenName(const QString senderScreenName);

    /** The name of the sender of this message as the user's preferences request it.*/
    QString sender() const;

    /** A color derived from the user's name*/
    //FIXME what is this talking about...?
    QString senderColor() const;
    void setSenderColor(const QString& senderColor);

    /** The path to the status icon of the sender (available, away, etc...) */
    QString senderStatusIcon() const;
    void setSenderStatusIcon(const QString& senderStatusIcon);

    /** The text direction of the message (either rtl or ltr) */
    QString messageDirection() const;

    /** The serverside (remotely set) name of the sender, such as an MSN display name.*/
    QString senderDisplayName() const;
    void setSenderDisplayName(const QString& senderDisplayName);

    //textBackgroundColor

private:
    AdiumThemeContentInfoPrivate* d;
};

#endif // ADIUMTHEMECONTENTINFO_H
