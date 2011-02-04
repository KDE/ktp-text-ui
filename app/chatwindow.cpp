/*
    Copyright (C) 2010  David Edmundson <kde@davidedmundson.co.uk>
    Copyright (C) 2011  Dominik Schmidt <dev@dominik-schmidt.de>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "chatwindow.h"
#include "chatwidget.h"

#include <KStandardAction>
#include <KIcon>
#include <KLocale>
#include <KApplication>
#include <KAction>
#include <KActionCollection>
#include <KDebug>
#include <KIcon>
#include <KColorScheme>
#include <KTabBar>
#include <KSettings/Dialog>

ChatWindow::ChatWindow()
{
    //setup actions
    KStandardAction::quit(KApplication::instance(), SLOT(quit()), actionCollection());
    KStandardAction::preferences(this, SLOT(showSettingsDialog()), actionCollection());


    // set up m_tabWidget
    m_tabWidget = new KTabWidget(this);
    m_tabWidget->setTabReorderingEnabled(true);
    m_tabWidget->setDocumentMode(true);
    m_tabWidget->setCloseButtonEnabled(true);
    m_tabWidget->setHoverCloseButtonDelayed(true);
    m_tabWidget->setTabBarHidden(true);
    connect(m_tabWidget, SIGNAL(closeRequest(QWidget*)), m_tabWidget, SLOT(removePage(QWidget*)));
    connect(m_tabWidget, SIGNAL(currentChanged(int)), this, SLOT(onCurrentIndexChanged(int)));
    connect(qobject_cast<KTabBar*>(m_tabWidget->tabBar()), SIGNAL(mouseMiddleClick(int)),
                m_tabWidget, SLOT(removeTab(int)));

    setCentralWidget(m_tabWidget);

    setupGUI(static_cast<StandardWindowOptions>(Default^StatusBar), "chatwindow.rc");
}

ChatWindow::~ChatWindow()
{
}

void ChatWindow::addTab(ChatWidget* chatWidget)
{
    connect(chatWidget, SIGNAL(titleChanged(QString)), this, SLOT(updateTabText(QString)));
    connect(chatWidget, SIGNAL(iconChanged(KIcon)), this, SLOT(updateTabIcon(KIcon)));
    connect(chatWidget, SIGNAL(userTypingChanged(bool)), this, SLOT(onUserTypingChanged(bool)));

    m_tabWidget->addTab(chatWidget, chatWidget->icon(), chatWidget->title());
    m_tabWidget->setCurrentWidget(chatWidget);

    if(m_tabWidget->isTabBarHidden()) {
        if(m_tabWidget->count() > 1) {
            m_tabWidget->setTabBarHidden(false);
        }
    }

    activateWindow();
}

void ChatWindow::removeTab(ChatWidget* chatWidget)
{
    m_tabWidget->removeTab(m_tabWidget->indexOf(chatWidget));
}

void ChatWindow::updateTabText(const QString & newTitle)
{
    //find out which widget made the call, and update the correct tab.
    QWidget* sender = qobject_cast<QWidget*>(QObject::sender());
    if (sender) {
        int tabIndexToChange = m_tabWidget->indexOf(sender);
        m_tabWidget->setTabText(tabIndexToChange, newTitle);

        if (tabIndexToChange == m_tabWidget->currentIndex()) {
            onCurrentIndexChanged(tabIndexToChange);
        }
    }
}

void ChatWindow::updateTabIcon(const KIcon & newIcon)
{
    //find out which widget made the call, and update the correct tab.
    QWidget* sender = qobject_cast<QWidget*>(QObject::sender());
    if (sender) {
        int tabIndexToChange = m_tabWidget->indexOf(sender);
        m_tabWidget->setTabIcon(tabIndexToChange, newIcon);
    }
}


void ChatWindow::onCurrentIndexChanged(int index)
{
    kDebug() << index;

    if(index == -1) {
        close();
        return;
    }

    ChatWidget* currentChatWidget = qobject_cast<ChatWidget*>(m_tabWidget->widget(index));
    setWindowTitle(currentChatWidget->title());
    setWindowIcon(currentChatWidget->icon());
}

void ChatWindow::onUserTypingChanged(bool isTyping)
{
    QWidget* sender = qobject_cast<QWidget*>(QObject::sender());
    if (sender) {
        KColorScheme scheme(QPalette::Active, KColorScheme::Window);
        int tabIndex = m_tabWidget->indexOf(sender);
        if (isTyping) {
            m_tabWidget->setTabTextColor(tabIndex, scheme.foreground(KColorScheme::PositiveText).color());
        } else {
            m_tabWidget->setTabTextColor(tabIndex, scheme.foreground(KColorScheme::NormalText).color());
        }
    }
}

void ChatWindow::showSettingsDialog()
{
    kDebug();

    KSettings::Dialog *dialog = new KSettings::Dialog(this);

    dialog->addModule("kcm_telepathy_chat");
    dialog->addModule("kcm_telepathy_accounts");

    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

#include "chatwindow.moc"