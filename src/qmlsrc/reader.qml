import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import MyModel2 1.0
import EBook.Models 1.0

Item {
    id: root
    visible: true

    // 横屏控制变量
    property bool isLandscape: false

    // 原有属性
    property string strUrl: ""
    property string strText: ""
    property string pdfFile: ""
    property bool isPDF: false
    property bool isEPUBText: false

    // 原有功能函数（完整保留）
    function getText() {
        return m_text.text
    }
    function setText(str) {
        textModel.splitContent(str)
    }
    function loadText(str) {
        textModel.splitContent(str)
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
        textModel.splitContent(str)
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
        textModel.splitContent(strhtml)
        isEPUBText = true
    }
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
        let firstVisibleIndex = contentListView.indexAt(
                0, contentListView.contentY)
        if (firstVisibleIndex === -1)
            return "Bookmark"
        let delegateItem = contentListView.itemAtIndex(firstVisibleIndex)
        if (!delegateItem)
            return "Bookmark"
        let relY = contentListView.contentY - delegateItem.y
        let pos = delegateItem.positionAt(0, relY)
        return delegateItem.text.substring(pos, pos + 100)
    }
    function getBookmarkText() {
        if (contentListView.count === 0)
            return "Bookmark"
        let firstVisibleIndex = contentListView.indexAt(
                0, contentListView.contentY + 1)
        if (firstVisibleIndex === -1)
            firstVisibleIndex = 0
        let firstVisibleItem = contentListView.itemAtIndex(firstVisibleIndex)
        if (firstVisibleItem) {
            let renderedPlainText = firstVisibleItem.Accessible.text
            if (typeof renderedPlainText === 'string'
                    && renderedPlainText.length > 0) {
                return renderedPlainText.substring(
                            0, Math.min(100, renderedPlainText.length))
            } else {
                console.warn("Accessible.text is empty, falling back to raw text")
                let rawText = firstVisibleItem.text
                if (rawText) {
                    let fallbackPlainText = rawText.replace(/<[^>]*>/g, '')
                    return fallbackPlainText.substring(
                                0, Math.min(100, fallbackPlainText.length))
                }
            }
        }
        console.log("Could not retrieve text from visible item")
        return "Bookmark"
    }
    function setTextAreaCursorPos(nCursorPos) {
        textArea.cursorPosition = nCursorPos
    }
    function handleLinkClicked(link) {
        document.setBackDir(link)
        document.parsingLink(link, "reader")
    }

    // 横屏模式变化处理
    onIsLandscapeChanged: {
        rotateContainer.width = isLandscape ? root.height : root.width
        rotateContainer.height = isLandscape ? root.width : root.height
        console.log("横屏模式更新 - 旋转容器尺寸:", rotateContainer.width, "x",
                    rotateContainer.height)
    }

    DocumentHandler {
        id: document
        objectName: "dochandler"
        onLoaded: {
            textModel.splitContent(text)
        }
        onError: {
            errorDialog.text = message
            errorDialog.visible = true
        }
    }

    // 旋转容器
    Rectangle {
        id: rotateContainer
        color: "transparent"
        width: isLandscape ? root.height : root.width
        height: isLandscape ? root.width : root.height

        // 旋转逻辑
        transform: [
            Rotation {
                angle: isLandscape ? 90 : 0
                origin.x: rotateContainer.width / 2
                origin.y: rotateContainer.height / 2
            },
            Translate {
                x: isLandscape ? (root.width - rotateContainer.width) / 2 : 0
                y: isLandscape ? (root.height - rotateContainer.height) / 2 : 0
            }
        ]

        // 背景图片
        Image {
            id: m_Image
            width: rotateContainer.width
            height: rotateContainer.height
            fillMode: Image.Stretch
            source: backImgFile
            visible: backImgFile !== ""
        }

        // 背景色
        Rectangle {
            id: m_Rect
            width: rotateContainer.width
            height: rotateContainer.height
            color: myBackgroundColor
            visible: backImgFile === ""
        }

        // 内容列表
        ListView {
            id: contentListView
            width: rotateContainer.width
            height: rotateContainer.height
            spacing: 5
            cacheBuffer: 500
            model: TextChunkModel {
                id: textModel
            }
            clip: true

            delegate: Text {
                id: m_text
                width: rotateContainer.width
                leftPadding: 10
                rightPadding: 10
                textFormat: Text.RichText
                text: model.text || "测试文本：检查宽度是否填满"
                wrapMode: Text.Wrap
                font.pixelSize: FontSize
                font.family: FontName
                color: myTextColor
                renderType: Text.QtRendering

                onLinkActivated: handleLinkClicked(link)

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

    // 横屏切换按钮
    Button {
        text: isLandscape ? "切换竖屏" : "切换横屏"
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        visible: false
        onClicked: {
            isLandscape = !isLandscape
            console.log("QQuickWidget尺寸:", root.width, "x", root.height)
        }
    }
}
