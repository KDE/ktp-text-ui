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
#include <QtDBus/QDBusConnection>

#include <TelepathyQt/AccountSet>
#include <TelepathyQt/PendingVariant>

K_PLUGIN_FACTORY(KCMTelepathyChatOtrConfigFactory, registerPlugin<OTRConfig>();)
K_EXPORT_PLUGIN(KCMTelepathyChatOtrConfigFactory("ktp_chat_otr", "kcm_ktp_chat_otr"))

OTRConfig::OTRConfig(QWidget *parent, const QVariantList& args)
    : KCModule(KCMTelepathyChatOtrConfigFactory::componentData(), parent, args),
      ui(new Ui::OTRConfigUi()),
      am(Tp::AccountManager::create(QDBusConnection::sessionBus())),
      ps(NULL)
{
    kDebug();

    ui->setupUi(this);

    ui->policyGroupButtons->setId(ui->rbAlways, Tp::OTRPolicyAlways);
    ui->policyGroupButtons->setId(ui->rbOpportunistic, Tp::OTRPolicyOpportunistic);
    ui->policyGroupButtons->setId(ui->rbManual, Tp::OTRPolicyManual);
    ui->policyGroupButtons->setId(ui->rbNever, Tp::OTRPolicyNever);
    connect(ui->policyGroupButtons, SIGNAL(buttonClicked(int)), SLOT(onRadioSelected(int)));

    connect(ui->btGenFingerprint, SIGNAL(clicked()), SLOT(onGenerateClicked()));

    connect(ui->cbAccounts, SIGNAL(activated(int)), SLOT(onAccountChosen(int)));
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
    const int index = ui->cbAccounts->currentIndex();
    ps->generatePrivateKey(QDBusObjectPath(accounts.at(index)->objectPath()));
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
