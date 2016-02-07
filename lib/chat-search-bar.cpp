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

#include <KColorScheme>
#include <KLocalizedString>

#include <QPushButton>
#include <QLineEdit>
#include <QAction>
#include <QCheckBox>
#include <QDebug>
#include <QHBoxLayout>
#include <QToolButton>

#include <QKeyEvent>

ChatSearchBar::ChatSearchBar(QWidget *parent)
    : QWidget(parent)
    , m_searchInput(new QLineEdit(this))
    , m_closeButton(new QToolButton(this))
    , m_nextButton(new QPushButton(QIcon::fromTheme(QStringLiteral("go-down-search")), i18nc("Next search result" ,"&Next"), this))
    , m_previousButton(new QPushButton(QIcon::fromTheme(QStringLiteral("go-up-search")), i18nc("Previous search result" ,"&Previous"), this))
    , m_caseSensitive(false)
{
    // close button setup
    m_closeButton->setAutoRaise(true);
    m_closeButton->setIcon(QIcon::fromTheme(QStringLiteral("dialog-close")));
    connect(m_closeButton, SIGNAL(clicked(bool)), this, SLOT(toggleView(bool)));

    // search line setup
    m_searchInput->setPlaceholderText(i18n("Insert search text..."));

    // search arrows, start disabled
    enableSearchButtons(false);

    connect(m_nextButton, SIGNAL(clicked()), this, SLOT(onNextButtonClicked()));
    connect(m_previousButton, SIGNAL(clicked()), this, SLOT(onPreviousButtonClicked()));

    // options for search criteria
    QCheckBox *caseSensitiveAction = new QCheckBox(i18n("Case sensitive"), this);

    connect(caseSensitiveAction, SIGNAL(clicked(bool)), this, SLOT(toggleCaseSensitive(bool)));

    // text changed signal
    connect(m_searchInput, SIGNAL(textChanged(QString)), this, SLOT(textChanged(QString)));

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(2, 0, 2, 0);
    layout->addWidget(m_closeButton);
    layout->setAlignment(m_closeButton, Qt::AlignLeft | Qt::AlignTop);

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

void ChatSearchBar::enableSearchButtons(bool enable)
{
    m_nextButton->setEnabled(enable);
    m_previousButton->setEnabled(enable);
    Q_EMIT enableSearchButtonsSignal(enable);
}

QWebEnginePage::FindFlags ChatSearchBar::findFlags()
{
    QWebEnginePage::FindFlags flags;

    if(m_caseSensitive) {
        flags |= QWebEnginePage::FindCaseSensitively;
    }
    return flags;
}

QLineEdit *ChatSearchBar::searchBar() const
{
    return m_searchInput;
}

void ChatSearchBar::onNextButtonClicked()
{
    // no need to call this if search bar is hidden
    if(isVisible()) {
        Q_EMIT findNextSignal(m_searchInput->text(), findFlags());
    }
}

void ChatSearchBar::onPreviousButtonClicked()
{
    // no need to call this if search bar is hidden
    if(isVisible()) {
        Q_EMIT findPreviousSignal(m_searchInput->text(), findFlags());
    }
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

bool ChatSearchBar::event(QEvent *e)
{
    if (e->type() == QEvent::ShortcutOverride && static_cast<QKeyEvent*>(e)->key() == Qt::Key_Escape) {
        if (isVisible()) {
            setVisible(false);
            e->accept();
            return true;
        }
    }
    return QWidget::event(e);
}

void ChatSearchBar::textChanged(const QString& text)
{
    // enable/disable next and previous buttons
    if (!m_searchInput->text().isEmpty()) {
        enableSearchButtons(true);
    } else {
        enableSearchButtons(false);
    }
    Q_EMIT findTextSignal(text, findFlags());
}

void ChatSearchBar::toggleCaseSensitive(bool toggle)
{
    m_caseSensitive = toggle;
    Q_EMIT flagsChangedSignal(m_searchInput->text(), findFlags());
}
