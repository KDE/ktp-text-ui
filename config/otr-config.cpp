/***************************************************************************
 *   Copyright (C) 2014 by Marcin Ziemiński <zieminn@gmail.com>            *
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

#include "otr-config.h"
#include "ui_otr-config.h"
#include "ktp_otr.h"

#include <KDebug>
#include <KPluginFactory>
#include <KLocalizedString>
#include <QtGui/QtEvents>

K_PLUGIN_FACTORY(KCMTelepathyChatOtrConfigFactory, registerPlugin<OTRConfig>();)
K_EXPORT_PLUGIN(KCMTelepathyChatOtrConfigFactory("ktp_chat_otr", "kcm_ktp_chat_otr"))

OTRConfig::OTRConfig(QWidget *parent, const QVariantList& args)
    : KCModule(KCMTelepathyChatOtrConfigFactory::componentData(), parent, args),
      ui(new Ui::OTRConfigUi())
{
    kDebug();

    ui->setupUi(this);

	addConfig(KtpOtrKcfg::self(), this);
	KtpOtrKcfg::self()->readConfig();
}

OTRConfig::~OTRConfig() 
{
    delete ui;
}


void OTRConfig::changeEvent(QEvent* e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

