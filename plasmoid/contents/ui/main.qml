import Qt 4.7
import org.kde.telepathy.declarativeplugins 0.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets
import org.kde.plasma.components 0.1 as PlasmaComponents
// import "createDialog.js" as MyScript

Item {
    id: top
    ListView {
        id: base
        anchors.fill: parent
        orientation: Qt.Horizontal

        model: ConversationsModel {
        }

        delegate : ConversationDelegate {
            id:conv
            anchors.top: parent.top
            anchors.bottom: parent.bottom

            image: model.conversation.target.avatar
            overlayText: model.conversation.messages.unreadCount

            //FIXME: put in a loader to not slow down the model
            PlasmaCore.Dialog {
                id: dialog
                windowFlags: Qt.Dialog
                visible: conv.pressed

                mainItem: ChatWidget {
                    width: 250
                    height: 350
                    conv: model.conversation

                    onCloseRequested: {
                        conv.pressed = false;
                    }
                    onConversationEndRequested: {
                        model.conversation.messages.printallmessages();
                    }
                    Binding {
                        target: model.conversation.messages
                        property: "visibleToUser"
                        value: dialog.visible
                    }
                }

            }

            Connections {
                target: model.conversation.messages
                onPopoutRequested: {
                    conv.pressed = true;
                }
            }

            onToggled: {
                if(pressed) {
                    var point = dialog.popupPosition(conv, Qt.AlignBottom);
                    console.log("Showing dialog at (" + point.x + "," + point.y + ")");

                    dialog.x = point.x;
                    dialog.y = point.y;
                }
            }

//             function popupApplet() {
//                 if(model.conversation.messages.visibleToUser == false) {
//                     var point = dialog.popupPosition(conv, Qt.AlignBottom);
//                     console.log("Showing dialog at (" + point.x + "," + point.y + ")");
// 
//                     dialog.x = point.x;
//                     dialog.y = point.y;
// 
// //                     dialog.visible = true;
//                     model.conversation.messages.visibleToUser = true;
//                 } else {
//                     console.log("height = " + dialog.height);
//                     console.log("width = " + dialog.width);
// //                     dialog.visible = false;
//                     model.conversation.messages.visibleToUser = false;
//                 }
//             }
        }
    }

//     height: parent.height
//     width: 60
}