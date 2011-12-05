import Qt 4.7
import org.kde.telepathy.declarativeplugins 0.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets
import org.kde.plasma.components 0.1 as PlasmaComponents
// import "createDialog.js" as MyScript

Item {
//     PlasmaCore.SvgItem {
//         elementId: "horizzontal-line"
//         anchors.fill:parent
//         svg: PlasmaCore.Svg {
//             id: mySvg
//             imagePath: "widgets/line"
//         }
//     }

//     PlasmaCore.FrameSvgItem {
//         id: surface
// 
//         anchors.fill: parent
// //         prefix: (internal.userPressed || checked) ? "pressed" : "normal"
//         //internal: if there is no hover status, don't paint on mouse over in touchscreens
// //         opacity: (internal.userPressed || checked || !flat || (shadow.hasOverState && mouse.containsMouse)) ? 1 : 0
// //         Behavior on opacity {
// //             PropertyAnimation { duration: 100 }
// //         }
//     }
//     PlasmaComponents.ListItem {
//         anchors.fill: parent
//         enabled: true
//         sectionDelegate: true
//     }
    ListView {
        id: base
        anchors.fill: parent
        orientation: Qt.Horizontal

        model: ConversationsModel {
        }

        delegate : ConversationDelegate {
            anchors.top: parent.top
//             height: 60
            anchors.bottom: parent.bottom
//             anchors.margins: 5
        }
    }

//     height: parent.height
//     width: 60
}