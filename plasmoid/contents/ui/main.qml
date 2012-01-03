import Qt 4.7
import org.kde.telepathy.declarativeplugins 0.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets
import org.kde.plasma.components 0.1 as PlasmaComponents
// import "createDialog.js" as MyScript

Item {

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
//             text: model.conversation.target.nick
            overlayText: model.conversation.model.unreadCount

            pressed: dialog.visible
            onClicked: popupApplet();

            //FIXME: put in a loader to not slow down the model
            PlasmaCore.Dialog {
                id: dialog
                windowFlags: Qt.Dialog
                mainItem: ChatWidget {
                    width: 250
                    height: 350
                    conv: model.conversation

                    visible: model.conversation.model.visibleToUser

                    onCloseRequested: conv.popupApplet()
                    onConversationEndRequested: {
                        model.conversation.model.printallmessages();
                    }
                }
            }

            Connections {
                target: model.conversation
                onPopoutRequested: popupApplet();
            }

            function popupApplet() {
                if(dialog.visible == false) {
                    var point = dialog.popupPosition(conv, Qt.AlignBottom);
                    console.log("Showing dialog at (" + point.x + "," + point.y + ")");

                    dialog.x = point.x;
                    dialog.y = point.y;

                    dialog.visible = true;
                    model.conversation.model.visibleToUser = true;
                } else {
                    console.log("height = " + dialog.height);
                    console.log("width = " + dialog.width);
                    dialog.visible = false;
                    model.conversation.model.visibleToUser = false;
                }
            }
        }
    }

    function derp() {
        console.log("deeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeerrrrrrrrrrrrrrrrp!");
    }

//     height: parent.height
//     width: 60
}