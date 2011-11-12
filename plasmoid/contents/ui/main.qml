import Qt 4.7
import org.kde.telepathy.declarativeplugins 0.1 as KTelepathy

Item {
    id:main

    ListView {
        id: view
        anchors.fill: parent
    }

    KTelepathy.ConversationWatcher {
        id:watcher
        onNewConversation: {
            console.log("SOMETHINGAHPPENDED!");
            console.log(con.model);
        }
    }
}