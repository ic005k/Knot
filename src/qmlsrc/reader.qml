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

    function getBookmarkText() {
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

    function setTextAreaCursorPos(nCursorPos) {
        textArea.cursorPosition = nCursorPos
    }

    function handleLinkClicked(link) {
        console.log("Clicked link:", link)
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

            //renderType: Text.NativeRendering // 使用原生渲染引擎,在Qt6.6.3的安卓上表现迟钝，非常容易导致崩溃
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
