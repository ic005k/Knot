import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls.Fusion

import MyModel2 1.0
import EBook.Models 1.0

Rectangle {
    id: textitem
    visible: true
    width: myW
    height: myH

    property string strUrl: ""
    property string strText: ""
    property string pdfFile: ""
    property bool isPDF: false
    property bool isEPUBText: false

    function getText() {
        return m_text.text
    }

    function setText(str) {
        textModel.splitContent(str) // 调用C++处理
    }

    function loadText(str) {
        textModel.splitContent(str) // 调用C++处理
        isPDF = false
        isEPUBText = true
    }

    function loadHtml(htmlFile, skipID) {

        document.load("file://" + htmlFile)
        isPDF = false
        isEPUBText = true
        if (skipID !== "") {

        }
    }

    function loadHtmlStr(str) {

        textModel.splitContent(str) // 调用C++处理

        isPDF = false
        isEPUBText = true
    }

    function loadPDF(pdffile) {
        pdfFile = "file:///" + pdffile
        isPDF = true
        isEPUBText = false
        console.debug(pdfFile)
    }

    function loadHtmlBuffer(strhtml) {
        //document.loadBuffer(strhtml)
        textModel.splitContent(strhtml) // 调用C++处理
        isEPUBText = true
    }

    // 修改为基于 ListView 的滚动控制
    function setVPos(vpos) {
        contentListView.contentY = vpos
    }

    function getVPos() {
        return contentListView.contentY
    }

    function getVHeight() {
        return contentListView.contentHeight
    }

    function getSelectedText() {
        console.log("selectedText=" + textArea.selectedText)
        return textArea.selectedText
    }

    function move(x0) {
        x = x + x0
        file.curX = x
        console.log(file.curX)
    }

    function setX(x0) {
        x = 0
    }

    function getBookmarkText_() {
        // 基于当前可视区域的首个可见项计算
        let firstVisibleIndex = contentListView.indexAt(
                0, contentListView.contentY)
        if (firstVisibleIndex === -1)
            return "Bookmark"

        let delegateItem = contentListView.itemAtIndex(firstVisibleIndex)
        if (!delegateItem)
            return "Bookmark"

        // 获取该段落中坐标对应的字符位置
        let relY = contentListView.contentY - delegateItem.y
        let pos = delegateItem.positionAt(0, relY)
        return delegateItem.text.substring(pos, pos + 100) // 取前100字符
    }

    function getBookmarkText() {
        // 检查 ListView 是否有内容
        if (contentListView.count === 0) {
            return "Bookmark";
        }

        // 1. 找到第一个可见的 delegate 项的索引
        // 使用 contentY + 1 来稍微向下一点，确保能获取到正在进入视图的项
        let firstVisibleIndex = contentListView.indexAt(0, contentListView.contentY + 1);

        // 2. 如果 indexAt 没找到（可能在列表顶部），则默认为索引 0
        if (firstVisibleIndex === -1) {
            firstVisibleIndex = 0;
        }

        // 3. 使用索引获取对应的 delegate 项实例
        let firstVisibleItem = contentListView.itemAtIndex(firstVisibleIndex);

        // 4. 检查是否成功获取到了 delegate 项
        if (firstVisibleItem) {
            // 5. 确保 delegate 项是 Text 类型并且已加载内容
            //    假设 delegate 的根元素就是 Text { id: m_text }
            let delegateTextItem = firstVisibleItem;

            // 6. 关键：使用 Accessible.text 获取渲染后的纯文本
            //    这会自动处理 HTML 标签
            let renderedPlainText = delegateTextItem.Accessible.text;

            // 7. 检查获取到的文本是否存在且非空
            if (typeof renderedPlainText === 'string' && renderedPlainText.length > 0) {
                // 8. 返回前100个字符
                return renderedPlainText.substring(0, Math.min(100, renderedPlainText.length));
            } else {
                 // 如果 Accessible.text 暂时不可用或为空，尝试回退到原始 text 并简单处理
                 // 这种情况较少见，但可以作为一种后备方案
                 console.warn("Accessible.text is empty or not available, falling back to raw text processing for item at index:", firstVisibleIndex);
                 let rawText = delegateTextItem.text;
                 if (rawText) {
                     let fallbackPlainText = rawText.replace(/<[^>]*>/g, '');
                     return fallbackPlainText.substring(0, Math.min(100, fallbackPlainText.length));
                 }
            }
        }

        // 如果以上方法都失败，则返回默认值
        console.log("Could not retrieve text from the first visible item.");
        return "Bookmark";
    }

    function setTextAreaCursorPos(nCursorPos) {
        textArea.cursorPosition = nCursorPos
    }

    function handleLinkClicked(link) {
        //console.log("Clicked link:", link)
        document.setBackDir(link)
        document.parsingLink(link, "reader")
        // 如果需要保持滚动位置，可在此处记录位置
    }

    DocumentHandler {
        id: document
        objectName: "dochandler"

        onLoaded: {

            textModel.splitContent(text) // 调用C++处理
        }
        onError: {
            errorDialog.text = message
            errorDialog.visible = true
        }
    }

    Image {
        id: m_Image
        width: myW
        height: myH
        fillMode: Image.Tile
        horizontalAlignment: Image.AlignLeft
        verticalAlignment: Image.AlignTop

        smooth: true
        source: backImgFile
        visible: backImgFile === "" ? false : true
    }

    Rectangle {
        id: m_Rect
        width: myW
        height: myH
        color: myBackgroundColor
        visible: backImgFile === "" ? true : false
    }

    ListView {
        id: contentListView
        width: parent.width
        anchors.fill: parent
        spacing: 5 // 段落间距
        cacheBuffer: 500 // 预加载区域

        // 声明 C++ 模型
        model: TextChunkModel {
            id: textModel
        }

        clip: true
        delegate: Text {
            id: m_text
            width: m_Rect.width - 10 // 留出滚动条空间
            leftPadding: 10
            textFormat: Text.RichText // 必须启用富文本
            text: model.text
            wrapMode: Text.Wrap
            font.pixelSize: FontSize
            font.family: FontName
            color: myTextColor

            renderType: Text.QtRendering // 比NativeRendering更轻量

            // 处理链接点击
            onLinkActivated: function (link) {
                handleLinkClicked(link)
            }

            PropertyAnimation on x {
                easing.type: Easing.Linear
                running: isAni
                from: aniW
                to: 0
                duration: 200
                loops: 1
            }
        }

        ScrollBar.vertical: ScrollBar {
            id: vbar
            policy: ScrollBar.AsNeeded
            interactive: false // 关键！禁止拖动操作
            width: 10

            // 关键：size 绑定到可视区域比例
            size: contentListView.visibleArea.heightRatio
            // 可选：设置滑块最小尺寸（避免过小）
            minimumSize: 0.1

            // 动态显隐控制
            visible: opacity > 0

            //用于某个view中，如果没有则忽略
            //opacity: view.isScrolling ? 1 : 0
            Behavior on opacity {
                NumberAnimation {
                    duration: 300
                }
            }

            // 极简样式
            contentItem: Rectangle {
                color: isDark ? "#3498db" : "#606060"
                opacity: vbar.active ? (isDark ? 0.8 : 0.7) : 0
                Behavior on opacity {
                    NumberAnimation {
                        duration: 200 // 更流畅的动画
                        easing.type: Easing.OutQuad
                    }
                }
                radius: 3
            }

            background: null // 彻底消除背景容器
        }
    }
}
