import QtQuick 6.2
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    height: 40
    width: 300
    color: "#f0f0f0"
    border.color: "#cccccc"
    border.width: 1

    Flickable {
        id: flick
        anchors.fill: root
        flickableDirection: Flickable.HorizontalFlick
        // 内容宽度由按钮组实际宽度决定
        contentWidth: buttonRow.implicitWidth
        contentHeight: root.height

        Row {
            id: buttonRow
            // 仅保留垂直居中（相对于根节点）
            y: (root.height - height) / 2
            // 取消所有水平居中相关设置，默认左对齐
            spacing: 10
            padding: 10 // 左侧保留一定边距，避免贴边

            Repeater {
                model: 11
                ToolButton {
                    id: toolButton
                    width: 38
                    height: 38
                    icon.width: 25
                    icon.height: 25
                    icon.source: ["qrc:/res/back.png", "qrc:/res/view.svg", "qrc:/res/edit.svg", "qrc:/res/find.png", "qrc:/res/up.svg", "qrc:/res/down.svg", "qrc:/res/delitem.svg", "qrc:/res/move.svg", "qrc:/res/topdf.svg", "qrc:/res/rename.svg", "qrc:/res/recycle.svg"][index]

                    background: Rectangle {
                        color: toolButton.hovered ? "#e0e0e0" : "transparent"
                        radius: 6
                    }

                    onClicked: {
                        console.log("Button", index + 1, "clicked")
                        // 按钮点击逻辑
                        switch (index) {
                        case 0:
                            // 第1个按钮操作
                            mw_one.on_btnBackNoteList_clicked()
                            break
                        case 1:
                            // 第2个按钮操作
                            mw_one.on_btnOpenNote_clicked()
                            break
                        case 2:
                            // 第3个按钮操作
                            mw_one.on_btnEditNote_clicked()
                            break
                        case 3:
                            mw_one.on_btnShowFindNotes_clicked()
                            break
                        case 4:
                            mw_one.on_btnUpMove_clicked()
                            break
                        case 5:
                            mw_one.on_btnDownMove_clicked()
                            break
                        case 6:
                            mw_one.on_btnDelNote_NoteBook_clicked()
                            break
                        case 7:
                            mw_one.on_btnMoveTo_clicked()
                            break
                        case 8:
                            mw_one.on_btnToPDF_clicked()
                            break
                        case 9:
                            mw_one.on_btnRename_clicked()
                            break
                        case 10:
                            mw_one.on_btnNoteRecycle_clicked()
                            break
                        }
                    }
                }
            }
        }
    }
}
