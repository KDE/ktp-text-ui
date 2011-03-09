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

#ifndef CHATTAB_H
#define CHATTAB_H

#include "chat-widget.h"

#include <QtCore/QString>
#include <QtGui/QWidget>
#include <KIcon>
#include <KColorScheme>


class KTabWidget;
class ChatWidgetPrivate;
class QShowEvent;

class ChatTab : public ChatWidget
{
    Q_OBJECT

public:
    explicit ChatTab(const Tp::TextChannelPtr & channel, QWidget *parent = 0);
    virtual ~ChatTab();

    void setTabWidget(KTabWidget *tabWidget);
    KTabWidget* tabWidget() const;

public slots:
    void showOnTop();

private:
    KTabWidget *m_tabWidget;
};

#endif // CHATWIDGET_H
