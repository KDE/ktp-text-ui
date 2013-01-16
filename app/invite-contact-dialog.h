/*
 * Contact Chooser Dialog
 *
 * Copyright (C) 2012 David Edmundson <kde@davidedmundson.co.uk>
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


#ifndef INVITECONTACTDIALOG_H
#define INVITECONTACTDIALOG_H

#include <KDialog>
#include <TelepathyQt/Types>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/Account>

namespace Tp {
class PendingOperation;
}

namespace KTp {
class ContactGridWidget;
class ContactsListModel;
}


class InviteContactDialog : public KDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(InviteContactDialog)

public:
    InviteContactDialog(const Tp::AccountManagerPtr &accountManager, const Tp::AccountPtr &account, const Tp::TextChannelPtr &channel, QWidget *parent);
    Tp::AccountPtr account() const;
    Tp::TextChannelPtr channel() const;

private Q_SLOTS:
    void onOkClicked();
    void onChanged();

private:
    Tp::AccountPtr m_account;
    Tp::TextChannelPtr m_channel;
    KTp::ContactsListModel *m_contactsModel;
    KTp::ContactGridWidget *m_contactGridWidget;
};

#endif // CONTACTDIALOG_H
