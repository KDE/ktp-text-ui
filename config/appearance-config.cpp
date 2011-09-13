/***************************************************************************
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

#include "appearance-config.h"
#include "ui_appearance-config.h"

#include <KDETelepathy/ChatWindowStyleManager>
#include <KDETelepathy/ChatWindowStyle>
#include <KDETelepathy/AdiumThemeHeaderInfo>
#include <KDETelepathy/AdiumThemeContentInfo>
#include <KDETelepathy/AdiumThemeStatusInfo>

#include <KDebug>
#include <KLocalizedString>
#include <KPluginFactory>

K_PLUGIN_FACTORY(KCMTelepathyChatAppearanceConfigFactory, registerPlugin<AppearanceConfig>();)
K_EXPORT_PLUGIN(KCMTelepathyChatAppearanceConfigFactory("telepathy_chat_appearance_config", "kcm_telepathy_chat_appearance_config"))

AppearanceConfig::AppearanceConfig(QWidget *parent, const QVariantList& args)
    : KCModule(KCMTelepathyChatAppearanceConfigFactory::componentData(), parent, args),
      ui(new Ui::ChatWindowConfig)
{
    ui->setupUi(this);

    ChatWindowStyleManager *manager = ChatWindowStyleManager::self();
    manager->loadStyles();
    connect(manager, SIGNAL(loadStylesFinished()), SLOT(onStylesLoaded()));

    m_demoChatHeader.setChatName(i18n("A demo chat"));
    m_demoChatHeader.setSourceName(i18n("Jabber"));
    m_demoChatHeader.setTimeOpened(QDateTime::currentDateTime());
    m_demoChatHeader.setDestinationName(i18n("BobMarley@yahoo.com"));
    m_demoChatHeader.setDestinationDisplayName(i18n("Bob Marley"));

    ui->chatView->initialise(m_demoChatHeader);

    ui->showHeader->setChecked(ui->chatView->isHeaderDisplayed());
    ui->customFontBox->setChecked(ui->chatView->isCustomFont());
    ui->fontFamily->setCurrentFont(QFont(ui->chatView->fontFamily()));
    ui->fontSize->setValue(ui->chatView->fontSize());

    connect(ui->chatView, SIGNAL(loadFinished(bool)), SLOT(sendDemoMessages()));
    connect(ui->styleComboBox, SIGNAL(activated(int)), SLOT(onStyleSelected(int)));
    connect(ui->variantComboBox, SIGNAL(activated(QString)), SLOT(onVariantSelected(QString)));
    connect(ui->showHeader, SIGNAL(clicked(bool)), SLOT(onShowHeaderChanged(bool)));
    connect(ui->customFontBox, SIGNAL(clicked(bool)), SLOT(onFontGroupChanged(bool)));
    connect(ui->fontFamily, SIGNAL(currentFontChanged(QFont)), SLOT(onFontFamilyChanged(QFont)));
    connect(ui->fontSize, SIGNAL(valueChanged(int)), SLOT(onFontSizeChanged(int)));
}

AppearanceConfig::~AppearanceConfig()
{
    delete ui;
}

void AppearanceConfig::changeEvent(QEvent *e)
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

void AppearanceConfig::onStylesLoaded()
{
    kDebug();

    QMap<QString, QString> styles = ChatWindowStyleManager::self()->getAvailableStyles();
    ChatWindowStyle *currentStyle = ui->chatView->chatStyle();

    ui->styleComboBox->clear();
    QMap<QString, QString>::const_iterator i = styles.constBegin();
    while (i != styles.constEnd()) {
        ui->styleComboBox->addItem(i.value(), i.key());

        if (i.key() == currentStyle->id()) {
            ui->styleComboBox->setCurrentItem(i.value());
        }

        ++i;
    }

    //ui->styleComboBox->setCurrentItem(currentStyle->getStyleName());

    updateVariantsList();
    //FIXME call onStyleSelected
}

void AppearanceConfig::updateVariantsList()
{
    kDebug();
    QHash<QString, QString> variants = ui->chatView->chatStyle()->getVariants();
    ui->variantComboBox->clear();
    ui->variantComboBox->addItems(variants.keys());
    ui->variantComboBox->setCurrentItem(ui->chatView->variantName());
}

void AppearanceConfig::onStyleSelected(int index)
{
    kDebug();
    //load the style.
    QString styleId = ui->styleComboBox->itemData(index).toString();

    ChatWindowStyle *style = ChatWindowStyleManager::self()->getValidStyleFromPool(styleId);

    if (style) {
        ui->chatView->setChatStyle(style);
        updateVariantsList();
        ui->showHeader->setEnabled(style->hasHeader());
        ui->chatView->initialise(m_demoChatHeader);
    }
    changed();
}

void AppearanceConfig::onVariantSelected(const QString &variant)
{
    kDebug();
    ui->chatView->setVariant(variant);
    ui->chatView->initialise(m_demoChatHeader);
    changed();
}

void AppearanceConfig::onShowHeaderChanged(bool showHeader)
{
    ui->chatView->setHeaderDisplayed(showHeader);
    ui->chatView->initialise(m_demoChatHeader);
    changed();
}

void AppearanceConfig::onFontGroupChanged(bool useCustomFont)
{
    kDebug();
    ui->chatView->setUseCustomFont(useCustomFont);
    ui->chatView->initialise(m_demoChatHeader);
    changed();
}

void AppearanceConfig::onFontFamilyChanged(QFont fontFamily)
{
    kDebug() << fontFamily.family();
    ui->chatView->setFontFamily(fontFamily.family());
    ui->chatView->initialise(m_demoChatHeader);
    changed();
}

void AppearanceConfig::onFontSizeChanged(int fontSize)
{
    kDebug() << fontSize;
    ui->chatView->setFontSize(fontSize);
    ui->chatView->initialise(m_demoChatHeader);
    changed();
}

void AppearanceConfig::sendDemoMessages()
{
    //add a fake message

    AdiumThemeContentInfo message(AdiumThemeMessageInfo::RemoteToLocal);
    message.setMessage(i18n("Hello"));
    message.setSenderDisplayName(i18n("larry@example.com"));
    message.setSenderScreenName(i18n("Larry Demo"));
    message.setService(i18n("Jabber"));
    message.setTime(QDateTime::currentDateTime());
    ui->chatView->addContentMessage(message);

    message = AdiumThemeContentInfo(AdiumThemeMessageInfo::LocalToRemote);
    message.setMessage(i18n("A different example message"));
    message.setSenderDisplayName(i18n("ted@example.com"));
    message.setSenderScreenName(i18n("Ted Example"));
    message.setService(i18n("Jabber"));
    message.setTime(QDateTime::currentDateTime());
    ui->chatView->addContentMessage(message);

    AdiumThemeStatusInfo  statusMessage;
    statusMessage.setMessage(i18n("Ted Example has left the chat.")); //FIXME sync this with chat text logic.
    statusMessage.setTime(QDateTime::currentDateTime());
    statusMessage.setService(i18n("Jabber"));
    statusMessage.setStatus("away");
    ui->chatView->addStatusMessage(statusMessage);
}

void AppearanceConfig::save()
{
    kDebug();

    KSharedConfigPtr config = KSharedConfig::openConfig("ktelepathyrc");
    KConfigGroup appearanceConfig = config->group("Appearance");

    appearanceConfig.writeEntry("styleName", ui->styleComboBox->itemData(ui->styleComboBox->currentIndex()).toString());
    appearanceConfig.writeEntry("styleVariant", ui->variantComboBox->currentText());
    appearanceConfig.writeEntry("displayHeader", ui->showHeader->isChecked());
    appearanceConfig.writeEntry("useCustomFont", ui->customFontBox->isChecked());
    appearanceConfig.writeEntry("fontFamily", ui->fontFamily->currentFont().family());
    appearanceConfig.writeEntry("fontSize", ui->fontSize->value());

    appearanceConfig.sync();
    config->sync();

    KCModule::save();
}
