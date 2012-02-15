import Qt 4.7
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.telepathy.chat 0.1

Item {
    property color textColor: "black"

    PlasmaComponents.Label {
        id: header

        width: view.width
        wrapMode: Text.Wrap

        color: textColor
        text: "<b>[" + Qt.formatTime(model.time) + "] " + model.user + " :</b>"

        verticalAlignment: Text.AlignBottom

        visible: !model.continuing
        Component.onCompleted: {
            if(model.continuing) {
                height = 0;
            }
        }
    }
    PlasmaComponents.Label {
        id: body

        anchors.top: header.bottom
        width: view.width
        wrapMode: Text.Wrap

        color: textColor
        text: model.text
        textFormat: Text.RichText
        height: paintedHeight

        onLinkActivated: {
            console.log("opening link: " + link);
            plasmoid.openUrl(link);
        }
    }

    height: header.height + body.height
}