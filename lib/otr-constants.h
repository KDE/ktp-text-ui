#ifndef OTR_CONSTANTS_HEADER
#define OTR_CONSTANTS_HEADER

#include <KGlobal>

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


/**
 * OTR message event enum type - the same as OtrlMessageEvent in libotr
 */
enum OTRMessageEvent 
{
    OTRL_MSGEVENT_NONE,
    OTRL_MSGEVENT_ENCRYPTION_REQUIRED,
    OTRL_MSGEVENT_ENCRYPTION_ERROR,
    OTRL_MSGEVENT_CONNECTION_ENDED,
    OTRL_MSGEVENT_SETUP_ERROR,
    OTRL_MSGEVENT_MSG_REFLECTED,
    OTRL_MSGEVENT_MSG_RESENT,
    OTRL_MSGEVENT_RCVDMSG_NOT_IN_PRIVATE,
    OTRL_MSGEVENT_RCVDMSG_UNREADABLE,
    OTRL_MSGEVENT_RCVDMSG_MALFORMED,
    OTRL_MSGEVENT_LOG_HEARTBEAT_RCVD,
    OTRL_MSGEVENT_LOG_HEARTBEAT_SENT,
    OTRL_MSGEVENT_RCVDMSG_GENERAL_ERR,
    OTRL_MSGEVENT_RCVDMSG_UNENCRYPTED,
    OTRL_MSGEVENT_RCVDMSG_UNRECOGNIZED,
    OTRL_MSGEVENT_RCVDMSG_FOR_OTHER_INSTANCE
};

}

/**
 * \ingroup ifacestrconsts
 *
 * The interface name "org.freedesktop.Telepathy.Channel.Interface.OTR1" as a QLatin1String, usable in QString requiring contexts even when
 * building with Q_NO_CAST_FROM_ASCII defined.
 */
#define TP_QT_IFACE_CHANNEL_INTERFACE_OTR1 (QLatin1String("org.freedesktop.Telepathy.Channel.Interface.OTR1"))


/**
 * OTR message headers
 */
#define OTR_MESSAGE_EVENT_HEADER       (QLatin1String("otr-message-event"))
#define OTR_REMOTE_FINGERPRINT_HEADER  (QLatin1String("otr-remote-fingerprint"))
#define OTR_ERROR_HEADER               (QLatin1String("otr-error"))
#define OTR_UNENCRYPTED_MESSAGE_HEADER (QLatin1String("otr-unencrypted-message"))

#endif
