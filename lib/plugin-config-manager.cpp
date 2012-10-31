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

#include "plugin-config-manager.h"
#include "version.h"

#include <QMutex>
#include <QSet>

#include <KGlobal>
#include <KDebug>
#include <KServiceTypeTrader>

typedef QSet<KPluginInfo> PluginSet;

class PluginConfigManager::Private {
public:
    PluginSet all;
    PluginSet enabled;
};

PluginConfigManager *PluginConfigManager::self()
{
    static PluginConfigManager *pcm_instance;
    static QMutex mutex;
    mutex.lock();
    if (!pcm_instance) {
        pcm_instance = new PluginConfigManager;
    }
    mutex.unlock();

    return pcm_instance;
}

PluginConfigManager::PluginConfigManager() :
    d(new Private)
{
    generateCache();
}

KService::List offers() {
    return KServiceTypeTrader::self()->query(QLatin1String("KTpTextUi/MessageFilter"),
                                             QLatin1String("[X-KTp-PluginInfo-Version] == " KTP_TEXT_UI_PLUGIN_FRAMEWORK_VERSION));
}

void PluginConfigManager::generateCache()
{
    KPluginInfo::List all = KPluginInfo::fromServices(offers(), configGroup());
    for (KPluginInfo::List::Iterator i = all.begin(); i != all.end(); i++) {
        KPluginInfo &plugin = *i;

        d->all.insert(plugin);

        plugin.load();
        if (plugin.isPluginEnabled()) {
            d->enabled.insert(plugin);
        }
    }
}

KPluginInfo::List PluginConfigManager::allPlugins() const
{
    return d->all.toList();
}

KPluginInfo::List PluginConfigManager::enabledPlugins() const
{
    return d->enabled.toList();
}

KConfigGroup PluginConfigManager::configGroup() const
{
    return sharedConfig()->group("Plugins");
}

KSharedConfig::Ptr PluginConfigManager::sharedConfig() const
{
    return KSharedConfig::openConfig(QLatin1String("ktelepathyrc"));
}
