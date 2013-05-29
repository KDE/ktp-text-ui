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

#ifndef LATEX_CONFIG_H
#define LATEX_CONFIG_H

#include <KCModule>

#include "ui_latex-config.h"

class LatexFilterConfig : public KCModule
{
    Q_OBJECT

public:
    explicit LatexFilterConfig(QWidget* parent = 0, const QVariantList& args = QVariantList());
    virtual ~LatexFilterConfig();

private:
    Ui::LatexConfig ui;
};

#endif // LATEX_CONFIG_H