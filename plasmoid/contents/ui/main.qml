import Qt 4.7
import org.kde.telepathy.declarativeplugins 0.1 as KTelepathy
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets

Item {
    id:main


    function derp() {
        console.log("deeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeerp!");
        return true;
    }

    Item {
        id:chatArea

        anchors.top: parent.top
        anchors.left: parent.left; anchors.right: parent.right
        anchors.bottom: input.top
        anchors.margins: 5

        ListView {
            id: view

            anchors.fill: parent
            clip: true

            delegate: TextDelegate {
            }
            ListView.onAdd: {
                derp();
            }
        }

        //used states here because it'll make a scrollbar (dis)appear later on
        states: [
            State {
                name: "static"
            },
            State {
                name: "auto-scrolling"
                PropertyChanges {
                    target: view.model
                    restoreEntryValues: true
                    onRowsInserted: {
                        view.positionViewAtEnd();
                    }
                }
            }
        ]
    }

    PlasmaWidgets.LineEdit {
        id: input

//         anchors.top: view.bottom
        anchors.left: parent.left; anchors.right: parent.right
        anchors.bottom: parent.bottom

        onReturnPressed: {
            view.model.sendNewMessage(text);
            text = "";
        }
    }

    KTelepathy.ConversationWatcher {
        id:watcher
        onNewConversation: {
            console.log("New Convo!");
            view.model = con.model;
//             view.model.rowsInserted.connect(view.positionViewAtEnd);
            chatArea.state = "auto-scrolling";
        }
    }
}