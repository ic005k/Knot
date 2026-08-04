import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Fusion
import QtQuick.Layouts

ListView {
    id: noteListView
    anchors.fill: parent
    anchors.margins: 12
    clip: true
    spacing: 4
    model: proxyModel
    boundsBehavior: Flickable.StopAtBounds
    focus: true
    keyNavigationEnabled: true

    // ✅ Fusion 样式全局生效（需在 main.cpp 或 ApplicationWindow 中设置）
    // 若仅需局部生效，可在此处用 Component.onCompleted 动态切换

    property string currentSelectedUid: ""
    property bool isDark: false
    property var displayList: []

    ListModel { id: proxyModel }

    Connections {
        target: noteListView
        function onDisplayListChanged() {
            proxyModel.clear()
            for (var i = 0; i < displayList.length; i++)
                proxyModel.append(displayList[i])
        }
    }

    signal openNote(string noteUid)

    delegate: ItemDelegate {
        width: noteListView.width
        implicitHeight: layout.implicitHeight + 16
        leftPadding: 12
        rightPadding: 12

        readonly property bool isCurrent: ListView.isCurrentItem

        contentItem: ColumnLayout {
            id: layout
            spacing: 4

            Text {
                text: model.title
                font.bold: true
                elide: Text.ElideRight
                Layout.fillWidth: true
                color: isCurrent ? "#ffffff" : (isDark ? "#eeeeee" : "#212121")
            }

            Text {
                text: model.path
                font.italic: true
                elide: Text.ElideMiddle
                Layout.fillWidth: true
                color: isCurrent ? "#cccccc" : (isDark ? "#bbbbbb" : "#606060")
            }
        }

        background: Rectangle {
            radius: 6
            color: isCurrent ? (isDark ? "#111827" : "#757575") : "transparent"
            border.color: isCurrent ? "#616161" : "transparent"
            border.width: 1
        }

        onClicked: {
            noteListView.currentIndex = index
            noteListView.currentSelectedUid = model.uid
            noteListView.openNote(model.uid)
        }
    }

    Label {
        anchors.centerIn: parent
        visible: noteListView.count === 0
        text: qsTr("No matching notes")
        color: isDark ? "#999999" : "#777777"
    }

    ScrollBar.vertical: ScrollBar {
        policy: ScrollBar.AsNeeded
        width: 8
    }
}