/*
 *    Copyright (C) 2012  Lasath Fernando <kde@lasath.org>
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 2.1 of the License, or (at your option) any later version.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "pipes-config.h"
#include "pipes-delegate.h"

#include <KPluginFactory>

K_PLUGIN_FACTORY(PipesConfigFactory, registerPlugin<PipesConfig>();)
K_EXPORT_PLUGIN(PipesConfigFactory( "kcm_ktp_filter_pipes" ))

PipesConfig::PipesConfig(QWidget *parent, const QVariantList &args):
    KCModule(PipesConfigFactory::componentData(), parent, args)
{
    m_ui.setupUi(this);

    {
        QHash<int, QString> map;
        map[PipesPrefs::Incoming] = i18n("Incoming");
        map[PipesPrefs::Outgoing] = i18n("Outgoing");
        map[PipesPrefs::Both] = i18n("Both");

        m_ui.tableView->setItemDelegateForColumn(
            PipesModel::DirectionColumn,
            new PipesDelegate(map, this)
        );
    }

    {
        QHash<int, QString> map;
        map[PipesPrefs::FormatPlainText] = i18n("Plain Text");

        m_ui.tableView->setItemDelegateForColumn(
            PipesModel::FormatColumn,
            new PipesDelegate(map, this)
        );
    }
    m_ui.tableView->setModel(&m_model);
    connect(&m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(changed()));

    connect(m_ui.addButton, SIGNAL(clicked(bool)), SLOT(addButtonPressed()));
    connect(m_ui.removeButton, SIGNAL(clicked(bool)), SLOT(removeButtonPressed()));
}

void PipesConfig::defaults()
{
    m_model.clear();
}

void PipesConfig::load() {
    m_model.revert();
}

void PipesConfig::save() {
    m_model.submit();
}

void PipesConfig::addButtonPressed()
{
    m_model.insertRow(m_model.rowCount());
}

void PipesConfig::removeButtonPressed()
{
    m_model.removeRow(m_ui.tableView->currentIndex().row());
    changed();
}
