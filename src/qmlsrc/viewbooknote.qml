import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Fusion

Rectangle {
    id: root
    width: 400
    height: 600
    color: "#f0f0f0"

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // 列表视图 - 使用Layout.fillHeight使其占据剩余空间
        ListView {
            id: listView
            model: notesModel
            Layout.fillWidth: true
            Layout.fillHeight: true // 关键：填充剩余高度

            delegate: Rectangle {
                id: itemDelegate
                width: listView.width
                height: column.implicitHeight + 6
                color: index % 2 === 0 ? "#ffffff" : "#f9f9f9"

                Column {
                    id: column
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.margins: 10
                    spacing: 6

                    Rectangle {
                        color: "#8000FF00"
                        radius: 4
                        width: text0.implicitWidth + 8
                        height: text0.implicitHeight + 4

                        Text {
                            id: text0
                            width: parent.width
                            text: qsTr("Pages:") + model.page
                            font.pointSize: 10
                            color: "#333333"
                            wrapMode: Text.WordWrap
                        }
                    }

                    Rectangle {
                        id: rect1
                        width: parent.width
                        height: text1.implicitHeight + 8
                        color: "#f0f0f0"
                        radius: 4

                        Text {
                            id: text1
                            anchors.fill: parent
                            anchors.margins: 4
                            text: model.quote
                            font.pointSize: 10
                            color: "#333333"
                            wrapMode: Text.WordWrap
                        }
                    }

                    Text {
                        id: text2
                        width: parent.width
                        text: model.content
                        font.pointSize: 10
                        color: "#333333"
                        wrapMode: Text.WordWrap
                    }
                }

                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: 1
                    color: "#e0e0e0"
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: console.log("点击了条目:", index, model.page)
                }
            }

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
                width: 8
            }
        }

        // 按钮 - 现在会固定在底部
        Button {
            text: qsTr("Close")
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: 10
            Layout.topMargin: 10

            onClicked: {
                m_Reader.closeViewBookNote()
            }
        }
    }
}
