import QtQuick 1.1
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1 as ExtraComponents

PlasmaComponents.ToolButton {
    id: base
    width: height

    property alias image: icon.icon
    property alias overlayText: text.text
    checkable: true

    ExtraComponents.QIconItem {
        id: icon
        anchors {
            fill: parent
            margins: 5
        }
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
            text: "0"
            color: "white"

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        visible: base.overlayText != "0"
    }
}