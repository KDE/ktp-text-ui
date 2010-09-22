#include "adiumthemestatusinfo.h"
#include <QString>

class AdiumThemeStatusInfoPrivate
{
public:
    QString status;
};

AdiumThemeStatusInfo::AdiumThemeStatusInfo():
        d(new AdiumThemeStatusInfoPrivate)
{
}

QString AdiumThemeStatusInfo::status() const
{
    return d->status;
}

void AdiumThemeStatusInfo::setStatus(const QString& status)
{
    d->status = status;
}

