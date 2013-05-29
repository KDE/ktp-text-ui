/*
 * Contact Delegate
 *
 * Copyright (C) 2010-2011 Collabora Ltd. <info@collabora.co.uk>
 *   @Author Dario Freddi <dario.freddi@collabora.co.uk>
 * Copyright (C) 2011 Martin Klapetek <martin.klapetek@gmail.com>
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

#ifndef CONTACTDELEGATE_H
#define CONTACTDELEGATE_H

#include "abstract-contact-delegate.h"

class ContactDelegate : public AbstractContactDelegate
{
    Q_OBJECT

public:
    ContactDelegate(QObject *parent = 0);
    ~ContactDelegate();

    virtual void paintContact(QPainter *painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
    virtual QSize sizeHintContact(const QStyleOptionViewItem & option, const QModelIndex & index) const;


public Q_SLOTS:
protected:

private:
    int         m_avatarSize;
    int         m_presenceIconSize;
    int         m_spacing;
};

#endif // CONTACTDELEGATE_H
