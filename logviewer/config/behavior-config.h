/*
 * Copyright (C) 2013  Daniel Vrátil <dvratil@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef BEHAVIORCONFIG_H
#define BEHAVIORCONFIG_H

#include <KCModule>

namespace Ui
{
class BehaviorConfig;
}

class BehaviorConfig : public KCModule
{
    Q_OBJECT

  public:
    explicit BehaviorConfig(QWidget *parent = 0, const QVariantList &args = QVariantList());
    virtual ~BehaviorConfig();

  public Q_SLOTS:
    virtual void load();
    virtual void save();

  Q_SIGNALS:
    void reloadMessages();

  protected:
    virtual void changeEvent(QEvent *event);

  private Q_SLOTS:
    void onRadioChanged();

  private:
    Ui::BehaviorConfig* ui;

};

#endif // BEHAVIORCONFIG_H
