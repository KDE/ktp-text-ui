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

#include "otr-constants.h"

#include <KCModule>
#include <QList>

#include <TelepathyQt/AccountManager>

namespace Ui {
class OTRConfigUi;
}

class OTRConfig : public KCModule
{
    Q_OBJECT

public:
    explicit OTRConfig(QWidget *parent = 0, const QVariantList &args = QVariantList());
    virtual ~OTRConfig();

protected:
    virtual void changeEvent(QEvent *e);

public Q_SLOTS:
    virtual void load();
    virtual void save();

private Q_SLOTS:
    void onRadioSelected(int id);
    void onGenerateClicked();
    void onAccountChosen(int id);

private:
    Ui::OTRConfigUi *ui;
    Tp::AccountManagerPtr am;
    QList<Tp::AccountPtr> accounts;
    QMap<int, QString> fpCache;
    Tp::OTRPolicy policy;
};

#endif // OTR_CONFIG_H
