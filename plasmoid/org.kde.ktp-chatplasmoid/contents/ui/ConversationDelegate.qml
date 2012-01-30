import QtQuick 1.1
import org.kde.telepathy.declarativeplugins 0.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets
import org.kde.plasma.components 0.1 as PlasmaComponents

Item {
    id:base
    width: height

    property alias image: icon.icon
    property alias text: icon.text
    property bool pressed: false
    property string overlayText: "0"

    signal toggled

    PlasmaCore.FrameSvgItem {
        id: canvas

        prefix: "normal"
        imagePath: "widgets/tasks"
        opacity: 1
        anchors.fill: parent
    }

    PlasmaWidgets.IconWidget {
        id: icon

        anchors.fill: parent
        anchors.margins: 5

        orientation: Qt.Horizontal
    }

    Item {
        id: notification
        anchors {
            right: parent.right
            top: parent.top
        }

        width: parent.width / 3
        height: parent.height / 3

        Rectangle {
            id: background
            anchors.fill: parent
            color: "red"
            radius: 3
        }

        Text {
            id: text
            anchors.fill: parent

            font.pixelSize: parent.height
            text: base.overlayText
            color: "white"

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        visible: base.overlayText != "0"
    }

    MouseArea {
        id: mouse
        anchors.fill: parent

        hoverEnabled: true
        preventStealing: true

        onClicked: toggle();
    }

    function toggle() {
        base.pressed = !base.pressed;
        base.toggled();
    }

    states: [
        State {
            name: "pressed"
            when: base.pressed
            PropertyChanges {
                target: canvas
                prefix: "focus"
            }
        },
        State {
            name: "hover"
            when: mouse.containsMouse
            PropertyChanges {
                target: canvas
                prefix: "hover"
            }
        }
    ]
}