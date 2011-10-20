/***************************************************************************
 *   Copyright (C) 2011 by David Edmundson <kde@davidedmundson.co.uk>      *
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

#ifndef CHATTEXTEDIT_H
#define CHATTEXTEDIT_H

#include <KTextEdit>

class ChatTextEdit : public KTextEdit
{
    Q_OBJECT
public:
    explicit ChatTextEdit(QWidget *parent = 0);

    // reimplemented
    QSize minimumSizeHint() const;
    QSize sizeHint() const;

protected:
    void keyPressEvent(QKeyEvent *e);

    bool event(QEvent *);

    // reimplemented
    void resizeEvent(QResizeEvent*);

private Q_SLOTS:
    void recalculateSize();
    void updateScrollBar();

Q_SIGNALS:
    void returnKeyPressed();

public Q_SLOTS:
    /** wraps setFontWeight to a simple on/off bold) */
    void setFontBold(bool);
};

#endif // CHATTEXTEDIT_H


