import QtQuick 1.1
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.telepathy.chat 0.1

PlasmaComponents.Label {
    text: "<i>* " + model.user + " " + model.text + "</i>"
}