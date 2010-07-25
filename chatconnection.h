#ifndef CHATCONNECTION_H
#define CHATCONNECTION_H

#include <QObject>
#include <TelepathyQt4/Account>
#include <TelepathyQt4/Connection>
#include <TelepathyQt4/TextChannel>

using namespace Tp;

class ChatConnection : public QObject
{
    Q_OBJECT
public:
    explicit ChatConnection(QObject *parent, const AccountPtr, const ConnectionPtr,  QList<ChannelPtr>);

    const AccountPtr account()
    {
        return m_account;
    };
    const ConnectionPtr connection()
    {
        return m_connection;
    };
    const TextChannelPtr channel()
    {
        return m_channel;
    };


signals:
    void channelReadyStateChanged(bool newState);

public slots:



private:
    AccountPtr m_account;
    ConnectionPtr m_connection;
    TextChannelPtr m_channel;

private slots:
    void onChannelReady(Tp::PendingOperation*);

};

#endif // CHATCONNECTION_H


