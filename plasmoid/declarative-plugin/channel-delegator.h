#ifndef CHANNELDELEGATOR_H
#define CHANNELDELEGATOR_H

#include <TelepathyQt/Account>

//BODGE CLASS to expose dispatcherInterface
class AccountProxy : public Tp::Account
{
public:
    Tp::Client::ChannelDispatcherInterface* dispatcherInterface() {
        return Tp::Account::dispatcherInterface();
    }
};

//BODGE class to allow delegating channels before I merge into TpQt properly
class ChannelDelegator {
public:
    static void delegateChannel(const Tp::AccountPtr account, const Tp::ChannelPtr channel, QDateTime userActionTime = QDateTime::currentDateTime()) {
        static_cast<AccountProxy*>(account.data())->dispatcherInterface()->DelegateChannels(Tp::ObjectPathList() << QDBusObjectPath(channel->objectPath()), userActionTime.toTime_t(), QLatin1String("org.freedesktop.Telepathy.Client.KTp.TextUi"));
    }
};

#endif // CHANNELDELEGATOR_H
