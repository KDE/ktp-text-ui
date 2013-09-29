/***************************************************************************
 *   Copyright (C) 2013 by Daniel Cohen <analoguecolour@gmail.com>        *
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

#include "KTpTextChatPart.h"
#include "chat-widget.h"

#include <kdemacros.h>
#include <kparts/genericfactory.h>
#include <QVBoxLayout>
#include <KTp/types.h>
#include <TelepathyQt/Account>
#include <TelepathyQt/TextChannel>

#include <KApplication>
#include <KAction>
#include <KLocale>
#include <KActionCollection>
#include <KStandardAction>

K_PLUGIN_FACTORY(KTpTextChatPartFactory, registerPlugin<KTpTextChatPart>();)  // produce a factory
K_EXPORT_PLUGIN(KTpTextChatPartFactory("KTpTextChatPart", "ktp-text-ui") )

KTpTextChatPart::KTpTextChatPart( QWidget *parentWidget, QObject *parent, const QVariantList& args )
    : KParts::Part(parent)
{
    KGlobal::locale()->insertCatalog(QLatin1String("textUI"));
    // we need an instance
    setComponentData( KTpTextChatPartFactory::componentData() );
    Tp::AccountPtr account = args[0].value<Tp::AccountPtr>();
    Tp::TextChannelPtr textChannel = args[1].value<Tp::TextChannelPtr>();
    tabWidget = new ChatTabWidget(textChannel, account, 0);
    setWidget(tabWidget);
}

KTpTextChatPart::~KTpTextChatPart()
{
}

#include "KTpTextChatPart.moc"