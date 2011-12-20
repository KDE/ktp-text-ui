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

#ifndef APPEARANCE_CONFIG_H
#define APPEARANCE_CONFIG_H

#include "adium-theme-header-info.h"

#include <KCModule>

namespace Ui
{
  class ChatWindowConfig;
}


class AppearanceConfig : public KCModule
{
    Q_OBJECT

public:
    explicit AppearanceConfig(QWidget *parent = 0,
				const QVariantList& args = QVariantList());
    ~AppearanceConfig();

public Q_SLOTS:
    void save();

protected:
    void changeEvent(QEvent *e);

private Q_SLOTS:
    void sendDemoMessages();
    void onStylesLoaded();
    void updateVariantsList();

    void onStyleSelected(int index);
    void onVariantSelected(const QString&);
    void onShowHeaderChanged(bool);
    void onFontGroupChanged(bool);
    void onFontFamilyChanged(QFont);
    void onFontSizeChanged(int);

private:
    Ui::ChatWindowConfig *ui;
    AdiumThemeHeaderInfo m_demoChatHeader;

};

#endif // APPEARANCE_CONFIG_H
