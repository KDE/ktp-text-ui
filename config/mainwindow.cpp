#include "mainwindow.h"
#include "ui_chatwindowconfig.h"
#include "chatwindowstylemanager.h"

#include "telepathychatinfo.h"
#include "telepathychatmessageinfo.h"

#include <QDebug>
#include <KDebug>

MainWindow::MainWindow(QWidget *parent) :
        QDialog(parent),
        ui(new Ui::ChatWindowConfig)
{
    ui->setupUi(this);

    ChatWindowStyleManager* manager = ChatWindowStyleManager::self();
   // manager->loadStyles();
    connect(manager, SIGNAL(loadStylesFinished()), SLOT(onStylesLoaded()));

    //set up a pretend config chat.
    TelepathyChatInfo info;

    info.setChatName("A demo chat");
    info.setSourceName("Jabber");
    info.setTimeOpened(QDateTime::currentDateTime());
    info.setDestinationName("BobMarley@yahoo.com");
    info.setDestinationDisplayName("Bob Marley");

    ui->chatView->initialise(info);

    ui->showHeader->setChecked(ui->chatView->isHeaderDisplayed());

    connect(ui->chatView, SIGNAL(loadFinished(bool)), SLOT(sendDemoMessages()));
    connect(ui->styleComboBox, SIGNAL(activated(QString)), SLOT(onStyleSelected(QString)));
    connect(ui->variantComboBox, SIGNAL(activated(QString)), SLOT(onVariantSelected(QString)));
    connect(ui->showHeader,SIGNAL(clicked(bool)), SLOT(onShowHeaderChanged(bool)));
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
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

    QStringList styles = ChatWindowStyleManager::self()->getAvailableStyles();
    ChatWindowStyle *currentStyle = ui->chatView->chatStyle();

    ui->styleComboBox->addItems(styles);
    ui->styleComboBox->setCurrentItem(currentStyle->getStyleName());

    updateVariantsList();
    //FIXME call onStyleSelected
}


void MainWindow::updateVariantsList()
{
    kDebug();
    QHash<QString, QString> variants = ui->chatView->chatStyle()->getVariants();
    ui->variantComboBox->clear();
    ui->variantComboBox->addItems(variants.keys());

    //FIXME maybe - select the correct one.
    //ui->variantComboBox->setCurrentItem(currentStyle->getCurrentVariantPath());

}


void MainWindow::onStyleSelected(const QString & styleName)
{
    kDebug();
    //load the style.
    ChatWindowStyle* style = ChatWindowStyleManager::self()->getValidStyleFromPool(styleName);
    ui->chatView->setChatStyle(style);
    updateVariantsList();
    ui->showHeader->setEnabled(style->hasHeader());

}

void MainWindow::onVariantSelected(const QString &variant)
{
    kDebug();
    ui->chatView->setVariant(variant);
}


void MainWindow::onShowHeaderChanged(bool showHeader)
{
    ui->chatView->setHeaderDisplayed(showHeader);
}


void MainWindow::sendDemoMessages()
{
    //add a fake message

    TelepathyChatMessageInfo message(TelepathyChatMessageInfo::RemoteToLocal);
    message.setMessage("Hello");
    message.setSenderDisplayName("larry@example.com");
    message.setSenderScreenName("Larry Demo");
    message.setService("Jabber");
    message.setTime(QDateTime::currentDateTime());
    ui->chatView->addMessage(message);

    message = TelepathyChatMessageInfo(TelepathyChatMessageInfo::LocalToRemote);
    message.setMessage("A different example message");
    message.setSenderDisplayName("ted@example.com");
    message.setSenderScreenName("Ted Example");
    message.setService("Jabber");
    message.setTime(QDateTime::currentDateTime());
    ui->chatView->addMessage(message);

    message = TelepathyChatMessageInfo(TelepathyChatMessageInfo::Status);
    message.setMessage("Ted Example has left the chat."); //FIXME sync this with chat text logic.
    message.setTime(QDateTime::currentDateTime());
    ui->chatView->addMessage(message);
}

void MainWindow::accept()
{
    kDebug();

    KSharedConfigPtr config = KSharedConfig::openConfig("ktelepathyrc");
    //KConfig config(KGlobal::dirs()->findResource("config","ktelepathyrc"));
    KConfigGroup appearanceConfig = config->group("Appearance");

    appearanceConfig.writeEntry("styleName", ui->styleComboBox->currentText());
    appearanceConfig.writeEntry("styleVariant", ui->variantComboBox->currentText());
    appearanceConfig.writeEntry("displayHeader", ui->showHeader->isChecked());

    appearanceConfig.sync();
    config->sync();

    QDialog::accept();
}
