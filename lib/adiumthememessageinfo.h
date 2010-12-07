#ifndef ADIUMTHEMEMESSAGEINFO_H
#define ADIUMTHEMEMESSAGEINFO_H

class QString;
class QDateTime;
class AdiumThemeMessageInfoPrivate;

class AdiumThemeMessageInfo
{
public:
    enum MessageType {
        RemoteToLocal,
        LocalToRemote,
        Status
    };

    explicit AdiumThemeMessageInfo(MessageType);
    explicit AdiumThemeMessageInfo(const AdiumThemeMessageInfo &other);
    ~AdiumThemeMessageInfo();
    AdiumThemeMessageInfo &operator=(const AdiumThemeMessageInfo &other);

    MessageType type() const;

    /** The message itself of the message/status. */
    QString message() const;
    void setMessage(const QString& message);

    /** The time at which message/status occurred*/
    QDateTime time() const;
    void setTime(const QDateTime& time);

    QString service() const;
    void setService(const QString& service);

    /** Will be replaced with "showIcons" if the "Show user icons" checkbox is selected,*/
    //FIXME in here or in AdiumThemeView..?
    QString userIcons() const;

    QString messageClasses() const;
    void appendMessageClass(const QString& messageClass);

private:
    AdiumThemeMessageInfoPrivate* d;
};

#endif // ADIUMTHEMEMESSAGEINFO_H
