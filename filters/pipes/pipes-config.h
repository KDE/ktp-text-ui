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

#ifndef PIPES_CONFIG_H
#define PIPES_CONFIG_H

#include "pipes-prefs.h"
#include "ui_pipes-config.h"

#include <KCModule>

class PipesConfig : public KCModule
{
public:
    explicit PipesConfig(const KComponentData &componentData, QWidget *parent = 0, const QVariantList &args = QVariantList());

    virtual void save();
    virtual void load();
    virtual void defaults();

private:
    PipesPrefs m_prefs;
    Ui::PipesConfigUi m_ui;
};

#endif // PIPES_CONFIG_H
