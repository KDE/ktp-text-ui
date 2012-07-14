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
#include "message-processor.h"
#include <plugin-config-manager.h>

#include <QVBoxLayout>

#include <KPluginInfo>
#include <KPluginFactory>
#include <KPluginSelector>
#include <KDebug>

K_PLUGIN_FACTORY(KTpMessagesConfigFactory, registerPlugin<MessagesConfig>();)
K_EXPORT_PLUGIN(KTpMessagesConfigFactory("kcm_ktp_message_filters", "kcm_ktp_chat_messages"))

MessagesConfig::MessagesConfig(QWidget *parent, const QVariantList &args)
    : PluginPage(KTpMessagesConfigFactory::componentData(), parent, args)
{
    pluginSelector()->addPlugins(
        PluginConfigManager::self()->allPlugins(),
        KPluginSelector::ReadConfigFile,
        QString(),
        QString(),
        PluginConfigManager::self()->sharedConfig() //why won't this take a KConfigGroup?
    );

    //Am surprised that PluginPage() doesn't do this for me
    QLayout *layout = new QVBoxLayout();
    layout->addWidget(pluginSelector());
    setLayout(layout);
}