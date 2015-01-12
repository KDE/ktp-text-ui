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

#include <QtCore/QPointer>

#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QVBoxLayout>

#include <KLocalizedString>

#include <TelepathyQt/Contact>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/PendingChannelRequest>

#include <KTp/debug.h>
#include <KTp/Models/contacts-list-model.h>
#include <KTp/Models/contacts-filter-model.h>
#include <KTp/Widgets/contact-grid-widget.h>

InviteContactDialog::InviteContactDialog(const Tp::AccountManagerPtr &accountManager, const Tp::AccountPtr &account, const Tp::TextChannelPtr &channel, QWidget *parent) :
    QDialog(parent),
    m_account(account),
    m_channel(channel),
    m_contactsModel(new KTp::ContactsListModel(this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok, this))
{
    resize(500,450);

    m_contactsModel->setAccountManager(accountManager);

    m_contactGridWidget = new KTp::ContactGridWidget(m_contactsModel, this);
    m_contactGridWidget->contactFilterLineEdit()->setPlaceholderText(i18n("Search in Contacts..."));
    m_contactGridWidget->filter()->setPresenceTypeFilterFlags(KTp::ContactsFilterModel::ShowOnlyConnected);
    m_contactGridWidget->filter()->setAccountFilter(account);
    m_contactGridWidget->setSelectionMode(QAbstractItemView::MultiSelection);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_contactGridWidget);
    mainLayout->addWidget(m_buttonBox);

    setWindowTitle(i18n("Select Contacts to Invite to Group Chat"));

    connect(m_contactGridWidget,
            SIGNAL(selectionChanged(Tp::AccountPtr,KTp::ContactPtr)),
            SLOT(onChanged()));

    m_buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);

    connect(m_buttonBox->button(QDialogButtonBox::Ok), &QAbstractButton::clicked, this, &InviteContactDialog::onOkClicked);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &InviteContactDialog::close);
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

    QList<Tp::ContactPtr> contacts;
    Q_FOREACH (const KTp::ContactPtr &contact, m_contactGridWidget->selectedContacts()) {
        contacts << contact;
    }

    if (contacts.isEmpty() || m_channel.isNull() || m_account.isNull()) {
        return;
    }

    //if can invite do so, otherwise make a new channel with the new contacts
    if (m_channel->canInviteContacts()) {
        m_channel->inviteContacts(contacts);
    } else {
        m_account->createConferenceTextChat(QList<Tp::ChannelPtr>() << m_channel, contacts);
    }
}

void InviteContactDialog::onChanged()
{
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(m_contactGridWidget->hasSelection());
}
