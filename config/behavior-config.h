/***************************************************************************
 *   Copyright (C) 2011 by Lasath Fernando <kde@lasath.org>
 *   Copyright (C) 2011 by David Edmundson <kde@davidedmundson.co.uk>
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
#include "text-chat-config.h"

namespace Ui {
class BehaviorConfigUi;
}

class BehaviorConfig : public KCModule
{
    Q_OBJECT

public:
    explicit BehaviorConfig(QWidget *parent = 0, const QVariantList &args = QVariantList());
    virtual ~BehaviorConfig();

public Q_SLOTS:
    virtual void load();
    virtual void save();

protected:
    virtual void changeEvent(QEvent *e);

private Q_SLOTS:
    void onRadioSelected(int id);
    void onScrollbackLengthChanged();
    void onShowMeTypingChanged(bool state);
    void onShowOthersTypingChanged(bool state);
    void onNicknameCompletionStyleChanged(int index);

private:
    TextChatConfig::TabOpenMode m_openMode;
    int m_scrollbackLength;
    bool m_showMeTyping; // show others I am typing
    bool m_showOthersTyping; // show me others are typing
    QString m_nicknameCompletionSuffix;
    Ui::BehaviorConfigUi *ui;

    static const QStringList nicknameCompletionSuffixes;
};

#endif // BEHAVIOR_CONFIG_H
