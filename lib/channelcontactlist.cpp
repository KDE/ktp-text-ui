#include "channelcontactlist.h"


/** \breif This class provides an abstracted way of getting presence changes from a contact
  * It was needed to be able to pair up a contact changing, with the name of the contact.
  * It gives a single signal for the calling code to handle for every contact, and
  * also safetly handles connect and disconnecting slots as contacts enter and leave a channel (future).
*/

/** Internal private class*/

ChannelContactListContact::ChannelContactListContact(Tp::ContactPtr contact, QObject *parent)
        : QObject(parent)
{
    m_contact = contact;
    connect(m_contact.data(), SIGNAL(simplePresenceChanged(const QString, uint, const QString)), SLOT(onSimplePresenceChanged(QString, uint)));
}

void ChannelContactListContact::onSimplePresenceChanged(const QString &status, uint type)
{
    Q_UNUSED(status);
    Q_EMIT contactPresenceChanged(m_contact, type);
}


ChannelContactList::ChannelContactList(Tp::TextChannelPtr channel, QObject *parent) :
        QObject(parent)
{
    foreach(Tp::ContactPtr contact, channel->groupContacts()) {
        //FIXME move this to a slot called "addContact" - also call this when chat gains a person.
        ChannelContactListContact*  contactProxy = new ChannelContactListContact(contact, this);
        connect(contactProxy, SIGNAL(contactPresenceChanged(Tp::ContactPtr, uint)), SIGNAL(contactPresenceChanged(Tp::ContactPtr, uint)));
    }
}

