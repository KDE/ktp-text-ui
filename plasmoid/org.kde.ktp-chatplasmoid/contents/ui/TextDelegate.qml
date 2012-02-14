import Qt 4.7
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.telepathy.chat 0.1

Item {
    property color textColor: "black"

    Component.onCompleted: {
        if(model.type == MessagesModel.MessageTypeIncoming) {
            console.log("Type: MessagesModel::MessageTypeIncoming");
            textColor = theme.textColor;
        } else if(model.type == MessagesModel.MessageTypeOutgoing) {
            console.log("Type: MessagesModel::MessageTypeOutgoing");
            textColor = theme.highlightColor;
        } else {
            console.log("Unkown Type: " + model.type);
        }
    }

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
    }

    height: header.height + body.height
}