/*
 * Copyright (C) 2011 Martin Klapetek <martin.klapetek@gmail.com>
 * Copyright (C) 2013  Daniel Vrátil <dvratil@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */


#ifndef ENTITYVIEWDELEGATE_H
#define ENTITYVIEWDELEGATE_H

#include <QtGui/QStyledItemDelegate>

class EntityViewDelegate : public QStyledItemDelegate
{
    Q_OBJECT

  public:
    explicit EntityViewDelegate(QObject* parent = 0);
    virtual ~EntityViewDelegate();

  public:
    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;

  private:
    void paintContact(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    QSize sizeHintContact(const QStyleOptionViewItem& option, const QModelIndex& index) const;

    void paintHeader(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    QSize sizeHintHeader(const QStyleOptionViewItem& option, const QModelIndex& index) const;

  private:
    int m_avatarSize;
    int m_spacing;
};

#endif // ENTITYVIEWDELEGATE_H
