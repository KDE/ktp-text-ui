import Qt 4.7
import org.kde.telepathy.declarativeplugins 0.1 as KTelepathy

Item {
    id:main

    Component {
        id: textDelegate

        Item {
            Text {
                id: header
                width: view.width
                wrapMode: Text.Wrap

                text: "[" + Qt.formatTime(model.time) + "] " + model.user + " :"
            }
            Text {
                id: body

                anchors.top: header.bottom
                width: view.width

                wrapMode: Text.Wrap
                text: model.text
            }

            height: header.height + body.height
        }
    }

    ListView {
        id: view

        anchors.fill: parent
        anchors.margins: 5

        clip: true

        delegate: textDelegate
    }

    KTelepathy.ConversationWatcher {
        id:watcher
        onNewConversation: {
            console.log("New Convo!");
            view.model = con.model;
        }
    }
}