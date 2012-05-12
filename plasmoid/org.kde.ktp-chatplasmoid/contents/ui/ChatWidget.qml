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
import org.kde.telepathy.chat 0.1
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets
import org.kde.plasma.components 0.1 as PlasmaComponents

Item {
    property Conversation conv

    signal closeRequested

    Item {
        id: titleArea
        anchors {
            margins: 5
            top: parent.top
            left: parent.left
            right: parent.right
        }
        height: 24

        PlasmaComponents.ToolButton {
            id: contactButton

            anchors {
                top: parent.top
                left: parent.left
                right: popoutButton.left
                bottom: parent.bottom
            }

            text: conv.target.nick
            iconSource: conv.target.presenceIcon

            onClicked: closeRequested()
        }

        PlasmaComponents.ToolButton {
            id: popoutButton

            anchors {
                top: parent.top
                right: closeButton.left
                bottom: parent.bottom
            }

            iconSource: "view-conversation-balloon"

            onClicked: {
                conv.delegateToProperClient();
                closeRequested();
            }
        }

        PlasmaComponents.ToolButton {
            id: closeButton

            anchors {
                top: parent.top
                right: parent.right
                bottom: parent.bottom
            }

            iconSource: "dialog-close"

            onClicked: conv.requestClose()
        }
    }

    PlasmaWidgets.Separator {
        id: space
        anchors {
            top: titleArea.bottom
            left: parent.left
            right: parent.right
        }
        orientation: Qt.Horizontal
    }

    ListView {
        id: view
        anchors {
            top: space.bottom
            left: parent.left
            right: parent.right
            bottom: input.top
            rightMargin: viewScrollBar.width+5
            leftMargin: 5
        }
        boundsBehavior: Flickable.StopAtBounds
        section.property: "user"
        section.delegate: PlasmaComponents.Label { text: section; font.bold: true; anchors.right: parent.right}
        clip: true

        delegate: Loader {
            Component.onCompleted: {
                console.log(model.type);
                switch(model.type) {
                    case MessagesModel.MessageTypeOutgoing:
//                             console.log("Type: MessagesModel::MessageTypeOutgoing");
                        source = "OutgoingDelegate.qml"
                        break;
                    case MessagesModel.MessageTypeAction:
//                             console.log("Type: MessagesModel::MessageTypeAction");
                        source = "ActionDelegate.qml";
                        break;
                    case MessagesModel.MessageTypeIncoming:
                    default:
                        source = "TextDelegate.qml";
                }
            }
        }

        model: conv.messages

        onCountChanged: {
            if((view.contentHeight-view.contentY)<(1.2*view.height)) {
                view.positionViewAtEnd()
            }
        }
    }

    PlasmaComponents.ScrollBar {
        id: viewScrollBar
        anchors {
            top: view.top
            bottom: view.bottom
            right: parent.right
        }

        flickableItem: view
        width: 16
        orientation: Qt.Vertical
        opacity: view.atYEnd ? 0.3 : 1

        Behavior on width { NumberAnimation { duration: 250 } }
        Behavior on opacity { NumberAnimation { duration: 250 } }
    }

    PlasmaComponents.TextField {
        id: input
        focus: true

        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        Keys.onReturnPressed: {
            view.model.sendNewMessage(text);
            text = "";
        }
    }
}
