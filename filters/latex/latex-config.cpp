/*
 *    Copyright (C) 2013  Andrea Scarpino <andrea@archlinux.org>
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

#include "latex-config.h"
// KConfigSkeleton
#include "latexconfig.h"

#include <KPluginFactory>

K_PLUGIN_FACTORY(LatexConfigFactory, registerPlugin<LatexFilterConfig>();)

LatexFilterConfig::LatexFilterConfig(QWidget* parent, const QVariantList& args)
    : KCModule(parent, args)
{
    ui.setupUi(this);
    addConfig(LatexConfig::self(), this);
    load();
}

LatexFilterConfig::~LatexFilterConfig()
{
}

#include "latex-config.moc"
