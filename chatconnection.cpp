#include "chatconnection.h"
#include <TelepathyQt4/PendingReady>
#include <TelepathyQt4/ContactManager>

ChatConnection::ChatConnection(QObject *parent, const AccountPtr account, const ConnectionPtr connection,  QList<ChannelPtr> channels)
        : QObject(parent),
        m_account(account),
        m_connection(connection)
{
    //FIXME loop through, find text channel
    if (channels.length() == 1)
    {
        m_channel = Tp::TextChannelPtr::dynamicCast(channels[0]);
        PendingReady* op = m_channel->becomeReady(Features() << TextChannel::FeatureMessageQueue
                           << TextChannel::FeatureMessageSentSignal
                           << TextChannel::FeatureChatState
                           << Channel::FeatureCore
                                                 );



        connect(op, SIGNAL(finished(Tp::PendingOperation*)), this, SLOT(onChannelReady(Tp::PendingOperation*)));

    }
    else
    {
        qDebug() << "more than one channel?"; // I don't understand channels yet.
    }

}


//Private slots

void ChatConnection::onChannelReady(Tp::PendingOperation*)
{
    m_connection->contactManager()->upgradeContacts(QList<ContactPtr>::fromSet(m_channel->groupContacts()), QSet<Contact::Feature>() <<
                                                    Contact::FeatureAlias <<
                                                    Contact::FeatureAvatarToken << Contact::FeatureCapabilities << Contact::FeatureSimplePresence
                                                    );

    qDebug() << "channel ready";

    //m_isChannelReady = true;
    emit(channelReadyStateChanged(true));
}
