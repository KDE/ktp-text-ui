/*
    Copyright (C) 2012 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef HIDEWINDOWCOMPONENT_H
#define HIDEWINDOWCOMPONENT_H

#include <QObject>

/**
 * Plasma is not exposing such a feature to make its dialogs hidden from the taskbar,
 * that's why we added that weird object
 */

class HideWindowComponent : public QObject
{
    Q_OBJECT
    public:
        explicit HideWindowComponent(QObject* parent = 0);
        
        Q_SCRIPTABLE void hideWindowFromTaskbar(qulonglong winId);
};

#endif // HIDEWINDOWCOMPONENT_H
