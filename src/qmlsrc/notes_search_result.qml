import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ListView {
    id: listView
    anchors.fill: parent
    clip: true
    spacing: 10 // 增加列表项间距
    model: searchResultModel

    property string mdFile: ""

    function getQmlCurrentMDFile() {
        return mdFile
    }

    delegate: ItemDelegate {
        width: listView.width
        hoverEnabled: false

        // 动态高度：根据内容自动计算
        implicitHeight: layout.implicitHeight + 20 // 增加上下边距

        // 背景悬停效果
        background: Rectangle {
            //color: hovered ? "#f5f6fa" : "transparent"
            color: listView.currentIndex
                   === index ? "#f0f0f0" : (hovered ? "#e0e0e0" : "transparent") //#f5f6fa
            radius: 4
        }

        ColumnLayout {
            id: layout
            anchors.fill: parent
            anchors.margins: 8 // 整体边距
            spacing: 8 // 增加子项间距

            // 文件标题
            Text {
                //text: filePath.split("/").pop()
                text: fileTitle
                font.pointSize: fontSize
                color: "#2c3e50"
                elide: Text.ElideRight
                Layout.fillWidth: true
                bottomPadding: 4 // 标题下方留白
            }

            // 高亮预览文本
            Text {
                text: formatPreviewText(previewText, highlightPos)
                font.pointSize: fontSize - 1
                color: "#7f8c8d"
                wrapMode: Text.Wrap
                maximumLineCount: 3 // 允许显示更多行
                textFormat: Text.RichText
                Layout.fillWidth: true
                topPadding: 2 // 预览文本上方留白
                bottomPadding: 2 // 预览文本下方留白
            }
        }

        // 高亮格式化函数
        function formatPreviewText(previewText, highlightPositions) {
            let result = previewText
            let sortedPositions = highlightPositions.slice().sort(
                    (a, b) => b.charStart - a.charStart)
            sortedPositions.forEach(pos => {
                                        const start = pos.charStart
                                        const end = pos.charEnd
                                        if (start >= 0
                                            && end <= result.length) {
                                            result = result.slice(
                                                0,
                                                start) + `<span style="background: #fff3b1;color:#333">${result.slice(
                                                start,
                                                end)}</span>` + result.slice(
                                                end)
                                        }
                                    })
            return result
        }

        // 点击打开文件
        onClicked: {
            mdFile = filePath

            console.log("Open file:", filePath)
            listView.currentIndex = index
        }

        onDoubleClicked: {
            mdFile = filePath

            console.log("Open file:", filePath)
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
