import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    width: 400
    height: 600
    color: "#f0f0f0"

    // 列表数据模型
    ListModel {
        id: listModel
        ListElement {
            name: "Item 1"
        }
        ListElement {
            name: "Item 2"
        }
        ListElement {
            name: "Item 3"
        }
        ListElement {
            name: "Item 4"
        }
        ListElement {
            name: "Item 5"
        }
    }

    // 列表视图
    ListView {
        id: listView
        model: notesModel // C++ 中暴露给 QML 的 QStandardItemModel
        anchors.fill: parent
        delegate: Rectangle {
            id: itemDelegate
            width: listView.width
            height: 50
            color: index % 2 === 0 ? "#ffffff" : "#f9f9f9"

            Text {
                anchors.centerIn: parent
                text: model.display
                font.pointSize: fontSize
                color: "#333333"

                // 超出宽度自动显示省略号
                width: parent.width - 20
                elide: Text.ElideRight
            }

            // 分隔线
            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 1
                color: "#e0e0e0"
            }

            // 点击事件
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    console.log("点击了条目:", index, model.display)


                }
            }
        }
        ScrollBar.vertical: ScrollBar {}
    }

    Button {
        text: qsTr("Close")
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: 10
        onClicked: {
            m_Reader.closeViewBookNote()
        }
    }
}
