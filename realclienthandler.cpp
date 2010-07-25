#include "realclienthandler.h"
#include <QMessageBox>


inline ChannelClassList channelClassList()
{
    ChannelClassList filters;
    QMap<QString, QDBusVariant> filter;
    filter.insert(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".ChannelType"),
                  QDBusVariant(TELEPATHY_INTERFACE_CHANNEL_TYPE_TEXT));
    //     filter.insert(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".TargetHandleType"),
    //                   QDBusVariant((uint) Tp::HandleTypeContact));
    filters.append(filter);
    return filters;
}


RealClientHandler::RealClientHandler() :
        QObject(),
        AbstractClientHandler(channelClassList())
{
}


void RealClientHandler::handleChannels(const MethodInvocationContextPtr<> &context,
                                       const AccountPtr & account,
                                       const ConnectionPtr & connection,
                                       const QList< ChannelPtr > & channels,
                                       const QList< ChannelRequestPtr > & requestsSatisfied,
                                       const QDateTime &  userActionTime,
                                       const QVariantMap & handlerInfo
                                      )
{
    ChatConnection* chatConnection = new ChatConnection(this, account, connection, channels);
    Q_EMIT(newConnection(chatConnection)); //FIXME FIXME FIXME I'm going all over the place with parenting here. It'll explode before I know it

    qDebug();
    context->setFinished();
}

