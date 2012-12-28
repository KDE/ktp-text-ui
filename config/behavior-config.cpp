/***************************************************************************
 *   Copyright (C) 2011 by Lasath Fernando <kde@lasath.org>                *
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

#include "behavior-config.h"
#include "ui_behavior-config.h"

#include <KDebug>
#include <KPluginFactory>

K_PLUGIN_FACTORY(KCMTelepathyChatBehaviorConfigFactory, registerPlugin<BehaviorConfig>();)
K_EXPORT_PLUGIN(KCMTelepathyChatBehaviorConfigFactory("ktp_chat_behavior", "kcm_ktp_chat_behavior"))


BehaviorConfig::BehaviorConfig(QWidget *parent, const QVariantList& args)
    : KCModule(KCMTelepathyChatBehaviorConfigFactory::componentData(), parent, args),
      ui(new Ui::BehaviorConfigUi())
{
    kDebug();

    load();

    ui->setupUi(this);

    ui->newTabButtonGroup->setId(ui->radioLast, TelepathyChatUi::LastWindow);
    ui->newTabButtonGroup->setId(ui->radioNew, TelepathyChatUi::NewWindow);
    ui->newTabButtonGroup->setId(ui->radioZero, TelepathyChatUi::FirstWindow);

    ui->newTabButtonGroup->button(m_openMode)->setChecked(true);
    connect(ui->newTabButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onRadioSelected(int)));

    ui->scrollbackLength->setValue(m_scrollbackLength);
    connect(ui->scrollbackLength, SIGNAL(valueChanged(int)), this, SLOT(onScrollbackLengthChanged()));
}

BehaviorConfig::~BehaviorConfig()
{
    delete ui;
}

void BehaviorConfig::load()
{
    KSharedConfigPtr config = KSharedConfig::openConfig(QLatin1String("ktelepathyrc"));
    KConfigGroup tabConfig = config->group("Behavior");

    QString mode = tabConfig.readEntry("tabOpenMode", "NewWindow");
    if(mode == QLatin1String("NewWindow")) {
        m_openMode = TelepathyChatUi::NewWindow;
    } else if (mode == QLatin1String("FirstWindow")) {
        m_openMode = TelepathyChatUi::FirstWindow;
    } else if (mode == QLatin1String("LastWindow")) {
        m_openMode = TelepathyChatUi::LastWindow;
    }

    m_scrollbackLength = tabConfig.readEntry("scrollbackLength", 4);
}

void BehaviorConfig::save()
{
    KSharedConfigPtr config = KSharedConfig::openConfig(QLatin1String("ktelepathyrc"));
    KConfigGroup tabConfig = config->group("Behavior");

    QString mode;
    switch (m_openMode) {
        case TelepathyChatUi::NewWindow :
            mode = QLatin1String("NewWindow");
            break;
        case TelepathyChatUi::FirstWindow :
            mode = QLatin1String("FirstWindow");
            break;
        case TelepathyChatUi::LastWindow :
            mode = QLatin1String("LastWindow");
            break;
    }

    tabConfig.writeEntry("tabOpenMode", mode);
    tabConfig.writeEntry("scrollbackLength", m_scrollbackLength);
    tabConfig.sync();
}

void BehaviorConfig::changeEvent(QEvent* e)
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

void BehaviorConfig::onRadioSelected(int id)
{
    kDebug() << "BehaviorConfig::m_openMode changed from " << id << " to " << m_openMode;
    m_openMode = (TelepathyChatUi::TabOpenMode) id;
    kDebug() << "emitting changed(true)";
    Q_EMIT changed(true);
}

void BehaviorConfig::onScrollbackLengthChanged()
{
    m_scrollbackLength = ui->scrollbackLength->value();
    Q_EMIT changed(true);
}
