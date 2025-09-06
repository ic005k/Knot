import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    height: 40
    width: 300
    color: "#f0f0f0"
    border.color: "#cccccc"
    border.width: 0

    Rectangle {
        id: rect
        height: 40

        border.color: "#cccccc"
        border.width: 0

        // 容器宽度：自适应内容（按钮组宽度），但不超过根节点宽度
        width: Math.min(buttonRow.implicitWidth, root.width)
        anchors.horizontalCenter: root.horizontalCenter
        anchors.verticalCenter: root.verticalCenter
        color: "transparent" // 透明，不影响外观

        Flickable {
            id: flick
            anchors.fill: rect
            flickableDirection: Flickable.HorizontalFlick
            // 内容宽度由按钮组实际宽度决定
            contentWidth: buttonRow.implicitWidth
            contentHeight: root.height

            Row {
                id: buttonRow
                // 仅保留垂直居中（相对于根节点）
                y: (rect.height - height) / 2
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

            WheelHandler {
                onWheel: event => {
                             // 计算新的滚动位置（带灵敏度控制）
                             let newX = flick.contentX - event.angleDelta.y / 3

                             // 限制滚动范围：不小于0，不大于最大可滚动距离（内容宽度 - 显示宽度）
                             newX = Math.max(
                                 0, Math.min(newX,
                                             flick.contentWidth - flick.width))

                             // 应用限制后的位置
                             flick.contentX = newX
                             event.accepted = true
                         }
            }
        }
    }
}
