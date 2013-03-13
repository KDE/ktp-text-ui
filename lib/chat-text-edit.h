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

#include <QtCore/QList>
#include <QtGui/QKeySequence>

#include <KTextEdit>
#include <KAction>

class ChannelContactModel;
class ChatTextEdit : public KTextEdit
{
    Q_OBJECT
public:
    explicit ChatTextEdit(QWidget *parent = 0);

    void setContactModel(ChannelContactModel *model);

    // reimplemented
    QSize minimumSizeHint() const;
    QSize sizeHint() const;

protected:
    void keyPressEvent(QKeyEvent *e);

    bool event(QEvent *);

    // reimplemented
    void resizeEvent(QResizeEvent*);

    void getHistory(bool up);
    void addHistory(const QString &text);

    void completeNick();

private Q_SLOTS:
    void recalculateSize();
    void updateScrollBar();

Q_SIGNALS:
    void returnKeyPressed();

public Q_SLOTS:
    /** wraps setFontWeight to a simple on/off bold) */
    void setFontBold(bool);
    void sendMessage(); // Sends message entered (<= Return key pressing)

    /**
     * Updates internal message sending shortcuts. Must be called on every window
     * creation and every message sending shortcuts change.
     */
    void setSendMessageShortcuts(const KShortcut &shortcuts);

private:
    QStringList m_history;
    int m_historyPos;

    /* Nick completion */
    ChannelContactModel *m_contactModel;
    QString m_lastCompletion;
    int m_oldCursorPos;
    int m_completionPosition;
    bool m_continuousCompletion;

    KShortcut m_sendMessageShortcuts;
};

#endif // CHATTEXTEDIT_H


