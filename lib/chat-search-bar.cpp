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

#include "chat-search-bar.h"

#include <KAction>
#include <KColorScheme>
#include <KLineEdit>
#include <KLocale>
#include <KPushButton>

#include <QCheckBox>
#include <QDebug>
#include <QHBoxLayout>

ChatSearchBar::ChatSearchBar(QWidget* parent)
    : QWidget(parent)
    , m_searchInput(new KLineEdit(this))
    , m_closeButton(new KPushButton(this))
    , m_nextButton(new KPushButton(KIcon("arrow-down"), i18n("&Next"), this))
    , m_previousButton(new KPushButton(KIcon("arrow-up"), i18n("&Previous"), this))
    , m_caseSensitive(false)
{
    // close button setup
    m_closeButton->setIcon(KIcon("dialog-close"));
    connect(m_closeButton, SIGNAL(clicked(bool)), this, SLOT(toggleView(bool)));

    // search line setup
    m_searchInput->setPlaceholderText(i18n("Insert search text..."));

    // search arrows
    connect(m_nextButton, SIGNAL(clicked()), this, SLOT(onNextButtonClicked()));
    connect(m_previousButton, SIGNAL(clicked()), this, SLOT(onPreviousButtonClicked()));

    // options for search criteria
    QCheckBox *caseSensitiveAction = new QCheckBox(i18n("Case sensitive"), this);

    connect(caseSensitiveAction, SIGNAL(clicked(bool)), this, SLOT(toggleCaseSensitive(bool)));

    // text changed signal
    connect(m_searchInput, SIGNAL(textChanged(QString)), this, SLOT(textChanged(QString)));

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(m_closeButton);
    layout->addWidget(m_searchInput);
    layout->addWidget(m_nextButton);
    layout->addWidget(m_previousButton);
    layout->addWidget(caseSensitiveAction);

    setLayout(layout);

    // start hidden
    hide();
}

ChatSearchBar::~ChatSearchBar()
{

}

QWebPage::FindFlags ChatSearchBar::findFlags()
{
    QWebPage::FindFlags flags;
    flags |= QWebPage::FindWrapsAroundDocument;

    if(m_caseSensitive) {
        flags |= QWebPage::FindCaseSensitively;
    }
    return flags;
}

KLineEdit* ChatSearchBar::searchBar() const
{
    return m_searchInput;
}

void ChatSearchBar::onNextButtonClicked()
{
    emit(findNextSignal(m_searchInput->text(), findFlags()));
}

void ChatSearchBar::onPreviousButtonClicked()
{
    emit(findPreviousSignal(m_searchInput->text(), findFlags()));
}

void ChatSearchBar::onSearchTextComplete(bool found)
{
    if(found || m_searchInput->text().isEmpty()) {
        KColorScheme scheme(QPalette::Active, KColorScheme::View);
        QColor background = scheme.background(KColorScheme::NormalBackground).color();

        if(m_searchInput->palette().color(QPalette::Base) != background) {
            QPalette p = m_searchInput->palette();
            p.setColor(QPalette::Base, background);
            m_searchInput->setPalette(p);
        }
    } else {
        KColorScheme scheme(QPalette::Active, KColorScheme::Window);
        QColor background = scheme.foreground(KColorScheme::ActiveText).color();

        // check for empty text as well. It's not to be considered as "text not found"
        if(m_searchInput->palette().color(QPalette::Base) != background && !m_searchInput->text().isEmpty()) {
            QPalette p = m_searchInput->palette();
            p.setColor(QPalette::Base, background);
            m_searchInput->setPalette(p);
        }
    }
}

void ChatSearchBar::toggleView(bool toggle)
{
    if(!toggle) {
        m_searchInput->clear();
        hide();
    } else {
        show();
        m_searchInput->setFocus();
    }
}

void ChatSearchBar::textChanged(const QString& text)
{
    emit(findTextSignal(text, findFlags()));
}

void ChatSearchBar::toggleCaseSensitive(bool toggle)
{
    m_caseSensitive = toggle;
    emit(flagsChangedSignal(m_searchInput->text(), findFlags()));
}




