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


#ifndef ENTITYMODEL_H
#define ENTITYMODEL_H

#include <QAbstractListModel>

#include <TelepathyQt/Types>

#include <TelepathyLoggerQt4/Entity>
#include <TelepathyQt/Account>


/**
    Lists all avilable entities.

    roles:
      - Qt::DisplayRole - name
      - Qt::DecorationRole - avatar
      - EntityModel::IdRole
      - EntityModel::TypeRole - EntityType (EntityTypeContact/Room/Self/Unknown)
      - EntityModel::EntityRole - relevant Tpl::EntityPtr
  */

namespace Tpl{
    class PendingOperation;
}


class EntityModelItem {
public:
    Tpl::EntityPtr entity;
    Tp::AccountPtr account;
};

class EntityModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Role {
        IdRole = Qt::UserRole,
        TypeRole,
        EntityRole,
        AccountRole
    };


    explicit EntityModel(QObject *parent = 0);
    void setAccountManager(const Tp::AccountManagerPtr &accountManager);

    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;

private Q_SLOTS:
    void onEntitiesSearchFinished(Tpl::PendingOperation*);

private:
    QList<EntityModelItem> m_entities;

};

Q_DECLARE_METATYPE(Tpl::EntityPtr);
Q_DECLARE_METATYPE(Tp::AccountPtr);


#endif // ENTITYMODEL_H
