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
#include <KLocalizedString>

#include "shareprovider.h"

K_PLUGIN_FACTORY(KCMTelepathyChatBehaviorConfigFactory, registerPlugin<BehaviorConfig>();)
K_EXPORT_PLUGIN(KCMTelepathyChatBehaviorConfigFactory("ktp_chat_behavior", "kcm_ktp_chat_behavior"))

BehaviorConfig::BehaviorConfig(QWidget *parent, const QVariantList& args)
    : KCModule(parent, args),
      ui(new Ui::BehaviorConfigUi())
{
    kDebug();

    load();

    ui->setupUi(this);

    ui->newTabButtonGroup->setId(ui->radioNew, TextChatConfig::NewWindow);
    ui->newTabButtonGroup->setId(ui->radioZero, TextChatConfig::FirstWindow);

    ui->newTabButtonGroup->button(m_openMode)->setChecked(true);
    connect(ui->newTabButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onRadioSelected(int)));

    ui->scrollbackLength->setSuffix(ki18ncp("Part of config 'show last [spin box] messages' This is the suffix to the spin box. Be sure to include leading space"," message", " messages"));
    ui->scrollbackLength->setValue(m_scrollbackLength);
    connect(ui->scrollbackLength, SIGNAL(valueChanged(int)), this, SLOT(onScrollbackLengthChanged()));

    ui->checkBoxShowMeTyping->setChecked(m_showMeTyping);
    connect(ui->checkBoxShowMeTyping, SIGNAL(toggled(bool)), this, SLOT(onShowMeTypingChanged(bool)));

    ui->checkBoxShowOthersTyping->setChecked(m_showOthersTyping);
    connect(ui->checkBoxShowOthersTyping, SIGNAL(toggled(bool)), this, SLOT(onShowOthersTypingChanged(bool)));

    ui->dontLeaveGroupChats->setChecked(m_dontLeaveGroupChats);
    connect(ui->dontLeaveGroupChats, SIGNAL(toggled(bool)), this, SLOT(onDontLeaveGroupChatsChanged(bool)));

    QStringList nicknameCompletionStyles;
    const QString namePlaceholder = ki18nc("Placeholder for contact name in completion suffix selector", "Nickname").toString();
    Q_FOREACH(const QString &suffix, BehaviorConfig::nicknameCompletionSuffixes) {
        nicknameCompletionStyles.append(namePlaceholder + suffix);
    }
    ui->nicknameCompletionStyle->addItems(nicknameCompletionStyles);
    ui->nicknameCompletionStyle->setCurrentIndex(BehaviorConfig::nicknameCompletionSuffixes.indexOf(m_nicknameCompletionSuffix));
    connect(ui->nicknameCompletionStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(onNicknameCompletionStyleChanged(int)));

    QMap<QString, ShareProvider::ShareService> availableShareServices = ShareProvider::availableShareServices();
    QStringList sharingServicesList = availableShareServices.keys();
    ui->imageSharingService->addItems(sharingServicesList);
    QString shareServiceName = availableShareServices.key(static_cast<ShareProvider::ShareService>(m_imageShareServiceType));
    ui->imageSharingService->setCurrentIndex(sharingServicesList.indexOf(shareServiceName));
    connect(ui->imageSharingService, SIGNAL(currentIndexChanged(int)), this, SLOT(onImageSharingServiceChanged(int)));
}

const QStringList BehaviorConfig::nicknameCompletionSuffixes = QStringList()
        << QLatin1String(", ")
        << QLatin1String(": ")
        << QLatin1String(" ");

BehaviorConfig::~BehaviorConfig()
{
    delete ui;
}

void BehaviorConfig::load()
{
    m_openMode = TextChatConfig::instance()->openMode();
    m_scrollbackLength = TextChatConfig::instance()->scrollbackLength();
    m_showMeTyping = TextChatConfig::instance()->showMeTyping();
    m_showOthersTyping = TextChatConfig::instance()->showOthersTyping();
    m_nicknameCompletionSuffix = TextChatConfig::instance()->nicknameCompletionSuffix();
    m_imageShareServiceType = TextChatConfig::instance()->imageShareServiceType();
    m_dontLeaveGroupChats = TextChatConfig::instance()->dontLeaveGroupChats();
}

void BehaviorConfig::save()
{
    TextChatConfig::instance()->setOpenMode(m_openMode);
    TextChatConfig::instance()->setScrollbackLength(m_scrollbackLength);
    TextChatConfig::instance()->setShowMeTyping(m_showMeTyping);
    TextChatConfig::instance()->setShowOthersTyping(m_showOthersTyping);
    TextChatConfig::instance()->setNicknameCompletionSuffix(m_nicknameCompletionSuffix);
    TextChatConfig::instance()->setImageShareServiceName(m_imageShareServiceType);
    TextChatConfig::instance()->setDontLeaveGroupChats(m_dontLeaveGroupChats);
    TextChatConfig::instance()->sync();
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
    m_openMode = (TextChatConfig::TabOpenMode) id;
    kDebug() << "emitting changed(true)";
    Q_EMIT changed(true);
}

void BehaviorConfig::onScrollbackLengthChanged()
{
    m_scrollbackLength = ui->scrollbackLength->value();
    Q_EMIT changed(true);
}

void BehaviorConfig::onShowMeTypingChanged(bool state)
{
    m_showMeTyping = state;
    Q_EMIT changed(true);
}

void BehaviorConfig::onShowOthersTypingChanged(bool state)
{
    m_showOthersTyping = state;
    Q_EMIT changed(true);
}


void BehaviorConfig::onNicknameCompletionStyleChanged(int index)
{
    m_nicknameCompletionSuffix = BehaviorConfig::nicknameCompletionSuffixes[index];
    Q_EMIT changed(true);
}

void BehaviorConfig::onImageSharingServiceChanged(int index)
{
    Q_UNUSED(index);
    QString imageShareServiceName = ui->imageSharingService->currentText();
    kDebug() << imageShareServiceName;
    m_imageShareServiceType = ShareProvider::availableShareServices()[imageShareServiceName];
    Q_EMIT changed(true);
}

void BehaviorConfig::onDontLeaveGroupChatsChanged(bool state)
{
    m_dontLeaveGroupChats = state;
    Q_EMIT changed(true);
}

#include "behavior-config.moc"
