import Qt 4.7

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