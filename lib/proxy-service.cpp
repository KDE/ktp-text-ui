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
#include "ktp-debug.h"

#include <KTp/OTR/proxy-service-interface.h>

#include <QMap>
#include <QScopedPointer>
#include <QCloseEvent>
#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>

#include <KLocalizedString>

class KeyGenDialog : public QDialog
{
    public:
        KeyGenDialog(const QString &accountName, QWidget *parent = 0)
            : QDialog(parent),
            blocked(true),
            accountName(accountName)
        {
            QWidget *widget = new QWidget(this);
            ui.setupUi(widget);
            ui.lbText->setText(i18n("Generating the private key for %1...", accountName));
            ui.lbTime->setText(i18n("This may take some time"));

            QVBoxLayout *mainLayout = new QVBoxLayout(this);
            setLayout(mainLayout);
            mainLayout->addWidget(widget);

            buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, this);
            buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);
            mainLayout->addWidget(buttonBox);
            connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
            connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

            this->setWindowTitle(i18n("Please wait"));

            ui.keyIcon->setPixmap(QIcon::fromTheme(QStringLiteral("dialog-password")).pixmap( 48, 48 ));
        }
        ~KeyGenDialog()
        {
            qCDebug(KTP_TEXTUI_LIB) << "Destructing";
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
            ui.lbTime->clear();
            if(error) {
                ui.lbText->setText(i18n("Could not generate a private key for %1", accountName));
            } else {
                ui.lbText->setText(i18n("Finished generating the private key for %1", accountName));
            }
            buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);

        }

    private:
        bool blocked;
        const QString accountName;
        Ui::KeyGenDialog ui;
        QDialogButtonBox *buttonBox;

};

class ProxyService::Private
{
    public:
        Private(KTp::Client::ProxyServiceInterface *psi, const QDBusConnection &dbusConnection, QWidget *parent)
            : psi(psi),
            am(Tp::AccountManager::create(dbusConnection)),
            parent(parent)
        {
        }

        QScopedPointer<KTp::Client::ProxyServiceInterface> psi;
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
    d(new Private(new KTp::Client::ProxyServiceInterface(dbusConnection, busName, objectPath), dbusConnection, parent))
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

QString ProxyService::fingerprintForAccount(const QDBusObjectPath& account) const
{
    QDBusPendingReply<QString> rep = d->psi->GetFingerprintForAccount(account);
    rep.waitForFinished();
    if(rep.isValid()) {
        return rep.value();
    } else {
        qCWarning(KTP_TEXTUI_LIB) << "Could not get fingerprint of account: " << account.path() <<
            " due to: " << rep.error().message();
        return QLatin1String("");
    }
}

KTp::FingerprintInfoList ProxyService::knownFingerprints(const QDBusObjectPath &account) const
{
    QDBusPendingReply<KTp::FingerprintInfoList> fpsRep = d->psi->GetKnownFingerprints(account);
    fpsRep.waitForFinished();
    if(fpsRep.isValid()) {
        return fpsRep.value();
    } else {
        qCWarning(KTP_TEXTUI_LIB) << "Could not get known fingerprints for account: " << account.path() <<
            " due to: " << fpsRep.error().message();
        return KTp::FingerprintInfoList();
    }
}

bool ProxyService::trustFingerprint(const QDBusObjectPath &account, const QString &contactName, const QString &fingerprint, bool trust)
{
    QDBusPendingReply<> res = d->psi->TrustFingerprint(account, contactName, fingerprint, trust);
    res.waitForFinished();
    if(res.isValid()) {
        return true;
    } else {
        qCWarning(KTP_TEXTUI_LIB) << "Could not trust fingerprint " << fingerprint << " for account: " << account.path() <<
            " due to: " << res.error().message();
        return false;
    }
}

bool ProxyService::forgetFingerprint(const QDBusObjectPath &account, const QString &contactName, const QString &fingerprint)
{
    QDBusPendingReply<> res = d->psi->ForgetFingerprint(account, contactName, fingerprint);
    res.waitForFinished();
    if(res.isValid()) {
        return true;
    } else {
        qCWarning(KTP_TEXTUI_LIB) << "Could not forget fingerprint " << fingerprint << " for account: " << account.path() <<
            " due to: " << res.error().message();
        return false;
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
    qCDebug(KTP_TEXTUI_LIB);
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
    connect(it.value(), SIGNAL(finished(int)), SLOT(onDialogClosed()));

    Q_EMIT keyGenerationFinished(d->am->accountForObjectPath(accountPath.path()), error);
}

void ProxyService::onDialogClosed()
{
    KeyGenDialog *dialog = dynamic_cast<KeyGenDialog*>(QObject::sender());
    for(QMap<QString, KeyGenDialog*>::iterator it = d->dialogs.begin(); it != d->dialogs.end(); ++it) {
        if(it.value() == dialog) {
            d->dialogs.erase(it);
            dialog->hide();
            dialog->deleteLater();
            return;
        }
    }
}

