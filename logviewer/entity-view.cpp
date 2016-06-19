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

#include <KTp/Logger/log-entity.h>
#include <KTp/types.h>

#include <QAbstractItemModel>
#include <QCoreApplication>

#include <TelepathyQt/Account>

#include "person-entity-merge-model.h"

EntityView::EntityView(QWidget *parent) :
    QTreeView(parent)
{
    setHeaderHidden(true);
}

void EntityView::setModel(QAbstractItemModel *model)
{
    QTreeView::setModel(model);

    //check if any existing items contain the selected contact
    if (model->rowCount() > 0) {
        rowsInserted(QModelIndex(), 0, model->rowCount() -1);
    }
}

void EntityView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    QTreeView::rowsInserted(parent, start, end);
    static bool loadedCurrentContact = false;

    if (loadedCurrentContact) {
        return;
    }

    QModelIndex selectedIndex;
    QCommandLineParser parser;

    if (QCoreApplication::arguments().count() == 1 && KTp::kpeopleEnabled()) {
        const QString selectedPersonaId = QCoreApplication::arguments().at(0);
        for (int i = start; i <= end; i++) {
            const QModelIndex index = model()->index(i, 0, parent);
            if (index.data(KTp::PersonIdRole).toUrl().toString() == selectedPersonaId) {
                selectedIndex = index;
                break;
            }
        }
    } else if (QCoreApplication::arguments().count() == 2) {
        QString selectAccountId = QCoreApplication::arguments().at(0);
        QString selectContactId = QCoreApplication::arguments().at(1);

        for (int i = start; i <= end; i++) {
            QModelIndex index = model()->index(i, 0, parent);
            Tp::AccountPtr account = index.data(PersonEntityMergeModel::AccountRole).value<Tp::AccountPtr>();
            KTp::LogEntity entity = index.data(PersonEntityMergeModel::EntityRole).value<KTp::LogEntity>();
            if (account.isNull() || !entity.isValid()) {
                continue;
            }

            if (selectAccountId == account->uniqueIdentifier() && selectContactId == entity.id()) {
                selectedIndex = index;
                break;
            }
        }
    }

    if (selectedIndex.isValid()) {
        loadedCurrentContact = true;
        setCurrentIndex(selectedIndex);
        scrollTo(selectedIndex);
    } else {
        Q_EMIT noSuchContact();
    }

    expandAll();

}
