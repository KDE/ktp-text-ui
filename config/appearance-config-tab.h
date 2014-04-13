/***************************************************************************
 *   Copyright (C) 2013 by Huba Nagy <12huba@gmail.com>                    *
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

#ifndef APPEARANCE_CONFIG_TAB_H
#define APPEARANCE_CONFIG_TAB_H

#include "adium-theme-header-info.h"

#include <QWidget>
#include <KConfigGroup>

namespace Ui
{
  class ChatWindowConfig;
}

class AppearanceConfigTab : public QWidget
{
    Q_OBJECT

public:
    enum TabMode {
        NormalChat,
        GroupChat
    };

    explicit AppearanceConfigTab(QWidget *parent = 0, TabMode mode = NormalChat);

    virtual ~AppearanceConfigTab();

    void saveTab(KConfigGroup appearanceConfigGroup);
    void loadTab();
    void defaultTab();

Q_SIGNALS:
    void tabChanged();

protected:
    void changeEvent(QEvent *e);

private Q_SLOTS:
    void sendDemoMessages();
    void onStylesLoaded();
    void updateVariantsList();

    void onStyleSelected(int index);
    void onVariantSelected(const QString &variant);
    void onShowHeaderChanged(bool showHead);
    void onFontGroupChanged(bool useCustomFont);
    void onFontFamilyChanged(const QFont &font);
    void onFontSizeChanged(int fontSize);
    void onShowPresenceChangesChanged(bool stateChanged);
    void onShowLeaveChangesChanged(bool leaveChanged);

private:
    Ui::ChatWindowConfig *ui;
    AdiumThemeHeaderInfo m_demoChatHeader;
    bool m_groupChat;
};

#endif // APPEARANCE_CONFIG_TAB_H
