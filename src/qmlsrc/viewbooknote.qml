import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Fusion

Rectangle {
    id: root
    width: 400
    height: 600
    color: "#f0f0f0"

    property int cIndex: -1
    property int nPagesIndex: -1
    property int cPage: -1
    property string strNoteText: ""

    function initValue(cindex, cpage) {
        cIndex = cindex
        cPage = cpage
    }

    function setText2(text) {
        text2.text = text
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // 列表视图 - 使用Layout.fillHeight使其占据剩余空间
        ListView {
            id: listView
            model: notesModel
            Layout.fillWidth: true
            Layout.fillHeight: true // 关键：填充剩余高度

            boundsBehavior: Flickable.StopAtBounds // 禁止滚动到边界外的弹性效果

            delegate: Rectangle {
                id: itemDelegate
                width: listView.width
                height: column.implicitHeight + 6
                color: index % 2 === 0 ? "#ffffff" : "#f9f9f9"

                Rectangle {
                    color: "#FF0000"
                    radius: 0
                    width: 5
                    height: parent.height

                    opacity: cIndex === index ? 1 : 0
                    Behavior on opacity {
                        NumberAnimation {
                            duration: 200
                        } // 200毫秒平滑过渡
                    }
                }

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
                            font.pointSize: fontSize
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
                            font.pointSize: fontSize
                            color: "#333333"
                            wrapMode: Text.WordWrap
                        }
                    }

                    Text {
                        id: text2
                        width: parent.width
                        text: model.content
                        font.pointSize: fontSize
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

                    onClicked: {

                        listView.currentIndex = index
                        cIndex = index
                        cPage = model.page
                        strNoteText = model.content
                        nPagesIndex = model.pageIndex
                        btnedit.enabled = true
                        console.log("点击了条目:", cIndex, cPage, nPagesIndex,
                                    strNoteText)
                    }
                }
            }

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
                width: 8
            }
        }

        // 按钮 - 现在会固定在底部
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: 10
            Layout.topMargin: 10
            spacing: 15

            Button {
                id: btnedit
                enabled: false
                text: qsTr("Edit")
                font.pointSize: fontSize

                onClicked: {

                    if (cIndex >= 0) {
                        console.log("开始编辑条目:", nPagesIndex, cPage, strNoteText)
                        m_Reader.editBookNote(nPagesIndex, cPage, strNoteText)
                    } else {
                        console.log("请先选中一个条目再编辑")
                    }
                }
            }

            Button {
                text: qsTr("Close")
                font.pointSize: fontSize
                onClicked: {
                    m_Reader.closeViewBookNote()
                }
            }
        }
    }
}
