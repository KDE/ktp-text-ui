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

#ifndef PROXY_SERIVCE_HEADER
#define PROXY_SERIVCE_HEADER

#include "otr-types.h"
#include "ktpchat_export.h"

#include <QWidget>
#include <TelepathyQt/AccountManager>

class KDE_TELEPATHY_CHAT_EXPORT ProxyService : public QObject
{
    Q_OBJECT

    public:
        ProxyService(
                const QDBusConnection &dbusConnection,
                const QString& busName,
                const QString& objectPath,
                QWidget* parent = 0);

        ~ProxyService();

        /** return true if generation could be started */
        bool generatePrivateKey(const QDBusObjectPath &account);
        bool isOngoingGeneration(const QDBusObjectPath &account);
        /** fingerprint or an empty string of none available */
        QString fingerprintForAccount(const QDBusObjectPath &account) const;
        Tp::FingerprintInfoList knownFingerprints(const QDBusObjectPath &account) const;
        bool trustFingerprint(const QDBusObjectPath &account, const QString &contactName, const QString &fingerprint, bool trust);
        bool forgetFingerprint(const QDBusObjectPath &account, const QString &contactName, const QString &fingerprint);

        Tp::PendingVariant* getOTRPolicy() const;
        Tp::PendingOperation* setOTRPolicy(uint policy);

    private Q_SLOTS:
        void onKeyGenerationStarted(const QDBusObjectPath &path);
        void onKeyGenerationFinished(const QDBusObjectPath &path, bool error);
        void onDialogClosed();

    Q_SIGNALS:
        void proxyConnected(const QDBusObjectPath &proxy);
        void proxyDisconnected(const QDBusObjectPath &proxy);
        void keyGenerationStarted(Tp::AccountPtr account);
        void keyGenerationFinished(Tp::AccountPtr, bool error);

    private:
        class Private;
        Private *d;
};

Q_DECLARE_METATYPE(ProxyService*)

#endif
