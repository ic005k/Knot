import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Fusion

Rectangle {
    id: root
    objectName: "diffHtmlRoot"
    width: 300
    //height: parent.height // 必须绑定高度，避免滚动范围异常
    color: "#ffffff"
    border.color: "#eee"
    border.width: 1

    property string htmlContent: ""

    ScrollView {
        id: scrollView
        anchors.fill: parent
        // 1. Qt6 中禁用水平滚动条的正确方式（通过子控件 ScrollBar.horizontal）
        //ScrollBar.horizontal.policy: ScrollBar.Off // 彻底关闭水平滚动条
        // 2. 固定内容宽度 = ScrollView 可用宽度（无水平滚动空间）
        contentWidth: scrollView.availableWidth // availableWidth 是扣除边距后的实际宽度
        // 3. 内边距（避免内容贴边，且不影响宽度计算）
        anchors.margins: 10

        Text {
            id: htmlDisplay
            // 5. 用锚定布局强制宽度 = ScrollView 内容宽度（无溢出可能）
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 10
            anchors.rightMargin: 10

            // 7. 文本内容与格式设置
            text: root.htmlContent
            textFormat: Text.RichText
            wrapMode: Text.Wrap // 自动换行（核心：确保文字不横向溢出）
            color: "#333333"
            renderType: Text.QtRendering
            verticalAlignment: Text.AlignTop // 顶部对齐，符合阅读习惯
        }
    }

    // 空内容提示
    Text {
        visible: root.htmlContent === ""
        text: qsTr("No differences")
        color: "#999999"
        anchors.centerIn: parent
    }
}
