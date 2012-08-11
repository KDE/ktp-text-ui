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

import Qt 4.7
import org.kde.telepathy.chat 0.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1 as ExtraComponents

ListView {
    id: base
    anchors.fill: parent
    property alias minimumHeight: base.contentHeight
    property alias minimumWidth: base.contentWidth
    orientation: (plasmoid.formFactor === Vertical ? ListView.Vertical
                    : plasmoid.formFactor === Horizontal ? ListView.Horizontal
                    : width>height ? ListView.Horizontal : ListView.Vertical)

    model: handler.conversations
    currentIndex: -1
    interactive: false

    TelepathyTextObserver {
        id: handler
    }

    HideWindowComponent {
        id: windowHide
    }

    delegate : ConversationDelegate {
        id: convButton
        height: Math.min(base.width, base.height)

        image: model.conversation.target.avatar
        overlayText: model.conversation.messages.unreadCount
        onClicked: {
            if(base.currentIndex == index)
                base.currentIndex = -1
            else
                base.currentIndex = index
        }

        //FIXME: put in a loader to not slow down the model
        PlasmaCore.Dialog {
            id: dialog
            windowFlags: Qt.WindowStaysOnTopHint
            visible: base.currentIndex==index

            mainItem: ChatWidget {
                width: 250
                height: 350
                conv: model.conversation

                onCloseRequested: {
                     base.currentIndex = -1
                }
            }

            onVisibleChanged: {
                if(visible) {
                    windowHide.hideWindowFromTaskbar(dialog.windowId)
                    var point = dialog.popupPosition(convButton, Qt.AlignBottom);
                    console.log("Showing dialog at (" + point.x + "," + point.y + ")");

                    dialog.x = point.x;
                    dialog.y = point.y;
                    dialog.activateWindow()
                }

            }
        }

        Connections {
            target: model.conversation.messages
            onPopoutRequested: {
                base.currentIndex = -1
            }
        }

        // needed to let MessageModel know when messages are visible
        // so that it can acknowledge them properly
        Binding {
            target: model.conversation.messages
            property: "visibleToUser"
            value: dialog.visible
        }
    }
}
