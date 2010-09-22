#include "adiumthemecontentinfo.h"
#include <QString>

class AdiumThemeContentInfoPrivate
{
public:
    QString userIconPath;
    QString senderScreenName;
    QString sender;
    QString senderColor;
    QString senderStatusIcon;
    QString senderDisplayName;
    QString textbackgroundcolor;
};

AdiumThemeContentInfo::AdiumThemeContentInfo() :
        d(new AdiumThemeContentInfoPrivate)
{
}

QString AdiumThemeContentInfo::userIconPath() const
{
    return d->userIconPath;
}

void AdiumThemeContentInfo::setUserIconPath(const QString &userIconPath)
{
    d->userIconPath = userIconPath;
}

QString AdiumThemeContentInfo::senderScreenName() const
{
    return d->senderScreenName;
}

void AdiumThemeContentInfo::setSenderScreenName(const QString senderScreenName)
{
    d->senderScreenName = senderScreenName;
}

QString AdiumThemeContentInfo::sender() const
{
    return d->senderDisplayName;
}


QString AdiumThemeContentInfo::senderColor() const
{
    return d->senderColor;
}

void AdiumThemeContentInfo::setSenderColor(const QString &senderColor)
{
    d->senderColor = senderColor;
}

QString AdiumThemeContentInfo::senderStatusIcon() const
{
    return d->senderStatusIcon;
}

void AdiumThemeContentInfo::setSenderStatusIcon(const QString &senderStatusIcon)
{
    d->senderStatusIcon = senderStatusIcon;
}

QString AdiumThemeContentInfo::messageDirection() const
{
    //FIXME
    return "rtl";
}

QString AdiumThemeContentInfo::senderDisplayName() const
{
    return d->senderDisplayName;
}

void AdiumThemeContentInfo::setSenderDisplayName(const QString &senderDisplayName)
{
    d->senderDisplayName = senderDisplayName;
}
