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
#include <KLocalizedString>

#include <QTextToSpeech>

class TTSFilter::Private {
public:
    QTextToSpeech *speech;
};

TTSFilter::TTSFilter(QObject *parent, const QVariantList &)
    : KTp::AbstractMessageFilter(parent),
      d(new Private)
{
    d->speech = new QTextToSpeech();
}

TTSFilter::~TTSFilter()
{
    delete d->speech;
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

    if (message.type() == Tp::ChannelTextMessageTypeNormal) {
        d->speech->say(i18nc("Text to Speech - text message %1 is name, %2 is message", "%1 says %2", message.mainMessagePart(), message.senderAlias()));
    }
    else if (message.type() == Tp::ChannelTextMessageTypeAction) {
        d->speech->say(i18nc("Text to Speech - text message %1 is name, %2 is message", "%1 %2", message.mainMessagePart(), message.senderAlias()));
    }
}

K_PLUGIN_FACTORY(MessageFilterFactory, registerPlugin<TTSFilter>();)

#include "tts-filter.moc"
