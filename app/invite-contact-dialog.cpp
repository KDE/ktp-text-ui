/*
 * Contact Chooser Dialog
 *
 * Copyright (C) 2011 David Edmundson <kde@davidedmundson.co.uk>
 * Copyright (C) 2012 Daniele E. Domenichelli <daniele.domenichelli@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include "invite-contact-dialog.h"

#include <KDE/KLineEdit>
#include <KDE/KPushButton>
#include <KDE/KLocalizedString>
#include <KDE/KDebug>

#include <TelepathyQt/Contact>
#include <TelepathyQt/TextChannel>

#include <KTp/debug.h>
#include <KTp/Models/contacts-model.h>
#include <KTp/Models/accounts-filter-model.h>
#include <KTp/Widgets/contact-grid-widget.h>
#include <telepathy-qt4/TelepathyQt/PendingChannelRequest>

InviteContactDialog::InviteContactDialog(const Tp::AccountPtr &account, const Tp::TextChannelPtr &channel, QWidget *parent) :
    KDialog(parent),
    m_account(account),
    m_channel(channel)
{
    resize(500,450);

    m_contactsModel = new ContactsModel(this);
    m_contactsModel->onNewAccount(account);


    m_contactGridWidget = new KTp::ContactGridWidget(m_contactsModel, this);
    m_contactGridWidget->contactFilterLineEdit()->setClickMessage(i18n("Search in Contacts..."));
    m_contactGridWidget->filter()->setPresenceTypeFilterFlags(AccountsFilterModel::ShowOnlyConnected);
    setMainWidget(m_contactGridWidget);

    connect(m_contactGridWidget,
            SIGNAL(selectionChanged(Tp::AccountPtr,Tp::ContactPtr)),
            SLOT(onChanged()));

    button(KDialog::Ok)->setDisabled(true);

    connect(this, SIGNAL(okClicked()), SLOT(onOkClicked()));
    connect(this, SIGNAL(rejected()), SLOT(close()));
}

Tp::AccountPtr InviteContactDialog::account() const
{
    return m_account;
}

Tp::TextChannelPtr InviteContactDialog::channel() const
{
    return m_channel;
}

void InviteContactDialog::onOkClicked()
{
    // don't do anytghing if no contact has been selected
    if (!m_contactGridWidget->hasSelection()) {
        return;
    }

    Tp::ContactPtr contact = m_contactGridWidget->selectedContact();

    if (contact.isNull() || m_channel.isNull() || m_account.isNull()) {
        return;
    }

    //if can invite do so, otherwise make a new channel with the new contacts
    if (m_channel->canInviteContacts()) {
        m_channel->inviteContacts(QList<Tp::ContactPtr>() << contact);
    }
    else {
        QList<Tp::ContactPtr> contacts;
        contacts << contact;
        contacts << m_channel->groupContacts(false).toList();
        m_account->createConferenceTextChat(QList<Tp::ChannelPtr>() << m_channel, contacts);
    }
}

void InviteContactDialog::onChanged()
{
    button(KDialog::Ok)->setEnabled(m_contactGridWidget->hasSelection());
}
