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

#include "appearance-config.h"
#include "ui_appearance-config.h"
#include "appearance-config-tab.h"

#include <KTabWidget>

#include "chat-window-style-manager.h"
#include "chat-window-style.h"
#include "adium-theme-header-info.h"
#include "adium-theme-content-info.h"
#include "adium-theme-status-info.h"

#include <KDebug>
#include <KDialog>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KIcon>
#include <KSharedConfig>

K_PLUGIN_FACTORY(KCMTelepathyChatAppearanceConfigFactory, registerPlugin<AppearanceConfig>();)
K_EXPORT_PLUGIN(KCMTelepathyChatAppearanceConfigFactory("ktp_chat_appearance", "kcm_ktp_chat_appearance"))

AppearanceConfig::AppearanceConfig(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
{
    QVBoxLayout *topLayout = new QVBoxLayout(this);
    topLayout->setMargin(0);
    topLayout->setSpacing(KDialog::spacingHint());

    KTabWidget *tabWidget = new KTabWidget(this);

    m_singleTab = new AppearanceConfigTab(this, AppearanceConfigTab::NormalChat);
    tabWidget->addTab(m_singleTab, QIcon(), i18nc("@title:tab", "Normal Chat"));
    connect(m_singleTab, SIGNAL(tabChanged()), this, SLOT(changed()));

    m_groupTab = new AppearanceConfigTab(this, AppearanceConfigTab::GroupChat);
    tabWidget->addTab(m_groupTab, QIcon(), i18nc("@title:tab", "Group Chat"));
    connect(m_groupTab, SIGNAL(tabChanged()), this, SLOT(changed()));

    topLayout->addWidget(tabWidget, 0, 0);
}

AppearanceConfig::~AppearanceConfig()
{

}

void AppearanceConfig::defaults()
{
    m_singleTab->defaultTab();
    m_groupTab->defaultTab();
}

void AppearanceConfig::load()
{
    m_singleTab->loadTab();
    m_groupTab->loadTab();
}


void AppearanceConfig::save()
{
    kDebug();

    KSharedConfigPtr config = KSharedConfig::openConfig(QLatin1String("ktelepathyrc"));

    m_singleTab->saveTab(config->group("Appearance"));
    m_groupTab->saveTab(config->group("GroupAppearance"));

    config->sync();

    KCModule::save();

    Q_EMIT reloadTheme();
}

#include "appearance-config.moc"
