import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Fusion
import QtQuick.Layouts

ColumnLayout {
    anchors.fill: parent
    spacing: 0

    ListView {
        id: noteListView
        objectName: "noteListView"
        Layout.fillWidth: true
        Layout.fillHeight: true
        anchors.margins: 0
        clip: true
        spacing: 4
        model: proxyModel
        boundsBehavior: Flickable.StopAtBounds
        focus: true
        keyNavigationEnabled: true

        property string currentSelectedUid: ""
        property var displayList: []

        ListModel {
            id: proxyModel
        }

        Connections {
            target: noteListView
            function onDisplayListChanged() {
                proxyModel.clear();
                for (var i = 0; i < displayList.length; i++)
                    proxyModel.append(displayList[i]);
            }
        }

        // ✅ ListView 自身背景适配暗黑模式
        Rectangle {
            anchors.fill: parent
            z: -1
            radius: 0
            color: isDark ? "#1e1e1e" : "#ffffff"
        }

        delegate: ItemDelegate {
            id: noteDelegate
            width: noteListView.width
            implicitHeight: layout.implicitHeight + 16
            leftPadding: 0
            rightPadding: 0
            topPadding: 0
            bottomPadding: 0

            readonly property bool isCurrent: ListView.isCurrentItem

            // 🔑 核心：用 contentItem 包裹整个可视区域，完全替代 background
            contentItem: Rectangle {
                radius: 0
                color: isCurrent ? (isDark ? "#111827" : "#757575") : (isDark ? "#2d2d2d" : "#f5f5f5")
                border.color: isCurrent ? (isDark ? "#3b82f6" : "#616161") : "transparent"
                border.width: 1

                ColumnLayout {
                    id: layout
                    anchors.fill: parent
                    anchors.leftMargin: 8
                    anchors.rightMargin: 8
                    anchors.topMargin: 8
                    anchors.bottomMargin: 8
                    spacing: 4

                    Text {
                        id: mTitle
                        text: model.title
                        font.bold: true
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                        color: isCurrent ? "#ffffff" : (isDark ? "#eeeeee" : "#212121")
                    }

                    Text {
                        text: model.subtitle
                        font.italic: true
                        font.pixelSize: mTitle.font.pixelSize - 1
                        elide: Text.ElideMiddle
                        Layout.fillWidth: true
                        color: isCurrent ? "#cccccc" : (isDark ? "#bbbbbb" : "#606060")
                    }
                }
            }

            onClicked: {
                noteListView.currentIndex = index;
            }
        }

        Label {
            anchors.centerIn: parent
            visible: noteListView.count === 0
            text: qsTr("No content")
            color: isDark ? "#999999" : "#777777"
        }

        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
            width: 8
        }
    }

    // 底部按钮栏，不会遮挡ListView内容，ListView会自动让出底部空间
    RowLayout {
        Layout.fillWidth: true
        Layout.preferredHeight: 48
        spacing: 8

        Button {
            text: qsTr("Cancel")
            Layout.fillWidth: true
            onClicked: {
                // 取消按钮事件，外部可绑定或者在这里写逻辑
                mw_one.on_btnBackAIAPIList_clicked();
                console.log("cancel clicked");
            }
        }
        Button {
            text: qsTr("OK")
            Layout.fillWidth: true
            onClicked: {
                // 确定按钮事件，可以读取 noteListView.currentSelectedUid 获取选中项
                mw_one.on_btnAIAPIListOk_clicked(noteListView.currentIndex);
                console.log("ok clicked, selected uid:", noteListView.currentSelectedUid);
            }
        }
    }
}
