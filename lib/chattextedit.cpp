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

#include "chattextedit.h"

#include <QMenu>
#include <QContextMenuEvent>
#include <QAction>
#include <QDebug>
#include <QTimer>


class ChatTextEditPrivate
{
    QWidget* formatToolbar;
};

ChatTextEdit::ChatTextEdit(QWidget *parent) :
    QTextEdit(parent)
{
    setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere); // no need for horizontal scrollbar with this
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setMinimumHeight(0);

    connect(this, SIGNAL(textChanged()), SLOT(recalculateSize()));
}

void ChatTextEdit::setFontBold(bool isBold)
{
    if (isBold)
    {
        setFontWeight(QFont::Bold);
    }
    else
    {
        setFontWeight(QFont::Normal);
    }
}

void ChatTextEdit::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = createStandardContextMenu();
    menu->addActions(actions());
    menu->exec(event->globalPos());
    delete menu;
}

//Size code based on LineEdit in PSI
//Justin Karneges, Michail Pishchagin

QSize ChatTextEdit::minimumSizeHint() const
{
    QSize sh = QTextEdit::minimumSizeHint();
    sh.setHeight(fontMetrics().height() + 1);
    sh += QSize(0, QFrame::lineWidth() * 2);
    return sh;
}

QSize ChatTextEdit::sizeHint() const
{
    QSize sh = QTextEdit::sizeHint();
    sh.setHeight(int(document()->size().height()));
    sh += QSize(0, (QFrame::lineWidth() * 2) + 1);
    ((QTextEdit*)this)->setMaximumHeight(sh.height());

    return sh;
}

void ChatTextEdit::resizeEvent(QResizeEvent* e)
{
        QTextEdit::resizeEvent(e);
        QTimer::singleShot(0, this, SLOT(updateScrollBar()));
}

void ChatTextEdit::recalculateSize()
{
        updateGeometry();
        QTimer::singleShot(0, this, SLOT(updateScrollBar()));
}

void ChatTextEdit::updateScrollBar()
{
        setVerticalScrollBarPolicy(sizeHint().height() > height() ? Qt::ScrollBarAlwaysOn : Qt::ScrollBarAlwaysOff);
        ensureCursorVisible();
}
