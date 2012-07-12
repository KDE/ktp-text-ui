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

#include <QVBoxLayout>

#include <KPluginInfo>
#include <KPluginFactory>
#include <KPluginSelector>
#include <KDebug>

K_PLUGIN_FACTORY(KTpMessagesConfigFactory, registerPlugin<MessagesConfig>();)
K_EXPORT_PLUGIN(KTpMessagesConfigFactory("kcm_ktp_message_filters", "kcm_ktp_chat_messages"))

class MessagesConfig::Private {
public:
    KPluginSelector *selector;
};

MessagesConfig::MessagesConfig(QWidget *parent, const QVariantList &args)
    : KCModule(KTpMessagesConfigFactory::componentData(), parent, args),
        d(new Private)
{
    d->selector = new KPluginSelector();

    QLayout *layout = new QVBoxLayout(this);
    layout->addWidget(d->selector);
    setLayout(layout);

    KPluginInfo::List plugins = MessageProcessor::pluginList();
    d->selector->addPlugins(plugins);

    connect(d->selector, SIGNAL(changed(bool)), SIGNAL(changed(bool)));
}

void MessagesConfig::save()
{
    kDebug();
    d->selector->save();
    KCModule::save();
}

void MessagesConfig::defaults()
{
    d->selector->defaults();
    KCModule::defaults();
}