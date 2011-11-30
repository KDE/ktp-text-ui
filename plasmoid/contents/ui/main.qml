import Qt 4.7
import org.kde.telepathy.declarativeplugins 0.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets

Item {
    ListView {
        id: base
        anchors.fill: parent
        orientation: Qt.Horizontal

        model: convos
        delegate : Item {
            PlasmaWidgets.SvgWidget {
                elementID: "widgets/frame.svg"
                anchors.fill: parent
            }
            PlasmaWidgets.IconWidget {
                text: model.conversation.target.nick
                icon: model.conversation.target.presenceIcon
                anchors.fill: parent
            }
        }
    }

    ConversationsModel {
        id: convos
    }
}