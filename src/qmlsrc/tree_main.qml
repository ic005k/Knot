import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

TreeView {
    id: treeView

    anchors.fill: parent
    model: treeModel

    selectionMode: TreeView.SingleSelection
    selectionBehavior: TreeView.SelectRows
    focus: true

    // ================= delegate =================
    delegate: Rectangle {
        required property TreeView treeView
        required property bool hasChildren
        required property bool expanded
        required property var model

        implicitWidth: treeView.width
        implicitHeight: 32

        // ✅ 选中高亮
        color: treeView.selectionModel?.isSelected(model.index) ? "#e6f0ff" : "transparent"

        RowLayout {
            anchors.fill: parent
            spacing: 4

            // ▶ / ▼
            Text {
                visible: hasChildren
                text: expanded ? "▼" : "▶"
                width: 20
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                TapHandler {
                    onTapped: {
                        if (expanded)
                            treeView.collapse(model.index);
                        else
                            treeView.expand(model.index);
                    }
                }
            }

            // 文本
            Text {
                text: model.display ?? ""
                Layout.fillWidth: true
                verticalAlignment: Text.AlignVCenter
            }

            // ✅ 整行点击选中（不抢事件）
            TapHandler {
                onTapped: {
                    if (!treeView.selectionModel)
                        return;
                    treeView.selectionModel.select(model.index, ItemSelectionModel.ClearAndSelect);
                    treeView.forceActiveFocus();
                }
            }
        }
    }
}
