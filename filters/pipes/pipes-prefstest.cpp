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
#include "pipes-prefstest.h"

void PipesPrefsTest::saveLoadTest()
{
    PipesPrefs::PipeList pipeList;
    pipeList << PipesPrefs::Pipe(QLatin1String("/usr/bin/cat"), PipesPrefs::Both);
    pipeList << PipesPrefs::Pipe(QLatin1String("sed 's/herp/derp/g'"), PipesPrefs::Incoming);

    {
        PipesPrefs pref;
        pref.m_pipeList = pipeList;
        pref.save();
    }

    {
        PipesPrefs pref;
        pref.load();
        QCOMPARE(pref.m_pipeList, pipeList);
    }
}


QTEST_MAIN(PipesPrefsTest);

#include "moc_pipes-prefstest.cpp"