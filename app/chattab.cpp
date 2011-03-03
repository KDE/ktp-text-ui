/***************************************************************************
 *   Copyright (C) 2011 by Dominik Schmidt <kde@dominik-schmidt.de>        *
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

#include "chattab.h"
#include "chatwidget.h"

#include <KTabWidget>
#include <KDebug>
#include <QStackedWidget>
#include <KDE/KColorScheme>

ChatTab::ChatTab(const Tp::TextChannelPtr & channel, QWidget *parent)
    : ChatWidget(channel, parent)
{
}

ChatTab::~ChatTab()
{
}

void ChatTab::setTabWidget(KTabWidget* tabWidget)
{
    m_tabWidget = tabWidget;
}

KTabWidget* ChatTab::tabWidget() const
{
    return m_tabWidget;
}

void ChatTab::showOnTop()
{
    kDebug() << "Show this widget on top" << title();
    if(m_tabWidget) {
        m_tabWidget->setCurrentWidget(this);
    }

    activateWindow();
}
