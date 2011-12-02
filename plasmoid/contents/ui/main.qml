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
//             PlasmaWidgets.SvgWidget {
//                 elementID: "widgets/frame.svg"
//                 anchors.fill: parent
//             }
            PlasmaWidgets.IconWidget {
                id: icon
                text: model.conversation.target.nick
                icon: model.conversation.target.presenceIcon
                anchors.fill: parent

//                 Component {
//                     id: chatComp

//                 }
                property Component popout: PlasmaCore.Dialog {
                        mainItem: ChatWidget {
                            width: 200
                            height: 400
                            conv: model.conversation
                        }
                    }

                onClicked: {
                    var dialog = popout.createObject(base, {});
                    console.log(dialog);
                    var point = dialog.popupPosition(icon, Qt.AlignTop);
                    console.log("Showing dialog at (" + point.x + "," + point.y + ")");
                    dialog.x = point.x;
                    dialog.y = point.y;
                    dialog.animatedShow(PlasmaCore.Up);
                }
            }
        }
    }

    ConversationsModel {
        id: convos
    }
}