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

#include "pipes-prefs.h"
#include <KSharedConfig>

PipesPrefs::PipesPrefs()
{
}

KConfigGroup PipesPrefs::config() const
{
    return KSharedConfig::openConfig(QLatin1String("ktelepathyrc"))->group("Pipes");
}

void PipesPrefs::load()
{
    Q_FOREACH (QString name, config().groupList()) {
        KConfigGroup cfg = config().group(name);
        Pipe p;

        p.direction = (PipeDirection) cfg.readEntry<int>("dir", 7);
        Q_ASSERT (p.direction != 7);

        p.format = FormatPlainText; //since we only have one format
        p.executable = cfg.readEntry("exec");

        m_pipeList.append(p);
    }
}

void PipesPrefs::save()
{
    config().deleteGroup();

    for (int i; i < m_pipeList.length(); i++) {
        KConfigGroup cfg = config().group(QString(QLatin1String("%1")).arg(i));
        cfg.writeEntry("dir", (int) m_pipeList[i].direction);
        cfg.writeEntry("exec", m_pipeList[i].executable);
        cfg.writeEntry("format", (int) m_pipeList[i].format);
        cfg.sync();
    }
}

PipesPrefs::PipeList PipesPrefs::pipeList() const
{
    return m_pipeList;
}

void PipesPrefs::reset()
{
    m_pipeList.clear();
}
