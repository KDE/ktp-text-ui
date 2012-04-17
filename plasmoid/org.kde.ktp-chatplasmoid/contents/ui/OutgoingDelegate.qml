import QtQuick 1.0

TextDelegate {
    Rectangle {
        color: theme.viewBackgroundColor
        anchors.fill: parent
        z: parent.z-1
        opacity: 0.7
    }
}