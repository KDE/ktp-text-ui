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

#ifndef PLUGIN_CONFIG_MANAGER_H
#define PLUGIN_CONFIG_MANAGER_H

#include <KPluginInfo>
#include <ktpchat_export.h>

class KDE_TELEPATHY_CHAT_EXPORT PluginConfigManager
{
public:
    static PluginConfigManager* self();

    KPluginInfo::List allPlugins() const;
    KPluginInfo::List enabledPlugins() const;

    KConfigGroup       configGroup() const;
    KSharedConfig::Ptr sharedConfig() const;

protected:
    PluginConfigManager();

private:
    class Private;
    Private *d;

    void generateCache();
};

#endif // PLUGIN_CONFIG_MANAGER_H
