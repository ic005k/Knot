import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import MyModel2 1.0
import EBook.Models 1.0
import QtQuick.Controls.Fusion

Item {
    id: root
    visible: true

    // 翻页属性
    property point pressPos: Qt.point(0, 0)
    property bool isPressing: false
    property bool isMoving: false
    property int scrollThreshold: 75
    property int offsetLimit: 55 // 35
    property bool needSwipePage: false
    property bool swipeToNextPage: false // true=下一页，false=上一页

    property int touchDeadzone: 15 // 触摸死区15px，过滤手指抖动
    property int lastTouchUpdate: 0
    property int touchThrottle: 16 // 50ms 内最多更新一次

    // 横屏控制变量
    property bool isLandscape: false
    property bool isReadyEnd: false

    // 面积法所需变量
    property real prevContentY: 0 // 切换前的滚动位置
    property real prevContentHeight: 0 // 切换前的内容总高度
    property bool isSwitching: false // 标记正在切换方向

    // 原有属性
    property string strUrl: ""
    property string strText: ""
    property string pdfFile: ""
    property bool isPDF: false
    property bool isEPUBText: false

    function setLandscape(isValue) {
        isLandscape = isValue
    }

    function getReadyEnd() {
        return isReadyEnd
    }

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
        document.parsingLink(link, "m_Reader")
    }

    // 横屏模式变化处理（集成面积法核心逻辑）
    onIsLandscapeChanged: {
        isReadyEnd = false
        // 1. 切换前保存当前状态
        prevContentY = contentListView.contentY
        prevContentHeight = contentListView.contentHeight
        isSwitching = true

        // 2. 更新容器尺寸
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

            MouseArea {

                anchors.fill: parent
                acceptedButtons: Qt.LeftButton // 恢复接收左键，才能处理点击/双击/长按
                propagateComposedEvents: true // 把事件传递给子元素（delegate）

                onDoubleClicked: {

                    m_Reader.on_SetReaderFunVisible()
                }

                onPressAndHold: {
                    if (!isMoving)
                        mw_one.on_btnSelText_clicked()
                }

                onPressed: {
                    isPressing = true
                    isMoving = false
                    pressPos = Qt.point(mouse.x, mouse.y)
                    root.setX(0)
                }

                onPositionChanged: {
                    // 时间节流
                    var now = Date.now()
                    if (now - root.lastTouchUpdate < root.touchThrottle) {
                        return
                    }
                    root.lastTouchUpdate = now

                    var deltaX = mouse.x - root.pressPos.x
                    var deltaY = mouse.y - root.pressPos.y

                    // 触摸死区过滤（防止轻微抖动）
                    if (Math.abs(deltaX) < root.touchDeadzone && Math.abs(
                                deltaY) < root.touchDeadzone) {
                        return
                    }

                    // 判断释放有滑动的动作
                    if (Math.abs(deltaX) > Math.abs(35)) {

                        root.isMoving = true
                        root.updatePageIndicator(deltaX, deltaY,
                                                 mouse.x, mouse.y)
                        mouse.accepted = true
                    } else {

                        mouse.accepted = false
                    }

                    console.log(deltaX + "  " + deltaY)
                }

                onReleased: {
                    if (root.isMoving) {
                        root.handleSwipeGesture()
                    }

                    // 非滑动时，事件传递给delegate处理点击
                    mouse.accepted = !root.isMoving
                }

                onCanceled: {
                    isPressing = false
                    isMoving = false
                    pageIndicator.visible = false
                }
            }

            // 3. 布局更新后应用面积法计算新位置
            onContentHeightChanged: {
                if (isSwitching) {
                    // 确保内容高度有效
                    if (prevContentHeight > 0
                            && contentListView.contentHeight > 0) {
                        // 核心公式：新位置 = 旧比例 × 新总高度
                        const scrollRatio = prevContentY / prevContentHeight
                        let newContentY = scrollRatio * contentListView.contentHeight

                        // 边界处理：不超过可滚动范围
                        const maxY = Math.max(
                                       0,
                                       contentListView.contentHeight - contentListView.height)
                        newContentY = Math.min(newContentY, maxY)
                        newContentY = Math.max(newContentY, 0)

                        // 应用新位置
                        contentListView.contentY = newContentY
                        console.log(`面积法映射: 旧位置=${prevContentY}, 旧总高=${prevContentHeight}, 新总高=${contentListView.contentHeight}, 新位置=${newContentY}`)
                        isReadyEnd = true
                    }
                    isSwitching = false // 结束切换标记
                }
            }

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

                //onLinkActivated: handleLinkClicked(link)
                PropertyAnimation on x {
                    easing.type: Easing.Linear
                    running: isAni
                    from: aniW
                    to: 0
                    duration: 200
                    loops: 1
                }

                // ！！！delegate内部的MouseArea：处理点击链接、双击、长按
                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton // 只处理左键点击
                    propagateComposedEvents: false // 不传递事件（避免和全局冲突）

                    onClicked: {
                        // 只有“非滑动”时才处理点击
                        if (!root.isMoving) {
                            // ！！！这里能直接访问m_text（同delegate作用域）
                            var link = m_text.linkAt(mouse.x, mouse.y)
                            if (link) {
                                root.handleLinkClicked(link) // 调用全局的链接处理函数
                            } else {
                                console.log("点击了非链接区域（文本块：" + index + "）")
                            }
                        }
                    }
                }
            }

            // 滚动条
            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
                width: 8
            }
        }
    }

    // 横屏切换按钮
    Button {
        objectName: "orientationButton"
        text: isLandscape ? "切换竖屏" : "切换横屏"
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        visible: false
        onClicked: {
            isLandscape = !isLandscape
            console.log("QQuickWidget尺寸:", root.width, "x", root.height)
        }
    }

    function updatePageIndicator(deltaX, deltaY, currentMouseX, currentMouseY) {
        if (!root.isPressing)
            return

        // 右滑 → 上一页
        if (deltaX > root.scrollThreshold && Math.abs(
                    deltaY) < root.offsetLimit) {
            if (currentPage > 1) {

                pageIndicator.showPage(currentPage - 1, totalPages)
                root.needSwipePage = true
                root.swipeToNextPage = false // 上一页
            } else {

                root.needSwipePage = false
            }
        } // 左滑 → 下一页
        else if (deltaX < -root.scrollThreshold && Math.abs(
                     deltaY) < root.offsetLimit) {
            if (currentPage <= totalPages - 1) {

                pageIndicator.showPage(currentPage + 1, totalPages)
                root.needSwipePage = true
                root.swipeToNextPage = true // 下一页
            } else {
                pageIndicator.visible = false
                root.needSwipePage = false
            }
        } // 不满足条件
        else {
            pageIndicator.visible = false
            root.needSwipePage = false
        }
    }

    function handleSwipeGesture() {
        pageIndicator.visible = false

        if (root.needSwipePage) {
            if (root.swipeToNextPage) {

                m_Reader.goNextPage()
                console.log("释放：触发下一页")
            } else {

                m_Reader.goUpPage()
                console.log("释放：触发上一页")
            }
        } else {
            root.setX(0)
        }

        // 重置翻页意图
        root.needSwipePage = false
    }

    Item {
        id: pageIndicator
        // 固定尺寸
        width: 150
        height: 60
        // 关键：屏幕中央
        anchors.centerIn: parent
        visible: false // 默认隐藏

        // 半透明背景
        Rectangle {
            anchors.fill: parent
            color: "blue"
            opacity: 0.7
            radius: 4

            // 根据横屏/竖屏自动旋转
            rotation: isLandscape ? 90 : 0
            // 旋转中心设为文本中心
            transformOrigin: Item.Center
        }

        // 页码文本
        Text {
            id: mytext
            anchors.centerIn: parent
            text: "1/1"
            font.pixelSize: 45
            font.bold: true
            color: "white"
            padding: 8 // 留点内边距，避免文字贴边

            // 根据横屏/竖屏自动旋转
            rotation: isLandscape ? 90 : 0
            // 旋转中心设为文本中心
            transformOrigin: Item.Center
        }

        // 自动隐藏计时器
        Timer {
            id: hideTimer
            interval: 3000
            onTriggered: pageIndicator.visible = false
        }

        // 对外方法：显示页码（无任何多余判断）
        function showPage(current, total) {

            pageIndicator.visible = true
            mytext.text = current

            // 定时关闭
            // hideTimer.restart()
        }
    }
}
