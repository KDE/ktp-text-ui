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
#include "channel-contact-model.h"

#include <QtGui/QMenu>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QAction>
#include <QtCore/QTimer>
#include <QtCore/QDebug>
#include <QtCore/QString>

#include <KStandardShortcut>
#include <KActionCollection>

#define MAXHISTORY 100

ChatTextEdit::ChatTextEdit(QWidget *parent) :
        KTextEdit(parent),
        m_contactModel(0),
        m_oldCursorPos(0),
        m_completionPosition(0),
        m_continuousCompletion(false)
{
    setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);    // no need for horizontal scrollbar with this
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setCheckSpellingEnabled(true);
    enableFindReplace(false);
    setMinimumHeight(0);

    // set to false so it doesn't paste anything unwanted apart from normal text
    setAcceptRichText(false);

    // Initialize the history
    m_history.prepend(QString());
    m_historyPos = 0;

    connect(this, SIGNAL(textChanged()), SLOT(recalculateSize()));
}

void ChatTextEdit::setContactModel(ChannelContactModel* model)
{
    m_contactModel = model;
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
    if (e->matches(QKeySequence::Copy)) {
        if (!textCursor().hasSelection()) {
            QWidget::keyReleaseEvent(e); //skip internal trapping, and pass to parent.
            return;
        }
    }

    if ((e->key() == Qt::Key_Up) && !textCursor().movePosition(QTextCursor::Up)) {
        getHistory(true);
    }

    if ((e->key() == Qt::Key_Down) && !textCursor().movePosition(QTextCursor::Down)) {
        getHistory(false);
    }

    if (e->key() == Qt::Key_PageUp ||
        e->key() == Qt::Key_PageDown) {
        QWidget::keyPressEvent(e); //pass to parent.
        return;
    }

    if (e->key() == Qt::Key_Tab) {
        if (e->modifiers() & Qt::ControlModifier) {
            QWidget::keyPressEvent(e);
        } else if (e->modifiers() == 0) {
            completeNick();
        }
        return;
    }

    if(!e->text().isEmpty() || ((e->key() >= Qt::Key_Home) && (e->key() <= Qt::Key_Down))) {
        m_continuousCompletion = false;
    }

    KTextEdit::keyPressEvent(e);
}

bool ChatTextEdit::event(QEvent *e)
{
    if (e->type() == QEvent::ShortcutOverride) {
        // Extract key code for shortcut sequence comparison
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(e);
        int key = keyEvent->key();
        if (keyEvent->modifiers() != Qt::KeypadModifier) {
            // Keypad modifier is not used in KDE shortcuts setup, so, we need to skip it.
            key |= keyEvent->modifiers();
        }

        if (m_sendMessageShortcuts.contains(key)) {
            // keyPressEvent() handles Control modifier wrong, so we need that thing
            // to be in event().
            this->sendMessage();
            e->setAccepted(true);
            return false;
        }
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

void ChatTextEdit::sendMessage()
{
    if (!toPlainText().isEmpty()) {
        addHistory(toPlainText());
    }
    m_continuousCompletion = false;

    Q_EMIT returnKeyPressed();
}

void ChatTextEdit::setSendMessageShortcuts(const KShortcut &shortcuts)
{
    m_sendMessageShortcuts = KShortcut(shortcuts);
}

// History of sent messages based on code from Konversation
// by Dario Abatianni

void ChatTextEdit::getHistory(bool up)
{
    m_history[m_historyPos] = toPlainText();

    if (up) {
        m_historyPos++;

        if (m_historyPos == m_history.length()) {
            m_historyPos--;
            return;
        }
    } else {
        if (m_historyPos == 0) {
            if (!toPlainText().isEmpty()) {
                addHistory(toPlainText());
            }

            setText(QLatin1String(""));
        } else {
            m_historyPos--;
        }
    }

    setText(m_history[m_historyPos]);
}

void ChatTextEdit::addHistory(const QString &text)
{
    if (m_history.value(1) != text) {
        m_history[0] = text;
        m_history.prepend(QString());

        if (m_history.length() > MAXHISTORY) {
            m_history.removeLast();
        }
    }

    m_historyPos = 0;
}

// Nick completion - based on code from Konversation by Eike Hein
void ChatTextEdit::completeNick()
{
    if (!m_contactModel) {
        return;
    }

    int pos, oldPos;
    QTextCursor cursor = textCursor();
    bool continousCompletion = m_continuousCompletion;

    pos = cursor.position();
    oldPos = m_oldCursorPos;

    QString line = toPlainText();
    QString newLine;

    // Check if completion position is out of range
    if (m_completionPosition >= m_contactModel->rowCount()) {
        m_completionPosition = 0;
    }

    if (continousCompletion) {
        line.remove(oldPos, pos - oldPos);
        pos = oldPos;
    }

    // If the cursor is at beginning of line, insert last completion if the nick is still around
    if (pos == 0 && !m_lastCompletion.isEmpty() && m_contactModel->containsNick(m_lastCompletion)) {
        newLine = m_lastCompletion;
        // New cursor position is behind nickname
        pos = newLine.length();
        // Add rest of the line
        newLine += line;
    } else {
        // remember old cursor position in input field
        m_oldCursorPos = pos;
        // remember old cursor position locally
        oldPos = pos;
        // step back to last space or start of line
        while (pos && line[pos - 1] != QLatin1Char(' ')) {
            pos--;
        }

        // copy search pattern (lowercase)
        QString pattern = line.mid(pos, oldPos - pos);
        // copy line to newLine-buffer
        newLine = line;

        // did we find any pattern?
        if (!pattern.isEmpty()) {
            bool complete = false;
            QString foundNick;

            if (m_contactModel->rowCount() > 0) {
                if (!continousCompletion) {
                    int listPosition = 0;

                    for (int i = 0; i < m_contactModel->rowCount(); ++i) {
                        QModelIndex index = m_contactModel->index(i, 0);
                        if (index.data().toString().startsWith(pattern, Qt::CaseInsensitive)) {
                            m_completionPosition = listPosition;

                            ++listPosition;
                        }
                    }
                }

                // remember old nick completion position
                int oldCompletionPosition = m_completionPosition;
                complete = true;

                do {
                    QModelIndex index = m_contactModel->index(m_completionPosition, 0);
                    QString lookNick = index.data(Qt::DisplayRole).toString();
                    if (lookNick.startsWith(pattern, Qt::CaseInsensitive)) {
                        foundNick = lookNick;
                    }

                    // increment search position
                    m_completionPosition++;

                    // wrap around
                    if(m_completionPosition == m_contactModel->rowCount()) {
                        m_completionPosition = 0;
                    }

                // the search ends when we either find a suitable nick or we end up at the
                // first search position
                } while ((m_completionPosition != oldCompletionPosition) && foundNick.isEmpty());
            }

            // did we find a suitable nick?
            if (!foundNick.isEmpty()) {
                m_continuousCompletion = true;

                // remove pattern from line
                newLine.remove(pos, pattern.length());

                // did we find the nick in the middle of the line?
                if ((pos > 0) && complete) {
                    m_lastCompletion = foundNick;
                    newLine.insert(pos, foundNick);
                    pos = pos + foundNick.length();
                } else if ((pos == 0) && complete) {
                    // no, it was at the beginning
                    m_lastCompletion = foundNick;
                    newLine.insert(pos, foundNick + QLatin1String(", "));
                    pos = pos + foundNick.length() + 2; /* 2 = strlen(", ") */
                } else {
                    // the nick wasn't complete
                    newLine.insert(pos, foundNick);
                    pos = pos + foundNick.length();
                }
            } else {
                // no pattern found, so restore old cursor position
                pos = oldPos;
            }
        }
    }

    // Set new text and cursor position
    setText(newLine);
    cursor.setPosition(pos);
    setTextCursor(cursor);
}
