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

#ifndef KCM_MESSAGES_H
#define KCM_MESSAGES_H

#include <KCModule>

class KPluginSelector;

class MessagesConfig : public KCModule
{
Q_OBJECT

public:
    explicit MessagesConfig(QWidget *parent = nullptr, const QVariantList &args = QVariantList());

    void save() override;
    void defaults() override;
    void load() override;
private:
    KPluginSelector *m_pluginSelector;
};

#endif // KCM_MESSAGES_H
