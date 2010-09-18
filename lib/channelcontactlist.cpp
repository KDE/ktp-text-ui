#include "channelcontactlist.h"
#include <TelepathyQt4/Contact>

/** \breif This class provides an abstracted way of getting presence changes from a contact
  * It was needed to be able to pair up a contact changing, with the name of the contact.
  * It gives a single signal for the calling code to handle for every contact, and
  * also safetly handles connect and disconnecting slots as contacts enter and leave a channel
*/

/** Internal private class*/

/*
 [23:50] <oggis_> if you want to show events just for the current members, then i'd maintain said list/set by connecting to
groupMembersChanged and adding to it everybody in the groupMembersAdded set, and removing to it everybody in any of the
other sets (taking set union of them is easiest)
 */




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
        qDebug() << contact->alias();
    }
    connect(channel.data(), SIGNAL(groupMembersChanged(Tp::Contacts, Tp::Contacts, Tp::Contacts, Tp::Contacts, Tp::Channel::GroupMemberChangeDetails)),
            SLOT(groupMembersChanged(Tp::Contacts, Tp::Contacts, Tp::Contacts, Tp::Contacts, Tp::Channel::GroupMemberChangeDetails)));
}

void ChannelContactList::groupMembersChanged(const Tp::Contacts &groupMembersAdded, const Tp::Contacts &groupLocalPendingMembersAdded, const Tp::Contacts &groupRemotePendingMembersAdded, const Tp::Contacts &groupMembersRemoved, const Tp::Channel::GroupMemberChangeDetails &details)
{
    qDebug() << "members changed.";
}

