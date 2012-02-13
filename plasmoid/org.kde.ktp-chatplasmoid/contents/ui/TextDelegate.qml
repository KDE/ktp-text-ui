import Qt 4.7
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.core 0.1 as PlasmaCore

Item {
    property color textColor: theme.textColor

    PlasmaComponents.Label {
        id: header

        width: view.width
        wrapMode: Text.Wrap

        color: textColor
        text: "<b>[" + Qt.formatTime(model.time) + "] " + model.user + " :</b>"

        verticalAlignment: Text.AlignBottom
    }
    PlasmaComponents.Label {
        id: body

        anchors.top: header.bottom
        width: view.width
        wrapMode: Text.Wrap

        color: textColor
        text: model.text
        textFormat: Text.RichText
        verticalAlignment: Text.AlignTop
    }

    height: header.height + body.height
}