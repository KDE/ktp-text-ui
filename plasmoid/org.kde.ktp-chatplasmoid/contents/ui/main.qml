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

ListView {
    id: base
    anchors.fill: parent
    property alias minimumHeight: base.contentHeight
    property alias minimumWidth: base.contentWidth
    orientation: width>height ? ListView.Horizontal : ListView.Vertical

    model: handler.conversations

    TelepathyTextObserver {
        id: handler
    }

    delegate : ConversationDelegate {
        //FIXME: rename the two variables named 'conv' as it's confusing
        property alias active: conv.checked
        id: conv
        height: Math.min(base.width, base.height)

        image: model.conversation.target.avatar
        overlayText: model.conversation.messages.unreadCount

        //FIXME: put in a loader to not slow down the model
        PlasmaCore.Dialog {
            id: dialog
            //Set as a Tool window to bypass the taskbar
            windowFlags: Qt.WindowStaysOnTopHint | Qt.Tool
            visible: conv.checked

            mainItem: ChatWidget {
                width: 250
                height: 350
                conv: model.conversation

                onCloseRequested: {
                    conv.checked = false;
                }
            }
            
            onVisibleChanged: {
                if(visible) {
                    //hides the previously visible chat
                    base.currentItem.active = false

                    var point = dialog.popupPosition(conv, Qt.AlignBottom);
                    console.log("Showing dialog at (" + point.x + "," + point.y + ")");

                    dialog.x = point.x;
                    dialog.y = point.y;
                    dialog.activateWindow()
                    base.currentIndex = index
                }
                conv.checked = visible
            }
        }

        Connections {
            target: model.conversation.messages
            onPopoutRequested: {
                conv.checked = true;
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
