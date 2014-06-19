#include "otr-utils.h"

#include <QEventLoop>
#include <QTimer>

OtrStatus::OtrStatus() 
    : otrImplemented(false) { }

OtrStatus::OtrStatus(Tp::OTRTrustLevel trustLevel) 
    : otrImplemented(true), trustLevel(trustLevel) { }

OtrStatus::operator bool() const {
    return otrImplemented;
}

bool OtrStatus::operator!() const {
    return !(OtrStatus::operator bool());
}

bool OtrStatus::operator==(const OtrStatus &other) const {

    if(otrImplemented != other.otrImplemented) return false;
    else if(otrImplemented) return trustLevel == other.trustLevel;
    else return true;
}

bool OtrStatus::operator!=(const OtrStatus &other) const {
    return !(*this == other);
}

Tp::OTRTrustLevel OtrStatus::otrTrustLevel() const {
    return trustLevel;
}

namespace Tp {
namespace Utils {

    QVariant waitForOperation(const Tp::PendingVariant *pendingVariant) {
        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);
        loop.connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
        loop.connect(pendingVariant, SIGNAL(finished(Tp::PendingOperation*)), &loop, SLOT(quit()));
        timer.start(3000); // 3s
        loop.exec();

        if(timer.isActive()) timer.stop();
        return pendingVariant->result();
    }
}
}
