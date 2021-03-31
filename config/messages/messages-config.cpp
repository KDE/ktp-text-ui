/*
 *    Copyright (C) 2012  Lasath Fernando <kde@lasath.org>
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 2.1 of the License, or (at your option) any later version.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "messages-config.h"
#include <KTp/message-filter-config-manager.h>

#include <QVBoxLayout>

#include <KPluginFactory>
#include <KPluginSelector>
#include <KLocalizedString>
#include <KAboutData>

K_PLUGIN_FACTORY(KTpMessagesConfigFactory, registerPlugin<MessagesConfig>();)

MessagesConfig::MessagesConfig(QWidget* parent, const QVariantList& args)
    : KCModule(parent, args)
    , m_pluginSelector(new KPluginSelector(this))
{
    m_pluginSelector->addPlugins(
        KTp::MessageFilterConfigManager::self()->allPlugins(),
        KPluginSelector::ReadConfigFile,
        i18n("Plugins"),
        QString(),
        KTp::MessageFilterConfigManager::self()->sharedConfig() //why won't this take a KConfigGroup?
    );

    connect(m_pluginSelector, &KPluginSelector::changed, this, &MessagesConfig::markAsChanged);

    QLayout *layout = new QVBoxLayout();
    layout->addWidget(m_pluginSelector);
    setLayout(layout);
}

void MessagesConfig::save()
{
    m_pluginSelector->save();
    KTp::MessageFilterConfigManager::self()->reloadConfig();
}

void MessagesConfig::defaults()
{
    m_pluginSelector->defaults();
}

void MessagesConfig::load()
{
    m_pluginSelector->load();
}

#include "messages-config.moc"
