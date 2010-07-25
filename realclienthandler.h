#ifndef REALCLIENTHANDLER_H
#define REALCLIENTHANDLER_H

#include <TelepathyQt4/AbstractClientHandler>
#include <TelepathyQt4/types.h>
#include <TelepathyQt4/Channel>
#include <TelepathyQt4/TextChannel>
#include <TelepathyQt4/ReceivedMessage>
#include <chatconnection.h>

using namespace Tp;

inline ChannelClassList channelClassList();

//TODO rename to something better, when I think of it.
class RealClientHandler : public QObject, public AbstractClientHandler
{
    Q_OBJECT
public:
    RealClientHandler();

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

signals:
    void newConnection(ChatConnection* newConnection);

};

#endif // REALCLIENTHANDLER_H
