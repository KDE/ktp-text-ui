/***************************************************************************
 *   Copyright (C) 2013 by Huba Nagy <12huba@gmail.com>                    *
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

#include "appearance-config-tab.h"
#include "ui_appearance-config.h"

#include "chat-window-style-manager.h"
#include "chat-window-style.h"
#include "adium-theme-header-info.h"
#include "adium-theme-content-info.h"
#include "adium-theme-status-info.h"

#include <KMessageBox>

AppearanceConfigTab::AppearanceConfigTab(QWidget *parent, TabMode mode)
    : QWidget(parent),
    ui(new Ui::ChatWindowConfig)
{
    m_groupChat = mode == GroupChat;

    ui->setupUi(this);

    m_demoChatHeader.setChatName(i18n("A demo chat"));
    m_demoChatHeader.setSourceName(i18n("Jabber"));
    m_demoChatHeader.setTimeOpened(QDateTime::currentDateTime());
    m_demoChatHeader.setDestinationName(i18nc("Example email", "ted@example.com"));
    m_demoChatHeader.setDestinationDisplayName(i18nc("Example name", "Ted"));
    m_demoChatHeader.setGroupChat(m_groupChat);
    m_demoChatHeader.setService(QLatin1String("jabber"));
     // check iconPath docs for minus sign in -KIconLoader::SizeMedium
    m_demoChatHeader.setServiceIconPath(KIconLoader::global()->iconPath(QLatin1String("im-jabber"), -KIconLoader::SizeMedium));

    ChatWindowStyleManager *manager = ChatWindowStyleManager::self();
    connect(manager, SIGNAL(loadStylesFinished()), SLOT(onStylesLoaded()));

    //loading theme settings.
    loadTab();

    connect(ui->chatView, SIGNAL(viewReady()), SLOT(sendDemoMessages()));
    connect(ui->styleComboBox, SIGNAL(activated(int)), SLOT(onStyleSelected(int)));
    connect(ui->variantComboBox, SIGNAL(activated(QString)), SLOT(onVariantSelected(QString)));
    connect(ui->showHeader, SIGNAL(clicked(bool)), SLOT(onShowHeaderChanged(bool)));
    connect(ui->customFontBox, SIGNAL(clicked(bool)), SLOT(onFontGroupChanged(bool)));
    connect(ui->fontFamily, SIGNAL(currentFontChanged(QFont)), SLOT(onFontFamilyChanged(QFont)));
    connect(ui->fontSize, SIGNAL(valueChanged(int)), SLOT(onFontSizeChanged(int)));
    connect(ui->showPresenceCheckBox, SIGNAL(toggled(bool)), SLOT(onShowPresenceChangesChanged(bool)));
    connect(ui->showLeaveCheckBox, SIGNAL(toggled(bool)), SLOT(onShowLeaveChangesChanged(bool)));
}

AppearanceConfigTab::~AppearanceConfigTab()
{
    delete ui;
}

void AppearanceConfigTab::changeEvent(QEvent* e)
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

void AppearanceConfigTab::onFontFamilyChanged(const QFont &fontFamily)
{
    ui->chatView->setFontFamily(fontFamily.family());
    ui->chatView->initialise(m_demoChatHeader);
    tabChanged();
}

void AppearanceConfigTab::onFontGroupChanged(bool useCustomFont)
{
    ui->chatView->setUseCustomFont(useCustomFont);
    ui->chatView->initialise(m_demoChatHeader);
    tabChanged();
}

void AppearanceConfigTab::onFontSizeChanged(int fontSize)
{
    ui->chatView->setFontSize(fontSize);
    ui->chatView->initialise(m_demoChatHeader);
    tabChanged();
}

void AppearanceConfigTab::onShowPresenceChangesChanged(bool stateChanged)
{
    ui->chatView->setShowPresenceChanges(stateChanged);
    ui->chatView->initialise(m_demoChatHeader);
    tabChanged();
}

void AppearanceConfigTab::onShowLeaveChangesChanged(bool leaveChanged)
{
    ui->chatView->setShowLeaveChanges(leaveChanged);
    ui->chatView->initialise(m_demoChatHeader);
    tabChanged();
}

void AppearanceConfigTab::onShowHeaderChanged(bool showHeader)
{
    ui->chatView->setHeaderDisplayed(showHeader);
    ui->chatView->initialise(m_demoChatHeader);
    tabChanged();
}

void AppearanceConfigTab::onStyleSelected(int index)
{
    //load the style.
    QString styleId = ui->styleComboBox->itemData(index).toString();

    ChatWindowStyle *style = ChatWindowStyleManager::self()->getValidStyleFromPool(styleId);

    if (style) {
        ui->chatView->setChatStyle(style);
        updateVariantsList();
        ui->showHeader->setEnabled(style->hasHeader());
        ui->chatView->initialise(m_demoChatHeader);
    }

    tabChanged();
}

void AppearanceConfigTab::onStylesLoaded()
{
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

void AppearanceConfigTab::onVariantSelected(const QString &variant)
{
    ui->chatView->setVariant(variant);
    ui->chatView->initialise(m_demoChatHeader);
    tabChanged();
}

void AppearanceConfigTab::sendDemoMessages()
{
    //add a fake message
    AdiumThemeContentInfo message(AdiumThemeMessageInfo::HistoryRemoteToLocal);
    message.setMessage(i18nc("Example message in preview conversation","Ok!"));
    message.setSenderDisplayName(i18nc("Example email", "larry@example.com"));
    message.setSenderScreenName(i18nc("Example name", "Larry Demo"));
    message.setTime(QDateTime::currentDateTime());
    ui->chatView->addAdiumContentMessage(message);

    message = AdiumThemeContentInfo(AdiumThemeMessageInfo::HistoryRemoteToLocal);
    message.setMessage(i18nc("Example message in preview conversation","Bye Bye"));
    message.setSenderDisplayName(i18nc("Example email", "larry@example.com"));
    message.setSenderScreenName(i18nc("Example name", "Larry Demo"));
    message.setTime(QDateTime::currentDateTime());
    ui->chatView->addAdiumContentMessage(message);

    message = AdiumThemeContentInfo(AdiumThemeMessageInfo::HistoryLocalToRemote);
    message.setMessage(i18nc("Example message in preview conversation","Have fun!"));
    message.setSenderDisplayName(i18nc("Example email", "ted@example.com"));
    message.setSenderScreenName(i18nc("Example name", "Ted Example"));
    message.setTime(QDateTime::currentDateTime());
    ui->chatView->addAdiumContentMessage(message);

    message = AdiumThemeContentInfo(AdiumThemeMessageInfo::HistoryLocalToRemote);
    message.setMessage(i18nc("Example message in preview conversation","cya"));
    message.setSenderDisplayName(i18nc("Example email", "ted@example.com"));
    message.setSenderScreenName(i18nc("Example name", "Ted Example"));
    message.setTime(QDateTime::currentDateTime());
    ui->chatView->addAdiumContentMessage(message);

    AdiumThemeStatusInfo statusMessage(true);
    statusMessage.setMessage(i18nc("Example message", "Ted Example waves."));
    statusMessage.setSender(i18nc("Example name", "Ted Example"));
    statusMessage.setTime(QDateTime::currentDateTime());
    ui->chatView->addAdiumStatusMessage(statusMessage);

    if (ui->chatView->showLeaveChanges()) {
        statusMessage = AdiumThemeStatusInfo(true);
        statusMessage.setMessage(i18nc("Example message in preview conversation","Ted Example has left the chat.")); //FIXME sync this with chat text logic.
        statusMessage.setSender(i18nc("Example name", "Ted Example"));
        statusMessage.setTime(QDateTime::currentDateTime());
        statusMessage.setStatus(QLatin1String("away"));
        ui->chatView->addAdiumStatusMessage(statusMessage);
    }

    message = AdiumThemeContentInfo(AdiumThemeMessageInfo::RemoteToLocal);
    message.setMessage(i18nc("Example message in preview conversation","Hello Ted"));
    message.setSenderDisplayName(i18nc("Example email", "larry@example.com"));
    message.setSenderScreenName(i18nc("Example name", "Larry Demo"));
    message.appendMessageClass(QLatin1String("mention"));
    message.setTime(QDateTime::currentDateTime());
    ui->chatView->addAdiumContentMessage(message);

    message = AdiumThemeContentInfo(AdiumThemeMessageInfo::RemoteToLocal);
    message.setMessage(i18nc("Example message in preview conversation","What's up?"));
    message.setSenderDisplayName(i18nc("Example email", "larry@example.com"));
    message.setSenderScreenName(i18nc("Example name", "Larry Demo"));
    message.setTime(QDateTime::currentDateTime());
    ui->chatView->addAdiumContentMessage(message);

    message = AdiumThemeContentInfo(AdiumThemeMessageInfo::LocalToRemote);
    message.setMessage(i18nc("Example message in preview conversation","Check out which cool adium themes work "
                            "<a href=\"http://community.kde.org/KTp/Components/Chat_Window/Themes\">"
                            "here</a>!"));
    message.setSenderDisplayName(i18nc("Example email", "ted@example.com"));
    message.setSenderScreenName(i18nc("Example name", "Ted Example"));
    message.setTime(QDateTime::currentDateTime());
    ui->chatView->addAdiumContentMessage(message);

    if ( m_groupChat == true) {
        message = AdiumThemeContentInfo(AdiumThemeMessageInfo::RemoteToLocal);
        message.setMessage(i18nc("Example message in preview conversation","Hello"));
        message.setSenderDisplayName(i18nc("Example email", "bob@example.com"));
        message.setSenderScreenName(i18nc("Example name", "Bob Example"));
        message.setTime(QDateTime::currentDateTime());
        ui->chatView->addAdiumContentMessage(message);
    }

    message = AdiumThemeContentInfo(AdiumThemeMessageInfo::LocalToRemote);
    message.setMessage(i18nc("Example message in preview conversation","A different example message"));
    message.setSenderDisplayName(i18nc("Example email", "ted@example.com"));
    message.setSenderScreenName(i18nc("Example name", "Ted Example"));
    message.setTime(QDateTime::currentDateTime());
    ui->chatView->addAdiumContentMessage(message);

    if (ui->chatView->showPresenceChanges()) {
        statusMessage = AdiumThemeStatusInfo();
        statusMessage.setMessage(i18nc("Example message in preview conversation","Ted Example is now Away.")); //FIXME sync this with chat text logic.
        statusMessage.setSender(i18nc("Example name", "Ted Example"));
        statusMessage.setTime(QDateTime::currentDateTime());
        statusMessage.setStatus(QLatin1String("away"));
        ui->chatView->addAdiumStatusMessage(statusMessage);

        statusMessage = AdiumThemeStatusInfo();
        statusMessage.setMessage(i18nc("Example message in preview conversations","Ted Example has left the chat.")); //FIXME sync this with chat text logic.
        statusMessage.setSender(i18nc("Example name", "Ted Example"));
        statusMessage.setTime(QDateTime::currentDateTime());
        statusMessage.setStatus(QLatin1String("away"));
        ui->chatView->addAdiumStatusMessage(statusMessage);
    }
}

void AppearanceConfigTab::updateVariantsList()
{
    QHash<QString, QString> variants = ui->chatView->chatStyle()->getVariants();
    ui->variantComboBox->clear();
    ui->variantComboBox->addItems(variants.keys());
    ui->variantComboBox->setCurrentItem(ui->chatView->variantName());
}

void AppearanceConfigTab::saveTab(KConfigGroup appearanceConfigGroup)
{
    appearanceConfigGroup.writeEntry(QLatin1String("styleName"), ui->styleComboBox->itemData(ui->styleComboBox->currentIndex()).toString());
    appearanceConfigGroup.writeEntry(QLatin1String("styleVariant"), ui->variantComboBox->currentText());
    appearanceConfigGroup.writeEntry(QLatin1String("displayHeader"), ui->showHeader->isChecked());
    appearanceConfigGroup.writeEntry(QLatin1String("useCustomFont"), ui->customFontBox->isChecked());
    appearanceConfigGroup.writeEntry(QLatin1String("fontFamily"), ui->fontFamily->currentFont().family());
    appearanceConfigGroup.writeEntry(QLatin1String("fontSize"), ui->fontSize->value());
    appearanceConfigGroup.writeEntry(QLatin1String("showPresenceChanges"), ui->showPresenceCheckBox->isChecked());
    appearanceConfigGroup.writeEntry(QLatin1String("showLeaveChanges"), ui->showLeaveCheckBox->isChecked());

    appearanceConfigGroup.sync();
}

void AppearanceConfigTab::loadTab()
{
    ChatWindowStyleManager *manager = ChatWindowStyleManager::self();
    manager->loadStyles();

    AdiumThemeView::ChatType chatType;

    if( m_groupChat ) {
        chatType = AdiumThemeView::GroupChat;
    } else {
        chatType = AdiumThemeView::SingleUserChat;
    }

    ui->chatView->load(chatType);
    ui->chatView->initialise(m_demoChatHeader);

    ui->showHeader->setChecked(ui->chatView->isHeaderDisplayed());
    ui->customFontBox->setChecked(ui->chatView->isCustomFont());
    ui->fontFamily->setCurrentFont(QFont(ui->chatView->fontFamily()));
    ui->fontSize->setValue(ui->chatView->fontSize());
    ui->showPresenceCheckBox->setChecked(ui->chatView->showPresenceChanges());
    ui->showLeaveCheckBox->setChecked(ui->chatView->showLeaveChanges());
}

void AppearanceConfigTab::defaultTab()
{
    ChatWindowStyleManager *manager = ChatWindowStyleManager::self();
    manager->loadStyles();

    if (m_groupChat) {
        onVariantSelected(QLatin1String("SimKete.AdiumMessageStyle"));
    } else {
        onVariantSelected(QLatin1String("renkoo.AdiumMessageStyle"));
    }

    onStyleSelected(0);
    ui->showHeader->setChecked(false);
    ui->customFontBox->setChecked(false);
    ui->chatView->setUseCustomFont(false);
    ui->fontFamily->setCurrentFont(KGlobalSettings::generalFont());
    ui->fontSize->setValue(QWebSettings::DefaultFontSize);
    ui->showPresenceCheckBox->setChecked(!m_groupChat);

}
