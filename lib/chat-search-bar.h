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

#include <QWebPage>
#include <QWidget>

class KLineEdit;
class KPushButton;

/**
 * @brief Input field for user to insert text to look for inside a chat window
 * @author Francesco Nwokeka <francesco.nwokeka@gmail.com>
 */

class ChatSearchBar : public QWidget
{
    Q_OBJECT
public:
    ChatSearchBar(QWidget *parent = 0);
    virtual ~ChatSearchBar();

    KLineEdit *searchBar() const;

public slots:
    void onNextButtonClicked();
    void onPreviousButtonClicked();

    void onSearchTextComplete(bool found);

    /** toggle search bar visibility */
    void toggleView(bool toggle);

private slots:
    /** called when user writes in search bar
     * this emits a signal for chat-window with the text to search for
     * and the appropriate flags for the search criteria
     */
    void textChanged(const QString &text);

    /** search criteria toggle: case sensitivity */
    void toggleCaseSensitive(bool toggle);

signals:
    void findTextSignal(const QString &text, QWebPage::FindFlags flags);
    void findNextSignal(const QString &text, QWebPage::FindFlags flags);
    void findPreviousSignal(const QString &text, QWebPage::FindFlags flags);

    /** emitted when search criteria is changed by user and updates current view */
    void flagsChangedSignal(const QString &, QWebPage::FindFlags flags);

private:
    /** returns selected search criteria chosen by user */
    QWebPage::FindFlags findFlags();

    KLineEdit *m_searchInput;
    KPushButton *m_closeButton
    , *m_nextButton
    , *m_previousButton;

    // search criteria variables
    bool m_highlightAll
    , m_caseSensitive;
};

#endif  // CHATSEARCHBAR_H