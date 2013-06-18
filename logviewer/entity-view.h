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

#ifndef ENTITYVIEW_H
#define ENTITYVIEW_H

#include <QTreeView>

//model is loaded asynchronously so we need to select the correct element on each new element
//this is done in the view to avoid having to be careful with proxy models.

class EntityView : public QTreeView
{
    Q_OBJECT
public:
    explicit EntityView(QWidget *parent = 0);

Q_SIGNALS:
    void noSuchContact();

protected Q_SLOTS:
    void rowsInserted(const QModelIndex &parent, int start, int end);
};

#endif // ENTITYVIEW_H
