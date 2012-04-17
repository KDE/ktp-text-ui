import QtQuick 1.0
import org.kde.plasma.components 0.1 as PlasmaComponents

PlasmaComponents.Label {
    id: body
    wrapMode: Text.Wrap
    width: view.width

    text: model.text
    textFormat: Text.RichText
    height: paintedHeight

    onLinkActivated: {
        console.log("opening link: " + link);
        plasmoid.openUrl(link);
    }

    //Hover to display the time when hovering a message
    PlasmaComponents.Label {
        text: Qt.formatTime(model.time)
        anchors {
            top: parent.top
            right: parent.right
        }
        Rectangle {
            color: theme.backgroundColor
            anchors.fill: parent
            z: parent.z-1
            opacity: 0.8
            radius: 5
        }
        visible: mouseArea.containsMouse
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
    }
}