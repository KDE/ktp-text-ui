#include "adiumthememessageinfo.h"
#include <QString>
#include <QStringList>
#include <QDateTime>

class AdiumThemeMessageInfoPrivate
{
public:
    QString message;
    QDateTime time;
    QString service;
    QStringList messageClasses;
    AdiumThemeMessageInfo::MessageType type;
};

AdiumThemeMessageInfo::AdiumThemeMessageInfo(MessageType type)
    : d(new AdiumThemeMessageInfoPrivate())
{
    d->type = type;
}

AdiumThemeMessageInfo::MessageType AdiumThemeMessageInfo::type() const
{
    return d->type;
}

QString AdiumThemeMessageInfo::message() const
{
    return d->message;
}

void AdiumThemeMessageInfo::setMessage(const QString& message)
{
    d->message = message;
}

QDateTime AdiumThemeMessageInfo::time() const
{
    return d->time;
}

void AdiumThemeMessageInfo::setTime(const QDateTime& time)
{
    d->time = time;
}

QString AdiumThemeMessageInfo::service() const
{
    return d->service;
}

void AdiumThemeMessageInfo::setService(const QString& service)
{
    d->service = service;
}

QString AdiumThemeMessageInfo::userIcons() const
{
    //FIXME.
    return QString("showIcons");
}

QString AdiumThemeMessageInfo::messageClasses() const {
    //in the future this will also contain history, consecutive, autoreply, status, event
    //these will be stored internally as flags

    QStringList classes = d->messageClasses;

    if (d->type == RemoteToLocal) {
        classes.append("incoming");
        classes.append("message");
    }

    if (d->type == LocalToRemote) {
        classes.append("outgoing");
        classes.append("message");
    }

    if (d->type == Status) {
        classes.append("status");
    }

    return classes.join(" ");
}


void AdiumThemeMessageInfo::appendMessageClass(const QString &messageClass)
{
    d->messageClasses.append(messageClass);
}
