/*
 * Contact Delegate - compact version
 *
 * Copyright (C) 2011 Martin Klapetek <martin.klapetek@gmail.com>
 * Copyright (C) 2014 David Edmundson <davidedmundson@kde.org>
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

#include <QStyledItemDelegate>


#ifndef CONTACTDELEGATE_H
#define CONTACTDELEGATE_H

class ContactDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    ContactDelegate(QObject *parent = 0);
    ~ContactDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
    QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;

private:
    int m_spacing;
    int m_avatarSize;
    int m_presenceIconSize;
    int m_clientTypeIconSize;
};

#endif // CONTACTDELEGATECOMPACT_H
