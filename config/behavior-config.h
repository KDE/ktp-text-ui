/***************************************************************************
 *   Copyright (C) 2011 by Lasath Fernando <kde@lasath.org>
 *   Copyright (C) 2011 by David Edmundson <kde@lasath.org>
 *
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


#ifndef BEHAVIOR_CONFIG_H
#define BEHAVIOR_CONFIG_H

#include <KCModule>
#include "../app/telepathy-chat-ui.h"

namespace Ui {
class BehaviorConfigUi;
}

class BehaviorConfig : public KCModule
{
    Q_OBJECT

public:
    explicit BehaviorConfig(QWidget *parent = 0, const QVariantList &args = QVariantList());
    virtual ~BehaviorConfig();

public slots:
    virtual void load();
    virtual void save();

protected:
    virtual void changeEvent(QEvent *e);

private slots:
    void onRadioSelected(int id);

private:
    int m_openMode;
    Ui::BehaviorConfigUi* ui;
};

#endif // BEHAVIOR_CONFIG_H
