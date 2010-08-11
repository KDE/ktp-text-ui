#ifndef CHANNELCONTACTLIST_H
#define CHANNELCONTACTLIST_H

#include <QObject>
#include <TelepathyQt4/TextChannel>
#include <TelepathyQt4/Contact>


class ChannelContactListContact: public QObject
{
    Q_OBJECT
public:
    explicit ChannelContactListContact(Tp::ContactPtr, QObject *parent);

signals:
    void contactPresenceChanged(Tp::ContactPtr contact, uint type);

private slots:
    void onSimplePresenceChanged(const QString &status, uint type);

private:
    Tp::ContactPtr m_contact;
};




class ChannelContactList : public QObject
{
    Q_OBJECT
public:
    explicit ChannelContactList(Tp::TextChannelPtr, QObject *parent = 0);

signals:
    void contactPresenceChanged(Tp::ContactPtr contact, uint type);

public slots:

};






#endif // CHANNELCONTACTLIST_H
