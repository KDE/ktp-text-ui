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

#include "main-window.h"
#include "ui_chatwindowconfig.h"

#include <KDETelepathy/ChatWindowStyleManager>
#include <KDETelepathy/AdiumThemeHeaderInfo>
#include <KDETelepathy/AdiumThemeContentInfo>
#include <KDETelepathy/AdiumThemeStatusInfo>

#include <KDebug>
#include <KLocalizedString>

K_PLUGIN_FACTORY(KCMTelepathyChatConfigFactory, registerPlugin<MainWindow>();)
K_EXPORT_PLUGIN(KCMTelepathyChatConfigFactory("telepathy_chat_config", "kcm_telepathy_chat_config"))

MainWindow::MainWindow(QWidget *parent, const QVariantList& args)
    : KCModule(KCMTelepathyChatConfigFactory::componentData(), parent, args),
      ui(new Ui::ChatWindowConfig)
{
    ui->setupUi(this);

    ChatWindowStyleManager* manager = ChatWindowStyleManager::self();
    manager->loadStyles();
    connect(manager, SIGNAL(loadStylesFinished()), SLOT(onStylesLoaded()));

    m_demoChatHeader.setChatName(i18n("A demo chat"));
    m_demoChatHeader.setSourceName(i18n("Jabber"));
    m_demoChatHeader.setTimeOpened(QDateTime::currentDateTime());
    m_demoChatHeader.setDestinationName(i18n("BobMarley@yahoo.com"));
    m_demoChatHeader.setDestinationDisplayName(i18n("Bob Marley"));

    ui->chatView->initialise(m_demoChatHeader);

    ui->showHeader->setChecked(ui->chatView->isHeaderDisplayed());

    connect(ui->chatView, SIGNAL(loadFinished(bool)), SLOT(sendDemoMessages()));
    connect(ui->styleComboBox, SIGNAL(activated(int)), SLOT(onStyleSelected(int)));
    connect(ui->variantComboBox, SIGNAL(activated(QString)), SLOT(onVariantSelected(QString)));
    connect(ui->showHeader, SIGNAL(clicked(bool)), SLOT(onShowHeaderChanged(bool)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
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

void MainWindow::onStylesLoaded()
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

void MainWindow::updateVariantsList()
{
    kDebug();
    QHash<QString, QString> variants = ui->chatView->chatStyle()->getVariants();
    ui->variantComboBox->clear();
    ui->variantComboBox->addItems(variants.keys());
    ui->variantComboBox->setCurrentItem(ui->chatView->variantName());
}

void MainWindow::onStyleSelected(int index)
{
    kDebug();
    //load the style.
    QString styleId = ui->styleComboBox->itemData(index).toString();

    ChatWindowStyle* style = ChatWindowStyleManager::self()->getValidStyleFromPool(styleId);

    if (style) {
        ui->chatView->setChatStyle(style);
        updateVariantsList();
        ui->showHeader->setEnabled(style->hasHeader());
        ui->chatView->initialise(m_demoChatHeader);
    }
    changed();
}

void MainWindow::onVariantSelected(const QString &variant)
{
    kDebug();
    ui->chatView->setVariant(variant);
    ui->chatView->initialise(m_demoChatHeader);
    changed();
}

void MainWindow::onShowHeaderChanged(bool showHeader)
{
    ui->chatView->setHeaderDisplayed(showHeader);
    ui->chatView->initialise(m_demoChatHeader);
    changed();
}

void MainWindow::sendDemoMessages()
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

void MainWindow::save()
{
    kDebug();

    KSharedConfigPtr config = KSharedConfig::openConfig("ktelepathyrc");
    KConfigGroup appearanceConfig = config->group("Appearance");

    appearanceConfig.writeEntry("styleName", ui->styleComboBox->itemData(ui->styleComboBox->currentIndex()).toString());
    appearanceConfig.writeEntry("styleVariant", ui->variantComboBox->currentText());
    appearanceConfig.writeEntry("displayHeader", ui->showHeader->isChecked());

    appearanceConfig.sync();
    config->sync();

    KCModule::save();
}
