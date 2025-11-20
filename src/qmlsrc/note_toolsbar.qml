import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    height: 40
    width: 300
    color: isDark ? "#19232D" : "#f0f0f0"
    border.color: "#cccccc"
    border.width: 0

    // 定义图标数组，避免重复代码
    property var lightIcons: [
        "qrc:/res/back.svg", "qrc:/res/view.svg", "qrc:/res/edit.svg",
        "qrc:/res/find.svg", "qrc:/res/up.svg", "qrc:/res/down.svg",
        "qrc:/res/delitem.svg", "qrc:/res/move.svg", "qrc:/res/topdf.svg",
        "qrc:/res/rename.svg", "qrc:/res/recycle.svg"
    ]

    property var darkIcons: [
        "qrc:/res/back_l.svg", "qrc:/res/view_l.svg", "qrc:/res/edit_l.svg",
        "qrc:/res/find_l.svg", "qrc:/res/up_l.svg", "qrc:/res/down_l.svg",
        "qrc:/res/delitem_l.svg", "qrc:/res/move_l.svg", "qrc:/res/topdf_l.svg",
        "qrc:/res/rename_l.svg", "qrc:/res/recycle_l.svg"
    ]

    Rectangle {
        id: rect
        height: 40
        border.color: "#cccccc"
        border.width: 0
        width: Math.min(buttonRow.implicitWidth, root.width)
        anchors.horizontalCenter: root.horizontalCenter
        anchors.verticalCenter: root.verticalCenter
        color: "transparent"

        Flickable {
            id: flick
            anchors.fill: rect
            flickableDirection: Flickable.HorizontalFlick
            contentWidth: buttonRow.implicitWidth
            contentHeight: root.height

            Row {
                id: buttonRow
                y: (rect.height - height) / 2
                spacing: 10
                padding: 10

                Repeater {
                    model: 11
                    ToolButton {
                        id: toolButton
                        width: 38
                        height: 38
                        icon.width: 25
                        icon.height: 25
                        // 根据主题选择图标
                        icon.source: isDark ? darkIcons[index] : lightIcons[index]
                        // 显式设置图标颜色，避免系统自动着色
                        icon.color: isDark ? "white" : "black"

                        background: Rectangle {
                            color: toolButton.hovered ? (isDark ? "#333333" : "#e0e0e0") : "transparent"
                            radius: 6
                        }

                        onClicked: {
                            console.log("Button", index + 1, "clicked")
                            switch (index) {
                            case 0:
                                mw_one.on_btnBackNoteList_clicked()
                                break
                            case 1:
                                mw_one.on_btnOpenNote_clicked()
                                break
                            case 2:
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
                    let newX = flick.contentX - event.angleDelta.y / 3
                    newX = Math.max(0, Math.min(newX, flick.contentWidth - flick.width))
                    flick.contentX = newX
                    event.accepted = true
                }
            }
        }
    }
}
