#include "mainwindow.h"
#include "chatwindow.h"

inline ChannelClassList channelClassList()
{
    ChannelClassList filters;
    QMap<QString, QDBusVariant> filter;
    filter.insert(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".ChannelType"),
                  QDBusVariant(TELEPATHY_INTERFACE_CHANNEL_TYPE_TEXT));
    filters.append(filter);
    return filters;
}


MainWindow::MainWindow() :
        KTabWidget(),
        AbstractClientHandler(channelClassList())
{
}


void MainWindow::handleChannels(const MethodInvocationContextPtr<> &context,
                                       const AccountPtr & account,
                                       const ConnectionPtr & connection,
                                       const QList< ChannelPtr > & channels,
                                       const QList< ChannelRequestPtr > & ,
                                       const QDateTime & ,
                                       const QVariantMap&
                                      )
{
    ChatConnection* chatConnection = new ChatConnection(this, account, connection, channels);
    ChatWindow* newWindow = new ChatWindow(chatConnection, this);

    addTab(newWindow,"test");
    resize(newWindow->sizeHint());// FUDGE

    context->setFinished();
}

