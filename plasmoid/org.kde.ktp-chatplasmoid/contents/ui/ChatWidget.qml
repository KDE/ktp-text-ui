import QtQuick 1.1
import org.kde.telepathy.chat 0.1
import org.kde.plasma.core 0.1 as PlasmaCore
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
            left: parent.left; right: parent.right
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


    Item {
        id:chatArea

        anchors {
            top: space.bottom
            left: parent.left
            right: parent.right
            bottom: input.top

            leftMargin: 5
            rightMargin: chatArea.anchors.leftMargin
        }

//         contemplating if this makes it look better or worse
//         PlasmaComponents.Highlight { anchors.fill: chatArea }

        ListView {
            id: view
            anchors {
                top: parent.top
                bottom: parent.bottom
                left: parent.left
                right: viewScrollBar.left
            }
            boundsBehavior: Flickable.StopAtBounds
            clip: true

            delegate: Loader {
                Component.onCompleted: {
                    console.log(model.type);
                    switch(model.type) {
                        case MessagesModel.MessageTypeIncoming:
//                             console.log("Type: MessagesModel::MessageTypeIncoming");
                            source = "IncomingDelegate.qml";
                            break;
                        case MessagesModel.MessageTypeOutgoing:
//                             console.log("Type: MessagesModel::MessageTypeOutgoing");
                            source = "OutgoingDelegate.qml"
                            break;
                        case MessagesModel.MessageTypeAction:
//                             console.log("Type: MessagesModel::MessageTypeAction");
                            source = "ActionDelegate.qml";
                            break;
                        default:
//                             console.log("ERROR: UNKNOWN MESSAGE TYPE! Defaulting to fallback handler");
                            source = "TextDelegate.qml";
                    }
                }
            }

            model: conv.messages
        }

        PlasmaComponents.ScrollBar {
            id: viewScrollBar
            anchors {
                top: parent.top
                bottom: parent.bottom
                right: parent.right
            }

            flickableItem: view
            interactive: true
            width: 16
            opacity: 1
            orientation: Qt.Vertical
        }


        //used states here to make the scroll bar (dis)appear
        states: [
            State {
                name: "auto-scrolling"
                when: view.atYEnd
                PropertyChanges {
                    target: view.model
                    restoreEntryValues: true
                    onRowsInserted: {
                        view.positionViewAtEnd();
                    }
                }
                PropertyChanges {
                    target: viewScrollBar
                    restoreEntryValues: true
                    width: 0
                    opacity: 0
                }
            }
        ]

        transitions: [
            Transition {
                from: "*"
                to: "auto-scrolling"

                PropertyAnimation {
                    properties: "width,opacity"
                    duration: 250
                }
            },
            Transition {
                from: "auto-scrolling"
                to: "*"

                PropertyAnimation {
                    properties: "width,opacity"
                    duration: 250
                }
            }
        ]
    }

    PlasmaWidgets.LineEdit {
        id: input
        //FIXME: replace with Plasma Component and focus
//         focus: true

        anchors.left: parent.left; anchors.right: parent.right
        anchors.bottom: parent.bottom

        onReturnPressed: {
            view.model.sendNewMessage(text);
            text = "";
        }
    }
}