/*
 *    Copyright (C) 2012  David Edmundson <davidedmundson@kde.org>
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

#include "tts-filter.h"


#include <KPluginFactory>
#include <KDebug>
#include <KLocalizedString>

#include <kspeech.h>
#include <kspeechinterface.h>


class TTSFilter::Private {
public:
    org::kde::KSpeech *kspeech;
};

static const KCatalogLoader loader(QLatin1String("ktp-filters"));

TTSFilter::TTSFilter(QObject *parent, const QVariantList &)
    : KTp::AbstractMessageFilter(parent),
      d(new Private)
{
    d->kspeech = new org::kde::KSpeech(QLatin1String("org.kde.kttsd"), QLatin1String("/KSpeech"), QDBusConnection::sessionBus());
    d->kspeech->setApplicationName(i18n("KDE Instant Messaging"));
}

TTSFilter::~TTSFilter()
{
    delete d->kspeech;
    delete d;
}

void TTSFilter::filterMessage(KTp::Message &message, const KTp::MessageContext &context)
{
    Q_UNUSED (context)
    if (message.direction() == KTp::Message::LocalToRemote) {
        return;
    }

    if (message.isHistory()) {
        return;
    }

    //FIXME with real name.
    d->kspeech->say(i18n("New message. %1", message.mainMessagePart()), KSpeech::soHtml);
}

K_PLUGIN_FACTORY(MessageFilterFactory, registerPlugin<TTSFilter>();)
K_EXPORT_PLUGIN(MessageFilterFactory("ktptextui_message_filter_tts"))
