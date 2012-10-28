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

#include "chat-text-edit.h"

#include <QtGui/QMenu>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QAction>
#include <QtCore/QTimer>
#include <QtCore/QDebug>

#include <KStandardShortcut>

ChatTextEdit::ChatTextEdit(QWidget *parent) :
        KTextEdit(parent)
{
    setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);    // no need for horizontal scrollbar with this
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setCheckSpellingEnabled(true);
    enableFindReplace(false);
    setMinimumHeight(0);

    // set to false so it doesn't paste anything unwanted apart from normal text
    setAcceptRichText(false);

    connect(this, SIGNAL(textChanged()), SLOT(recalculateSize()));
}

void ChatTextEdit::setFontBold(bool isBold)
{
    if (isBold) {
        setFontWeight(QFont::Bold);
    } else {
        setFontWeight(QFont::Normal);
    }
}

//Size code based on LineEdit in PSI
//Justin Karneges, Michail Pishchagin

QSize ChatTextEdit::minimumSizeHint() const
{
    QSize sh = KTextEdit::minimumSizeHint();
    sh.setHeight(fontMetrics().height() + 1);
    sh += QSize(0, QFrame::lineWidth() * 2);
    return sh;
}

QSize ChatTextEdit::sizeHint() const
{
    QSize sh = KTextEdit::sizeHint();
    sh.setHeight(int (document()->size().height()));
    sh += QSize(0, (QFrame::lineWidth() * 2) + 1);
    return sh;
}

void ChatTextEdit::keyPressEvent(QKeyEvent *e)
{
    if ((e->key()==Qt::Key_Return ||  e->key()==Qt::Key_Enter) && !e->modifiers()) {
        Q_EMIT returnKeyPressed();
        return;
    }

    if (e->matches(QKeySequence::Copy)) {
        if (!textCursor().hasSelection()) {
            QWidget::keyReleaseEvent(e); //skip internal trapping, and pass to parent.
            return;
        }
    }

    if (e->key() == Qt::Key_PageUp ||
        e->key() == Qt::Key_PageDown) {
        QWidget::keyPressEvent(e); //pass to parent.
        return;
    }

    if (e->key() == Qt::Key_Tab && e->modifiers() & Qt::ControlModifier) {
        QWidget::keyPressEvent(e);
        return;
    }

    KTextEdit::keyPressEvent(e);
}

bool ChatTextEdit::event(QEvent *e)
{
    if (e->type() == QEvent::ShortcutOverride) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(e);
        const int key = keyEvent->key() | keyEvent->modifiers();
        if (KStandardShortcut::find().contains(key)) {
            return false; //never catch "find" sequence.
        }
        if (KStandardShortcut::copy().contains(key)) {
            if (!textCursor().hasSelection()) {
                return false; //don't catch "copy" sequence if there is no selected text.
            }
        }
    }
    return KTextEdit::event(e);
}

void ChatTextEdit::resizeEvent(QResizeEvent *e)
{
    KTextEdit::resizeEvent(e);
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
