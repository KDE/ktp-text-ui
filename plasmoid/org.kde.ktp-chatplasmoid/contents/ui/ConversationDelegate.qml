/*
    Copyright (C) 2012  Lasath Fernando <kde@lasath.org>
    Copyright (C) 2012 David Edmundson <kde@davidedmundson.co.uk>
    Copyright (C) 2012 Aleix Pol <aleixpol@kde.org>

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

import QtQuick 1.1
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1 as ExtraComponents
import org.kde.plasma.core 0.1 as PlasmaCore

PlasmaComponents.ToolButton {
    id: base
    width: height
    ExtraComponents.QIconItem {
        id: icon
        icon: model.conversation.target.avatar
        anchors {
            fill: parent
            margins: 5
        }
    }
    
    PlasmaCore.ToolTip {
      target: icon
      mainText: model.conversation.target.nick
      image: model.conversation.target.presenceIconName
    }


    Rectangle {
        anchors {
            right: parent.right
            top: parent.top
        }
        width: parent.width / 3
        height: parent.height / 3
        color: "red"
        radius: 3

        Text {
            id: text
            anchors.fill: parent

            font.pixelSize: parent.height
            text: model.conversation.messages.unreadCount
            color: "white"

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        visible: model.conversation.messages.unreadCount !== 0
    }
}
