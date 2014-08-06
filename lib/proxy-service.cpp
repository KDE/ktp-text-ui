/*
    Copyright (C) 2014  Marcin Ziemiński   <zieminn@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "proxy-service.h"
#include "ui_keygendialog.h"

#include "proxy-service-interface.h"

#include <QMap>
#include <QScopedPointer>
#include <QCloseEvent>
#include <KDebug>
#include <KDialog>
#include <KLocale>

class KeyGenDialog : public KDialog
{
    public:
        KeyGenDialog(const QString &accountName, QWidget *parent = 0)
            : KDialog(parent),
            blocked(true),
            accountName(accountName)
        {
            QWidget *widget = new QWidget(this);
            ui.setupUi(widget);
            ui.lbText->setText(i18n("Generating the private key for %1", accountName));
            setMainWidget(widget);
            this->setCaption(i18n("Please wait"));
            this->setButtons(KDialog::Ok);
            this->enableButton(KDialog::Ok, false);
            ui.keyIcon->setPixmap(KIcon(QLatin1String("dialog-password")).pixmap( 48, 48 ));
        }
        ~KeyGenDialog()
        {
            kDebug() << "Destructing";
        }

        void block()
        {
            blocked = true;
        }

        void unblock()
        {
            blocked = false;
        }

        void closeEvent(QCloseEvent *e)
        {
            if(blocked) {
                e->ignore();
            } else {
                e->accept();
            }
        }

        void setFinished(bool error)
        {
            if(error) {
                ui.lbText->setText(i18n("Could not generate a private key for %1", accountName));
            } else {
                ui.lbText->setText(i18n("Finished generating the private key for %1", accountName));
            }
            this->enableButton(KDialog::Ok, true);
        }

    private:
        bool blocked;
        const QString accountName;
        Ui::KeyGenDialog ui;

};

class ProxyService::Private
{
    public:
        Private(Tp::Client::ProxyServiceInterface *psi, const QDBusConnection &dbusConnection, QWidget *parent)
            : psi(psi),
            am(Tp::AccountManager::create(dbusConnection)),
            parent(parent)
        {
        }

        QScopedPointer<Tp::Client::ProxyServiceInterface> psi;
        Tp::AccountManagerPtr am;
        QWidget *parent;
        QMap<QString, KeyGenDialog*> dialogs;
};

ProxyService::ProxyService(
        const QDBusConnection &dbusConnection,
        const QString& busName,
        const QString& objectPath,
        QWidget* parent)
    : QObject(parent),
    d(new Private(new Tp::Client::ProxyServiceInterface(dbusConnection, busName, objectPath), dbusConnection, parent))
{
    connect(d->psi.data(), SIGNAL(ProxyConnected(const QDBusObjectPath&)), SIGNAL(proxyConnected(const QDBusObjectPath&)));
    connect(d->psi.data(), SIGNAL(KeyGenerationStarted(const QDBusObjectPath&)),
            SLOT(onKeyGenerationStarted(const QDBusObjectPath&)));
    connect(d->psi.data(), SIGNAL(KeyGenerationFinished(const QDBusObjectPath&, bool)),
            SLOT(onKeyGenerationFinished(const QDBusObjectPath&, bool)));
}

ProxyService::~ProxyService()
{
    delete d;
}

bool ProxyService::generatePrivateKey(const QDBusObjectPath& account)
{
    QDBusPendingReply<> rep = d->psi->GeneratePrivateKey(account);
    rep.waitForFinished();
    return !rep.isError();
}

bool ProxyService::isOngoingGeneration(const QDBusObjectPath &account)
{
    return d->dialogs.contains(account.path());
}

QString ProxyService::fingerprintForAccount(const QDBusObjectPath& account)
{
    QDBusPendingReply<QString> rep = d->psi->GetFingerprintForAccount(account);
    rep.waitForFinished();
    if(rep.isValid()) {
        return rep.value();
    } else {
        kWarning() << "Could not get fingerprint of account: " << account.path() <<
            " due to: " << rep.error().message();
        return QLatin1String("");
    }
}

Tp::PendingVariant* ProxyService::getOTRPolicy() const
{
    return d->psi->requestPropertyPolicySettings();
}

Tp::PendingOperation* ProxyService::setOTRPolicy(uint policy)
{
    return d->psi->setPropertyPolicySettings(policy);
}

void ProxyService::onKeyGenerationStarted(const QDBusObjectPath &accountPath)
{
    kDebug();
    KeyGenDialog *dialog = new KeyGenDialog(
                d->am->accountForObjectPath(accountPath.path())->normalizedName(),
                d->parent);

    d->dialogs.insert(accountPath.path(), dialog);
    dialog->block();
    dialog->show();

    Q_EMIT keyGenerationStarted(d->am->accountForObjectPath(accountPath.path()));
}

void ProxyService::onKeyGenerationFinished(const QDBusObjectPath &accountPath, bool error)
{
    QMap<QString, KeyGenDialog*>::iterator it = d->dialogs.find(accountPath.path());
    if(it == d->dialogs.end()) {
        return;
    }
    it.value()->setFinished(error);
    it.value()->unblock();
    connect(it.value(), SIGNAL(closeClicked()), SLOT(onDialogClosed()));
    connect(it.value(), SIGNAL(okClicked()), SLOT(onDialogClosed()));

    Q_EMIT keyGenerationFinished(d->am->accountForObjectPath(accountPath.path()), error);
}

void ProxyService::onDialogClosed()
{
    KeyGenDialog *dialog = dynamic_cast<KeyGenDialog*>(QObject::sender());
    for(QMap<QString, KeyGenDialog*>::iterator it = d->dialogs.begin(); it != d->dialogs.end(); ++it) {
        if(it.value() == dialog) {
            d->dialogs.erase(it);
            dialog->delayedDestruct();
            return;
        }
    }
}

