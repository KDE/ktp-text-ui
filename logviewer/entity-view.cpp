/***************************************************************************
 *   Copyright (C) 2012 by David Edmundson <kde@davidedmundson.co.uk>      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#include "entity-view.h"

#include <KCmdLineArgs>

#include <TelepathyQt/Account>
#include <TelepathyLoggerQt4/Entity>

#include "entity-model.h"

EntityView::EntityView(QWidget *parent) :
    QTreeView(parent)
{
    setHeaderHidden(true);
}

void EntityView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    QTreeView::rowsInserted(parent, start, end);
    static bool loadedCurrentContact = false;

    if (loadedCurrentContact) {
        return;
    }

    if (KCmdLineArgs::parsedArgs()->count() == 2) {
        QString selectAccountId = KCmdLineArgs::parsedArgs()->arg(0);
        QString selectContactId = KCmdLineArgs::parsedArgs()->arg(1);

        for (int i = start; i <= end; i++) {
            QModelIndex index = model()->index(i, 0, parent);
            Tp::AccountPtr account = index.data(EntityModel::AccountRole).value<Tp::AccountPtr>();
            Tpl::EntityPtr contact = index.data(EntityModel::EntityRole).value<Tpl::EntityPtr>();

            if (account.isNull() || contact.isNull()) {
                continue;
            }

            if (selectAccountId == account->uniqueIdentifier() && selectContactId == contact->identifier()) {
                setCurrentIndex(index);
                loadedCurrentContact = true;
            }

        }
    }

    expandAll();
}
