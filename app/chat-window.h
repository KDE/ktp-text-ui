/*
    Copyright (C) 2013  Daniel Cohen    <analoguecolour@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <KTabWidget>
#include <kparts/part.h>
#include <KParts/MainWindow>

#include "KTpTextChatPart.h"

class ChatWindow : public KParts::MainWindow
{
Q_OBJECT

public:
    explicit ChatWindow();
    virtual ~ChatWindow();
    void setupWindow();
    void addTab(QVariantList args, QString channelalias);


private Q_SLOTS:
    void onZoomFactorChanged(qreal zoom);
    void onZoomIn();
    void onZoomOut();
    void onTabStateChanged();

private:
    void setupActions(KTpTextChatPart* part);
    KTabWidget* partTabWidget = new KTabWidget;
    qreal m_zoomFactor;
    int currTab;
    KParts::PartManager *m_partManager;
    QMap<QWidget*, KParts::Part*> m_tabsPartsMap;

};

#endif // CHATWINDOW_H
