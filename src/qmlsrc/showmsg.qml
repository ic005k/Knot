import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Fusion

Rectangle {
    id: root
    color: isDark ? "#19232D" : "white"

    Flickable {
        anchors.fill: parent
        clip: true
        // 固定内容宽度和可视区域一致，杜绝横向滚动
        contentWidth: width
        contentHeight: textItem.implicitHeight
        // 禁用水平拖动
        flickableDirection: Flickable.VerticalFlick

        TextEdit {
            id: textItem
            width: parent.width // 绑定Flickable宽度，不要绑定root
            text: textContent
            wrapMode: Text.WordWrap
            textFormat: Text.RichText
            readOnly: true
            color: isDark ? "#f0f0f0" : "#222222"
        }

        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
            width: 8
        }
    }
}
