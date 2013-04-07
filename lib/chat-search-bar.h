/***************************************************************************
 *   Copyright (C) 2011 by Francesco Nwokeka <francesco.nwokeka@gmail.com> *
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

#ifndef CHATSEARCHBAR_H
#define CHATSEARCHBAR_H

#include "ktpchat_export.h"

#include <QtWebKit/QWebPage>
#include <QtGui/QWidget>

class KLineEdit;
class KPushButton;
class QToolButton;

/**
 * @brief Input field for user to insert text to look for inside a chat window
 * @author Francesco Nwokeka <francesco.nwokeka@gmail.com>
 */

class KDE_TELEPATHY_CHAT_EXPORT ChatSearchBar : public QWidget
{
    Q_OBJECT
public:
    ChatSearchBar(QWidget *parent = 0);
    virtual ~ChatSearchBar();

    KLineEdit* searchBar() const;

public Q_SLOTS:
    void onNextButtonClicked();
    void onPreviousButtonClicked();

    void onSearchTextComplete(bool found);

    /** toggle search bar visibility */
    void toggleView(bool toggle);

protected:
    bool event(QEvent *e);

private Q_SLOTS:
    /** called when user writes in search bar
     * this emits a signal for chat-window with the text to search for
     * and the appropriate flags for the search criteria
     */
    void textChanged(const QString &text);

    /** search criteria toggle: case sensitivity */
    void toggleCaseSensitive(bool toggle);

Q_SIGNALS:
    void findTextSignal(const QString &text, QWebPage::FindFlags flags);
    void findNextSignal(const QString &text, QWebPage::FindFlags flags);
    void findPreviousSignal(const QString &text, QWebPage::FindFlags flags);

    /** emitted when search criteria is changed by user and updates current view */
    void flagsChangedSignal(const QString &, QWebPage::FindFlags flags);

    /** send signal to mainwindow to enable/disable search buttons */
    void enableSearchButtonsSignal(bool enable);

private:
    /** enable/disable next and previous buttons for search */
    void enableSearchButtons(bool enable);

    /** returns selected search criteria chosen by user */
    QWebPage::FindFlags findFlags();

    KLineEdit *m_searchInput;
    QToolButton *m_closeButton;
    KPushButton *m_nextButton;
    KPushButton *m_previousButton;

    // search criteria variables
    bool m_highlightAll
    , m_caseSensitive;
};

#endif  // CHATSEARCHBAR_H
