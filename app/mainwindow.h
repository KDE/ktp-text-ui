#ifndef REALCLIENTHANDLER_H
#define REALCLIENTHANDLER_H

#include <KTabWidget>

#include <TelepathyQt4/AbstractClientHandler>
#include <TelepathyQt4/types.h>
#include <TelepathyQt4/Channel>
#include <TelepathyQt4/TextChannel>
#include <TelepathyQt4/ReceivedMessage>
#include <chatconnection.h>

using namespace Tp;

inline ChannelClassList channelClassList();

//In the future I want to have a (potential) list of tab widgets. Like Kopete presently. This may need a bit of a rewrite.

class MainWindow : public KTabWidget, public AbstractClientHandler
{
    Q_OBJECT
public:
    MainWindow();

    virtual void handleChannels(const MethodInvocationContextPtr<> &context,
                                const AccountPtr & account,
                                const ConnectionPtr & connection,
                                const QList< ChannelPtr > & channels,
                                const QList< ChannelRequestPtr > & requestsSatisfied,
                                const QDateTime &  userActionTime,
                                const QVariantMap & handlerInfo
                               );

    bool bypassApproval() const
    {
        return false;
    }

};

#endif // REALCLIENTHANDLER_H
