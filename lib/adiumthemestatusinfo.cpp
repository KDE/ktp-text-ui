#include "adiumthemestatusinfo.h"
#include <QString>

class AdiumThemeStatusInfoPrivate
{
public:
    QString status;
};

AdiumThemeStatusInfo::AdiumThemeStatusInfo():
        AdiumThemeMessageInfo(AdiumThemeMessageInfo::Status),
        d(new AdiumThemeStatusInfoPrivate)
{
}

AdiumThemeStatusInfo::AdiumThemeStatusInfo(const AdiumThemeStatusInfo &other) :
    AdiumThemeMessageInfo(other),
    d(new AdiumThemeStatusInfoPrivate(*other.d))
{

}

AdiumThemeStatusInfo::~AdiumThemeStatusInfo()
{
    delete d;
}


AdiumThemeStatusInfo& AdiumThemeStatusInfo::operator=(const AdiumThemeStatusInfo& other)
{
    *d = *other.d;
    return *this;
}

QString AdiumThemeStatusInfo::status() const
{
    return d->status;
}

void AdiumThemeStatusInfo::setStatus(const QString& status)
{
    d->status = status;
}

