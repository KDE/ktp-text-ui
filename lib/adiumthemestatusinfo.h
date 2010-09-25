#ifndef ADIUMTHEMESTATUSINFO_H
#define ADIUMTHEMESTATUSINFO_H

#include "adiumthememessageinfo.h"
class QString;
class AdiumThemeStatusInfoPrivate;

class AdiumThemeStatusInfo : public AdiumThemeMessageInfo
{
public:
    explicit AdiumThemeStatusInfo();

    /** A description of the status event. This is neither in the user's local language nor expected to be displayed*/
    QString status() const;
    void setStatus(const QString& status);

private:
    AdiumThemeStatusInfoPrivate* d;
};

#endif // ADIUMTHEMESTATUSINFO_H
