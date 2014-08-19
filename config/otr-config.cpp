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
#include "otr-types.h"

#include <QLatin1String>
#include <KDebug>
#include <KPluginFactory>
#include <KLocalizedString>
#include <QtEvents>
#include <QDBusConnection>
#include <QAction>
#include <QMenu>
#include <KMessageBox>

#include <TelepathyQt/AccountSet>
#include <TelepathyQt/PendingVariant>

K_PLUGIN_FACTORY(KCMTelepathyChatOtrConfigFactory, registerPlugin<OTRConfig>();)
K_EXPORT_PLUGIN(KCMTelepathyChatOtrConfigFactory("ktp_chat_otr", "kcm_ktp_chat_otr"))

OTRConfig::OTRConfig(QWidget *parent, const QVariantList& args)
    : KCModule(KCMTelepathyChatOtrConfigFactory::componentData(), parent, args),
      ui(new Ui::OTRConfigUi()),
      am(Tp::AccountManager::create(QDBusConnection::sessionBus())),
      ps(NULL),
      fpCtxMenu(new QMenu(this))
{
    kDebug();

    ui->setupUi(this);

    ui->policyGroupButtons->setId(ui->rbAlways, Tp::OTRPolicyAlways);
    ui->policyGroupButtons->setId(ui->rbOpportunistic, Tp::OTRPolicyOpportunistic);
    ui->policyGroupButtons->setId(ui->rbManual, Tp::OTRPolicyManual);
    ui->policyGroupButtons->setId(ui->rbNever, Tp::OTRPolicyNever);

    ui->tbFingerprints->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tbFingerprints, SIGNAL(customContextMenuRequested(QPoint)), SLOT(onFpTableMenuRequested(QPoint)));

    connect(ui->policyGroupButtons, SIGNAL(buttonClicked(int)), SLOT(onRadioSelected(int)));
    connect(ui->btGenFingerprint, SIGNAL(clicked()), SLOT(onGenerateClicked()));
    connect(ui->cbAccounts, SIGNAL(activated(int)), SLOT(onAccountChosen(int)));
    connect(ui->tbFingerprints, SIGNAL(currentCellChanged(int,int,int,int)), SLOT(onCurrentFpCellChanged(int,int,int,int)));

    QAction *verifyAction = new QAction(i18n("Verify"), this);
    QAction *forgetAction = new QAction(i18n("Forget"), this);
    verifyAction->setEnabled(false);
    forgetAction->setEnabled(false);
    fpCtxMenu->addAction(verifyAction);
    fpCtxMenu->addAction(forgetAction);

    connect(verifyAction, SIGNAL(triggered()), SLOT(onVerifyClicked()));
    connect(forgetAction, SIGNAL(triggered()), SLOT(onForgetClicked()));
    connect(ui->btForget, SIGNAL(clicked()), forgetAction, SLOT(trigger()));
    connect(ui->btVerify, SIGNAL(clicked()), verifyAction, SLOT(trigger()));
}

OTRConfig::~OTRConfig()
{
    delete ui;
}

ProxyService* OTRConfig::proxyService()
{
    return ps;
}

void OTRConfig::setProxyService(ProxyService *proxyService)
{
    ps = proxyService;
    connect(ps, SIGNAL(keyGenerationFinished(Tp::AccountPtr, bool)), SLOT(onKeyGenerationFinished()));
}

void OTRConfig::load()
{
    Q_ASSERT(ps != NULL);
    kDebug();
    accounts = am->validAccounts()->accounts();
    QStringList items;
    Q_FOREACH(const Tp::AccountPtr ac, accounts) {
        items << ac->normalizedName();
    }
    ui->cbAccounts->clear();
    ui->cbAccounts->addItems(items);

    if(!items.isEmpty()) {
        ui->cbAccounts->setEnabled(true);
        ui->btGenFingerprint->setEnabled(true);
        ui->tlFingerprint->setEnabled(true);
        onAccountChosen(0);
    }

    updatePolicy();
    loadFingerprints();
}

void OTRConfig::loadFingerprints()
{
    if(accounts.isEmpty()) {
        ui->tbFingerprints->setRowCount(0);
        return;
    }

    const Tp::FingerprintInfoList fingerprints = ps->knownFingerprints(currentAccount());
    kDebug() << fingerprints.size();
    ui->tbFingerprints->setRowCount(fingerprints.size());
    int i = 0;
    Q_FOREACH(const Tp::FingerprintInfo &fp, fingerprints) {
        ui->tbFingerprints->setItem(i, 0, new QTableWidgetItem(fp.contactName));
        ui->tbFingerprints->setItem(i, 1, new QTableWidgetItem(fp.fingerprint));
        ui->tbFingerprints->setItem(i, 2, new QTableWidgetItem(fp.isVerified ? i18n("yes") : i18n("no")));
        ui->tbFingerprints->setItem(i, 3, new QTableWidgetItem(fp.inUse ? i18n("in use") : i18n("not in use")));

        i++;
    }

    ui->tbFingerprints->resizeColumnsToContents();
    ui->tbFingerprints->resizeRowsToContents();
    ui->tbFingerprints->horizontalHeader()->setStretchLastSection(true);
}

void OTRConfig::save()
{
    kDebug();
    connect(ps->setOTRPolicy(static_cast<uint>(policy)), SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onPolicySet(Tp::PendingOperation*)));
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

void OTRConfig::onRadioSelected(int id)
{
    kDebug();
    policy = static_cast<Tp::OTRPolicy>(id);
    Q_EMIT changed(true);
}

void OTRConfig::onGenerateClicked()
{
    kDebug();
    ps->generatePrivateKey(currentAccount());
}

void OTRConfig::onAccountChosen(int id)
{
    kDebug();
    const QString fp = ps->fingerprintForAccount(QDBusObjectPath(accounts.at(id)->objectPath()));
    if(fp.isEmpty()) {
        ui->tlFingerprint->setText(i18n("No fingerprint"));
    } else {
        ui->tlFingerprint->setText(fp);
    }
    loadFingerprints();
}

QDBusObjectPath OTRConfig::currentAccount() const
{
    const int index = ui->cbAccounts->currentIndex();
    return QDBusObjectPath(accounts.at(index)->objectPath());
}

void OTRConfig::updatePolicy()
{
    connect(ps->getOTRPolicy(), SIGNAL(finished(Tp::PendingOperation*)), SLOT(onPolicyGet(Tp::PendingOperation*)));
}

void OTRConfig::onPolicyGet(Tp::PendingOperation *getOp)
{
    if(getOp->isError()) {
        kWarning() << "Could not get OTR policy: " << getOp->errorMessage();
    } else {
        Tp::PendingVariant *pv = dynamic_cast<Tp::PendingVariant*>(getOp);
        const uint id = pv->result().toUInt(NULL);
        Q_FOREACH(QAbstractButton *bt, ui->policyGroupButtons->buttons()) {
            bt->setChecked(false);
        }
        ui->policyGroupButtons->button(id)->setChecked(true);
        policy = static_cast<Tp::OTRPolicy>(id);
    }
}

void OTRConfig::onPolicySet(Tp::PendingOperation *setOp)
{
    if(setOp->isError()) {
        kWarning() << "OTR policy set error: " << setOp->errorMessage();
    } else {
        updatePolicy();
    }
}

void OTRConfig::onKeyGenerationFinished()
{
    onAccountChosen(ui->cbAccounts->currentIndex());
}

void OTRConfig::onCurrentFpCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    Q_UNUSED(currentColumn);
    Q_UNUSED(previousRow);
    Q_UNUSED(previousColumn);

    if(currentRow != -1) {
        ui->btVerify->setEnabled(true);
        fpCtxMenu->actions().at(0)->setEnabled(true);
        if(ui->tbFingerprints->item(ui->tbFingerprints->currentRow(), 3)->text() == i18n("not in use")) {
            ui->btForget->setEnabled(true);
            fpCtxMenu->actions().at(1)->setEnabled(true);
        } else {
            ui->btForget->setEnabled(false);
            fpCtxMenu->actions().at(1)->setEnabled(false);
        }
    } else {
        ui->btForget->setEnabled(false);
        ui->btVerify->setEnabled(false);
        fpCtxMenu->actions().at(0)->setEnabled(false);
        fpCtxMenu->actions().at(1)->setEnabled(false);
    }
}

void OTRConfig::onFpTableMenuRequested(QPoint pos)
{
    kDebug();

    fpCtxMenu->popup(ui->tbFingerprints->viewport()->mapToGlobal(pos));
}

void OTRConfig::onVerifyClicked()
{
    const QString contact = ui->tbFingerprints->item(ui->tbFingerprints->currentRow(), 0)->text();
    const QString fingerprint = ui->tbFingerprints->item(ui->tbFingerprints->currentRow(), 1)->text();
    int trust = KMessageBox::questionYesNo(this,
            i18n("Please contact %1 via another secure way and verify that the following fingerprint is correct:", contact) +
            QLatin1String("\n\n") +
            fingerprint + QLatin1String("\n\n") +
            i18n("Are you sure you want to trust this fingerprint?"));

    if(trust == KMessageBox::Yes) {
        ps->trustFingerprint(currentAccount(), contact, fingerprint, true);
    } else {
        ps->trustFingerprint(currentAccount(), contact, fingerprint, false);
    }
    loadFingerprints();
}

void OTRConfig::onForgetClicked()
{
    kDebug();
    ps->forgetFingerprint(
            currentAccount(),
            ui->tbFingerprints->item(ui->tbFingerprints->currentRow(), 0)->text(),
            ui->tbFingerprints->item(ui->tbFingerprints->currentRow(), 1)->text());

    loadFingerprints();
    onCurrentFpCellChanged(ui->tbFingerprints->currentRow(), ui->tbFingerprints->currentColumn(), 0, 0);
}
