#ifndef OTR_CONSTANTS_HEADER
#define OTR_CONSTANTS_HEADER

namespace Tp
{
    
/**
 * \enum OTRTrustLevel
 * \ingroup enumtypeconsts
 *
 * Enumerated type generated from the specification.
 * 
 * Enumeration describing trust level of this conversation. The trust level 
 * can only increase unless Initialize/Stop are called or TrustFingerprint is 
 * called with trust=false.
 */
enum OTRTrustLevel
{
    /**
     * The conversation is currently unencrypted
     */
    OTRTrustLevelNotPrivate = 0,

    /**
     * The conversation is currently encrypted but the remote end is not 
     * verified
     */
    OTRTrustLevelUnverified = 1,

    /**
     * The conversation is currently encrypted and the remote end is verified
     */
    OTRTrustLevelPrivate = 2,

    /**
     * Remote end closed the OTR session, messages cannot be sent anymore. 
     * Either call Stop to continue unencrypted or Initialize to send 
     * encrypted messages again.
     */
    OTRTrustLevelFinished = 3,

    _OTRTrustLevelPadding = 0xffffffffU
};

/**
 * \ingroup enumtypeconsts
 *
 * 1 higher than the highest valid value of OTRTrustLevel.
 */
const int NUM_OTR_TRUST_LEVELS = (3+1);

}

/**
 * \ingroup ifacestrconsts
 *
 * The interface name "org.freedesktop.Telepathy.Channel.Interface.OTR1" as a QLatin1String, usable in QString requiring contexts even when
 * building with Q_NO_CAST_FROM_ASCII defined.
 */
#define TP_QT_IFACE_CHANNEL_INTERFACE_OTR1 (QLatin1String("org.freedesktop.Telepathy.Channel.Interface.OTR1"))

#endif
