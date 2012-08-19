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

#include <KPluginFactory>

K_PLUGIN_FACTORY(PipesConfigFactory, registerPlugin<PipesConfig>();)
K_EXPORT_PLUGIN(PipesConfigFactory( "kcm_ktp_filter_pipes" ))

PipesConfig::PipesConfig(QWidget *parent, const QVariantList &args):
    KCModule(PipesConfigFactory::componentData(), parent, args)
{
}

void PipesConfig::defaults()
{
    m_prefs.reset();
}

void PipesConfig::load() {
    m_prefs.load();
}

void PipesConfig::save() {
    m_prefs.save();
}