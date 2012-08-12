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

#include "substitution-config.h"

#include <KPluginFactory>
#include <KGlobal>
#include <KDebug>

K_PLUGIN_FACTORY(SubstitutionConfigFactory, registerPlugin<SubstitutionConfig>();)
K_EXPORT_PLUGIN(SubstitutionConfigFactory( "kcm_ktp_filter_substitution" ))

SubstitutionConfig::SubstitutionConfig(QWidget *parent, QVariantList args) :
    KCModule(SubstitutionConfigFactory::componentData(), parent, args),
    m_prefs(new SubstitutionPrefs()), m_ui(new Ui::SubstitutionConfigUi())
{
    m_ui->setupUi(this);
    m_ui->tableView->setModel(m_prefs);

    connect(m_ui->addButton, SIGNAL(clicked(bool)), SLOT(onAddWordPressed()));
    connect(m_ui->removeButton, SIGNAL(clicked(bool)), SLOT(onRemoveWordPressed()));
}

SubstitutionConfig::~SubstitutionConfig()
{
    delete m_prefs;
    delete m_ui;
}

void SubstitutionConfig::defaults()
{
    m_prefs->defaults();
}

void SubstitutionConfig::load()
{
    m_prefs->load();
}

void SubstitutionConfig::save()
{
    kDebug();
    m_prefs->save();
}

void SubstitutionConfig::onAddWordPressed()
{
    m_prefs->addReplacement(m_ui->sourceText->userText(), m_ui->resultText->userText());
    changed();
}

void SubstitutionConfig::onRemoveWordPressed()
{
    QString word = qvariant_cast<QString>(m_ui->tableView->currentIndex().data());
    kDebug() << "removing" << word;

    m_prefs->removeWord(word);
    changed();
}
