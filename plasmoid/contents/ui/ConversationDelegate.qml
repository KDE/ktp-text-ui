import Qt 4.7
import org.kde.telepathy.declarativeplugins 0.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets
import org.kde.plasma.components 0.1 as PlasmaComponents

Item {
    PlasmaCore.FrameSvgItem {
        id: canvas

        prefix: "normal"
        imagePath: "widgets/tasks"
        opacity: 1
        anchors.fill: parent
//         anchors.margins: 5
    }
//     height: icon.height + 10
//     width: icon.width + 10
    width: height

    PlasmaCore.Dialog {
        id: dialog
        windowFlags: Qt.Dialog
        mainItem: ChatWidget {
            width: 250
            height: 350
            conv: model.conversation

            onCloseRequested: mouse.popupApplet()
        }
    }

    //ise listitem?
    PlasmaWidgets.IconWidget {
        id: icon
//         text: model.conversation.target.nick
        icon: model.conversation.target.presenceIcon
//         anchors {
//             top: parent.top
//             bottom: parent.bottom
// //             left: parent.left
//         }
        anchors.fill: parent
        anchors.margins: 5
//         width: height

//         size: "32x32"
//         size: {
//             console.log("height = " + parent.height);
//             console.log("width = " + parent.width);
//             return Qt.size(icon.height, icon.height);
//         }
//         Component.onCompleted: {
//             console.log("height = " + parent.height);
//             console.log("width = " + parent.width);
//         }
    }

    MouseArea {
        id: mouse
        anchors.fill: parent

        hoverEnabled: true

        //move le onClicked into main
        onClicked: popupApplet()
        function popupApplet() {
            if(dialog.visible == false) {
                var point = dialog.popupPosition(icon, Qt.AlignBottom);
                console.log("Showing dialog at (" + point.x + "," + point.y + ")");

                dialog.x = point.x;
                dialog.y = point.y;

                dialog.visible = true;
            } else {
                console.log("height = " + dialog.height);
                console.log("width = " + dialog.width);
                dialog.visible = false;
            }
        }
    }

    states: [
        State {
            name: "focus"
            //use property instead
            when: dialog.visible
            PropertyChanges {
                target: canvas
                prefix: "focus"
            }
        },
        State {
            name: "hover"
            when: mouse.containsMouse
            PropertyChanges {
                target: canvas
                prefix: "hover"
            }
        }
    ]
}