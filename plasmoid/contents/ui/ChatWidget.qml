import Qt 4.7
import org.kde.telepathy.declarativeplugins 0.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets
import org.kde.plasma.components 0.1 as PlasmaComponents

Item {
    property Conversation conv

    signal closeRequested
    signal conversationEndRequested

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
                right: closeButton.left
                bottom: parent.bottom
            }

            text: conv.target.nick
            iconSource: conv.target.presenceIconSource

            onClicked: closeRequested()
        }

        PlasmaComponents.ToolButton {
            id: closeButton

            anchors {
                top: parent.top
                right: parent.right
                bottom: parent.bottom
            }

            iconSource: "dialog-close"

            onClicked: conversationEndRequested()
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

        anchors.top: space.bottom
        anchors.left: parent.left; anchors.right: parent.right
        anchors.bottom: input.top
        anchors.margins: 5

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

            delegate: TextDelegate {}
            model: conv.model
        }

        PlasmaComponents.ScrollBar {
            id: viewScrollBar
            anchors {
                top: parent.top
                bottom: parent.bottom
                right: parent.right
            }

            flickableItem: view
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


        anchors.left: parent.left; anchors.right: parent.right
        anchors.bottom: parent.bottom

        onReturnPressed: {
            view.model.sendNewMessage(text);
            text = "";
        }
    }
}