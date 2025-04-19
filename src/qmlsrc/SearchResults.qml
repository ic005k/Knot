import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ListView {
    id: listView
    clip: true
    spacing: 12
    model: searchModel

    property string mdFile: ""

    function getQmlCurrentMDFile() {
        return mdFile
    }

    delegate: ItemDelegate {
        width: listView.width

        // 动态高度：根据内容自动计算
        implicitHeight: layout.implicitHeight + 20 // 增加上下边距

        leftPadding: 16
        rightPadding: 16

        // 新增选中状态标识属性
        property bool isCurrent: ListView.isCurrentItem

        contentItem: ColumnLayout {
            id:layout
            spacing: 4

            Text {
                id: text1
                text: model.title
                font.bold: true
                font.pointSize: fontSize
                elide: Text.ElideRight
                Layout.fillWidth: true
                color: isCurrent ? "white" : "#212121" // 选中时文字变白
            }

            Text {
                id: text2
                text: model.preview
                textFormat: Text.RichText
                wrapMode: Text.Wrap
                maximumLineCount: 2
                font.pointSize: fontSize - 1
                color: isCurrent ? "#eeeeee" : "#495057" // 选中时浅灰色文字
                Layout.fillWidth: true
            }

            Text {
                id: text3
                text: model.path
                font.italic: true
                font.pointSize: fontSize - 2
                color: isCurrent ? "#bdbdbd" : "#868e96" // 选中时浅灰色路径
                elide: Text.ElideMiddle
                Layout.fillWidth: true
            }
        }

        background: Rectangle {
            anchors.fill: parent
            color: isCurrent ? "#757575" : "transparent" // 中灰色背景
            radius: 6

            // 添加边框效果
            border.color: isCurrent ? "#616161" : "transparent"
            border.width: 1
        }

        // 点击选中处理
        onClicked: {

            mdFile = text3.text
            console.log("Open file:", text3.text)
            listView.currentIndex = index
        }

        onDoubleClicked: {

            mdFile = text3.text
            console.log("Open file:", text3.text)
            listView.currentIndex = index

            mw_one.on_btnOpenSearchResult_clicked()
        }
    }

    // 空状态提示
    Label {
        anchors.centerIn: parent
        visible: listView.count === 0
        text: qsTr("No results were found")
        color: "#bdc3c7"
        font.pixelSize: 18
    }

    // 滚动条
    ScrollBar.vertical: ScrollBar {
        policy: ScrollBar.AsNeeded
        width: 8
    }
}
