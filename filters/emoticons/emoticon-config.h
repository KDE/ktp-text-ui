/*
 * Copyright (C) 2015 David Rosca <nowrep@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef EMOTICON_CONFIG_H
#define EMOTICON_CONFIG_H

#include <QHash>

#include <KCModule>
#include <KSharedConfig>

#include "ui_emoticon-config.h"

class EmoticonFilterConfig : public KCModule
{
    Q_OBJECT

public:
    explicit EmoticonFilterConfig(QWidget *parent = Q_NULLPTR, const QVariantList &args = QVariantList());

    void load() Q_DECL_OVERRIDE;
    void save() Q_DECL_OVERRIDE;
    void defaults() Q_DECL_OVERRIDE;

private Q_SLOTS:
    void addClicked();
    void removeClicked();
    void dataChanged(const QModelIndex &index, int role, const QVariant &data);
    void updateButtons();

private:
    QStringList accountsNotInList() const;

    Ui::EmoticonConfig ui;
    KSharedConfig::Ptr m_config;
    QHash<QString, QString> m_accounts;
};

#endif // EMOTICON_CONFIG_H
