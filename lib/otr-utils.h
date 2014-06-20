#ifndef OTR_UTILS_HEADER
#define OTR_UTILS_HEADER

#include "otr-constants.h"
#include <ktpchat_export.h>
#include <TelepathyQt/PendingVariant>
#include <TelepathyQt/Message>

class KDE_TELEPATHY_CHAT_EXPORT OtrStatus 
{
    public:
        /** Creates OtrStatus with bool() returning false */
        OtrStatus();
        /** Creates for given trust level. bool() returns true */
        OtrStatus(Tp::OTRTrustLevel trustLevel);

        /** Returns true if otr is supported */
        operator bool() const;
        bool operator!() const;

        bool operator==(const OtrStatus &other) const;
        bool operator!=(const OtrStatus &other) const;

        /** Returns valid OTRTrustLevel if and only if operator bool() returns true */
        Tp::OTRTrustLevel otrTrustLevel() const;

    private:
        bool otrImplemented;
        Tp::OTRTrustLevel trustLevel;

        friend class ChatWidget;
};

namespace Tp {
namespace Utils {

    /** Makes a block on pending operation with timout and returns its result */
    QVariant waitForOperation(const Tp::PendingVariant *pendingVariant, int timout = 3000);

    /** Returns true if message is generated internally by OTR implementation */
    bool isOtrEvent(const Tp::ReceivedMessage &message);

    /** Returns notification for a user assuming that the message is an otr event */
    QString processOtrMessage(const Tp::ReceivedMessage &message);
}
}

#endif
