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


#ifndef OTR_CONFIG_H
#define OTR_CONFIG_H

#include "proxy-service.h"
#include "types.h"

#include <KTp/OTR/constants.h>

#include <KCModule>
#include <QList>

#include <TelepathyQt/AccountManager>

namespace Ui {
class OTRConfigUi;
}
class QMenu;

class OTRConfig : public KCModule
{
    Q_OBJECT
    Q_PROPERTY(ProxyServicePtr proxyService READ proxyService WRITE setProxyService)

public:
    explicit OTRConfig(QWidget *parent = 0, const QVariantList &args = QVariantList());
    virtual ~OTRConfig();

protected:
    virtual void changeEvent(QEvent *e);

public Q_SLOTS:
    virtual void load();
    virtual void save();
    ProxyServicePtr proxyService();
    void setProxyService(const ProxyServicePtr &ps);

private Q_SLOTS:
    void onRadioSelected(int id);
    void onGenerateClicked();
    void onAccountChosen(int id);
    void updatePolicy();
    void onPolicyGet(Tp::PendingOperation *getOp);
    void onPolicySet(Tp::PendingOperation *setOp);
    void onKeyGenerationFinished();
    void onCurrentFpCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
    void onVerifyClicked();
    void onForgetClicked();
    void onFpTableMenuRequested(QPoint pos);

private:
    QDBusObjectPath currentAccount() const;
    void loadFingerprints();

private:
    Ui::OTRConfigUi *ui;
    Tp::AccountManagerPtr am;
    QList<Tp::AccountPtr> accounts;
    KTp::OTRPolicy policy;
    ProxyServicePtr ps;
    QMenu *fpCtxMenu;
};

#endif // OTR_CONFIG_H
