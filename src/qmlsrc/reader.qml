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

    // 笔记功能
    property bool isSelectionMode: false // 是否处于文本选择模式
    property int selectionStart: -1 // 文本选择起始位置
    property int selectionEnd: -1 // 文本选择结束位置
    property color selectionColor: "#FFFF0080" // 选择高亮颜色
    property int currentNoteIndex: -1 // 当前点击的笔记索引
    property bool isBookPagePressHold: false
    // 存储当前有选择的 m_text 实例（解决 delegate 内无法访问问题）
    property var currentSelectedTextEdit: null
    property string selectionText: "" // 文本选择的内容

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

    // 重置文本选择状态（解决 delegate 内 m_text 无法访问问题）
    function resetTextSelection() {
        // 1. 取消“当前有选择的 TextEdit”的选择（通过记录的实例）
        if (root.currentSelectedTextEdit) {
            root.currentSelectedTextEdit.deselect() // 取消高亮
            root.currentSelectedTextEdit.cursorPosition
                    = root.currentSelectedTextEdit.selectionStart
            console.log("已取消当前文本块的选择")
        }

        // 2. 清空所有记录（实例 + 选择位置）
        root.currentSelectedTextEdit = null // 清空实例引用
        root.selectionStart = -1
        root.selectionEnd = -1

        console.log("文本选择已完全重置")
    }

    function setBookPagePressHold(value) {
        root.isBookPagePressHold = value
    }

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
        strText = strhtml
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

    ///////////////////////////////////////////////////////////////


    /*function getBookmarkText() {
        if (contentListView.count === 0)
            return "Bookmark"

        // 获取当前视口顶部的字符位置
        let topCharPos = getCharPositionAtViewportTop()
        if (topCharPos === -1)
            return "Bookmark"

        // 从该位置开始向后获取实际显示的文本
        return getVisibleTextFromPosition(topCharPos, 100) + "..." // 获取100个字符
    }

    function getCharPositionAtViewportTop() {
        // 处理横屏模式
        let viewportTop = root.isLandscape ? rotateContainer.height
                                             - contentListView.contentX : contentListView.contentY

        // 获取第一个可见项的索引
        let firstVisibleIndex = contentListView.indexAt(0, viewportTop)
        if (firstVisibleIndex === -1)
            return -1

        // 获取该项实例
        let delegateItem = contentListView.itemAtIndex(firstVisibleIndex)
        if (!delegateItem)
            return -1

        // 计算该项内的相对位置
        let relativePos = viewportTop - delegateItem.y

        // 获取该位置对应的字符索引
        return delegateItem.positionAt(0, relativePos)
    }

    function getVisibleTextFromPosition(startPos, maxLength) {
        let result = ""
        let currentPos = startPos
        let charsCollected = 0

        // 遍历所有文本块
        for (var i = 0; i < contentListView.count; i++) {
            let delegateItem = contentListView.itemAtIndex(i)
            if (!delegateItem)
                continue

            // 获取文本块的实际显示内容
            let text = getRenderedText(delegateItem)

            // 如果当前块包含起始位置
            if (currentPos < text.length) {
                let chunk = text.substring(
                        currentPos,
                        Math.min(currentPos + maxLength - charsCollected,
                                 text.length))
                result += chunk
                charsCollected += chunk.length

                if (charsCollected >= maxLength) {
                    return result
                }
            }

            // 更新位置为下一个块的起始位置
            currentPos = Math.max(0, currentPos - text.length)
        }

        return result
    }

    // 获取实际渲染的文本内容（去除HTML标签）
    function getRenderedText(textEdit) {
        // 方法1：使用TextEdit的getText方法获取纯文本
        if (typeof textEdit.getText === "function") {
            return textEdit.getText(0, textEdit.length)
        }

        // 方法2：使用正则表达式去除HTML标签
        return textEdit.text.replace(/<[^>]*>/g, '')
    }*/



    function getBookmarkText() {
        if (contentListView.count === 0)
            return "Bookmark"

        console.log("===== 开始获取书签文本 =====")
        console.log("当前模式:", root.isLandscape ? "横屏" : "竖屏")

        // 获取第一个文本块（也是唯一一个）
        let delegateItem = contentListView.itemAtIndex(0)
        if (!delegateItem) {
            console.log("无法获取文本块")
            return "Bookmark"
        }

        // 获取屏幕左上角在文本块中的位置
        let charPos = getCharPositionAtScreenTopLeft(delegateItem)
        console.log("屏幕左上角字符位置:", charPos)

        if (charPos === -1)
            return "Bookmark"

        // 获取实际显示的文本
        let text = getRenderedText(delegateItem)
        console.log("文本长度:", text.length)

        // 从该位置开始向后获取文本
        let result = text.substring(charPos, Math.min(charPos + 100, text.length))
        console.log("书签文本:", result)

        return result
    }

    function getCharPositionAtScreenTopLeft(delegateItem) {
        // 使用屏幕左上角(0,0)作为模拟点击位置
        let mousePoint = Qt.point(0, 0)
        console.log("模拟点击位置:", mousePoint.x, mousePoint.y)

        // 将屏幕坐标映射到旋转容器坐标系
        let pointInRotateContainer = rotateContainer.mapFromItem(null, mousePoint.x, mousePoint.y)
        console.log("在旋转容器中的坐标:", pointInRotateContainer.x, pointInRotateContainer.y)

        // 在横屏模式下补偿Y坐标偏移
        if (root.isLandscape) {
            pointInRotateContainer.y = pointInRotateContainer.y - rotateContainer.height
            console.log("横屏模式 - 补偿后坐标:", pointInRotateContainer.x, pointInRotateContainer.y)
        }

        // 将旋转容器坐标映射到ListView坐标系
        let pointInListView = contentListView.mapFromItem(rotateContainer, pointInRotateContainer.x, pointInRotateContainer.y)
        console.log("在ListView中的坐标:", pointInListView.x, pointInListView.y)

        // 在横屏模式下，进一步调整ListView坐标
        if (root.isLandscape) {
            // 减去文本块的Y位置，确保局部坐标从0开始
            pointInListView.y = pointInListView.y - delegateItem.y
            console.log("横屏模式 - 调整后ListView坐标:", pointInListView.x, pointInListView.y)
        }

        // 将ListView坐标映射到文本块的局部坐标
        let pointInItem = delegateItem.mapFromItem(contentListView, pointInListView.x, pointInListView.y)
        console.log("在文本块内的局部坐标:", pointInItem.x, pointInItem.y)

        // 获取该位置对应的字符索引
        return delegateItem.positionAt(pointInItem.x, pointInItem.y)
    }

    function getRenderedText(textEdit) {
        // 优先使用 getText 方法
        if (typeof textEdit.getText === "function") {
            return textEdit.getText(0, textEdit.length)
        }

        // 其次使用 Accessible.text
        if (textEdit.Accessible && textEdit.Accessible.text) {
            return textEdit.Accessible.text
        }

        // 最后使用正则表达式去除HTML标签
        return textEdit.text.replace(/<[^>]*>/g, '')
    }





    ///////////////////////////////////////////////////////
    function setTextAreaCursorPos(nCursorPos) {
        textArea.cursorPosition = nCursorPos
    }

    function handleLinkClicked(link) {
        document.setBackDir(link)
        document.parsingLink(link, "reader")
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
            visible: !isSelectionMode // 选择模式下隐藏
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
                    if (!isMoving && !root.isBookPagePressHold) {
                        m_Reader.on_SetReaderFunVisible()
                    }
                }

                onPressAndHold: function (mouse) {

                    if (!isMoving && !root.isBookPagePressHold) {
                        mw_one.on_btnSelText_clicked()
                        root.isBookPagePressHold = true
                    }
                    mouse.accepted = false
                }
                onPositionChanged: {
                    isMoving = true
                }

                onReleased: {
                    isMoving = false
                }

                onCanceled: {
                    isMoving = false
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

            delegate: TextEdit {
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

                readOnly: true
                selectByMouse: false

                // 笔记高亮渲染 - 仅渲染，不添加交互
                Repeater {
                    id: notesRepeater
                    model: notesModel

                    delegate: Item {
                        id: noteDelegate

                        property int noteStart: model.start
                        property int noteEnd: model.end
                        property bool isValidPosition: noteStart >= 0
                                                       && noteEnd <= m_text.length
                        property color noteColor: model.color ? model.color : "#FFFF0080"

                        // 计算 start/end 矩形（用于点击判断）
                        property var startRect: isValidPosition ? m_text.positionToRectangle(
                                                                      noteStart) : null
                        property var endRect: isValidPosition ? m_text.positionToRectangle(
                                                                    noteEnd) : null

                        visible: isValidPosition

                        // 逐字符高亮
                        Repeater {
                            model: Math.max(0, noteEnd - noteStart)

                            delegate: Rectangle {
                                property int charIndex: noteStart + index
                                property var leftCursorRect: m_text.positionToRectangle(
                                                                 charIndex)
                                property var rightCursorRect: m_text.positionToRectangle(
                                                                  charIndex + 1)

                                property real charWidth: {
                                    if (!leftCursorRect)
                                        return 0
                                    if (!rightCursorRect)
                                        return leftCursorRect.width

                                    var w = rightCursorRect.x - leftCursorRect.x

                                    // 宽度有效
                                    if (w > 0) {
                                        return w
                                    }

                                    // 宽度无效时，取前一个字符的宽度
                                    if (charIndex > 0) {
                                        var prevLeft = m_text.positionToRectangle(
                                                    charIndex - 1)
                                        var prevRight = m_text.positionToRectangle(
                                                    charIndex)
                                        if (prevLeft && prevRight) {
                                            var prevW = prevRight.x - prevLeft.x
                                            if (prevW > 0) {
                                                return prevW
                                            }
                                        }
                                    }

                                    // 兜底
                                    return leftCursorRect.width
                                }

                                property real charHeight: leftCursorRect ? leftCursorRect.height : 0

                                visible: leftCursorRect !== null
                                         && charWidth > 0

                                x: leftCursorRect.x
                                y: leftCursorRect.y
                                width: charWidth
                                height: charHeight
                                color: noteColor
                                opacity: 0.4
                                z: 5
                            }
                        }
                    }
                }

                MouseArea {
                    id: textMouseArea
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton
                    propagateComposedEvents: true

                    onPressAndHold: function (mouse) {
                        console.log("文本长按已经启动...")

                        // 1. 获取长按位置对应的字符索引（相对于 m_text）
                        var pressCharPos = m_text.positionAt(mouse.x, mouse.y)

                        // 2. 边界判断：确保位置有效（避免 -1 等错误值）
                        if (pressCharPos >= 0
                                && pressCharPos < m_text.text.length) {
                            // 3. 计算选择范围（当前位置 + 2 个字符，不超过文本总长）
                            var selectStart = pressCharPos
                            var selectEnd = Math.min(pressCharPos + 2,
                                                     m_text.text.length)

                            root.currentSelectedTextEdit = m_text

                            // 4. 存储位置到全局属性（供后续自绘手柄使用）
                            root.selectionStart = selectStart
                            root.selectionEnd = selectEnd

                            // 5. 触发 TextEdit 自身的选择（显示默认文本高亮，可选）
                            m_text.select(selectStart, selectEnd)

                            root.selectionText = root.currentSelectedTextEdit.selectedText
                            m_Reader.setEditText(root.selectionText, "right")
                            m_Reader.setStartEnd(root.selectionStart,
                                                 root.selectionEnd)

                            console.log("长按选择存储：start=" + root.selectionStart
                                        + ", end=" + root.selectionEnd)
                        } else {
                            // 6. 位置无效时，重置全局属性
                            root.currentSelectedTextEdit = null
                            root.selectionStart = -1
                            root.selectionEnd = -1
                            console.log("长按位置无效，重置选择")
                        }
                    }

                    onClicked: function (mouse) {

                        // 检查是否点击了笔记区域
                        var clickedNote = false
                        var noteContent = ""
                        var noteIndex = -1

                        for (var i = 0; i < notesModel.count; i++) {
                            if (isPointInNote(mouse.x, mouse.y, i)) {
                                clickedNote = true
                                noteIndex = i
                                noteContent = notesModel.get(i).content
                                console.log("点击了笔记区域")
                                break
                            }
                        }

                        if (clickedNote) {
                            console.log("显示笔记内容:", noteContent)
                            m_Reader.setShowNoteValue(true)
                            notePopup.showNote(noteContent, noteIndex) // 传递索引
                            mouse.accepted = true
                        } else if (!root.isMoving) {

                            var link = m_text.linkAt(mouse.x, mouse.y)
                            if (link) {
                                root.handleLinkClicked(link)
                            } else {
                                console.log("点击了非链接区域（文本块：" + index + "）")
                            }
                        }
                    }

                    // 检查点是否在笔记区域内
                    function isPointInNote(x, y, noteIndex) {
                        // 获取笔记委托项
                        var noteItem = notesRepeater.itemAt(noteIndex)
                        if (!noteItem || !noteItem.isValidPosition)
                            return false

                        var startRect = noteItem.startRect
                        var endRect = noteItem.endRect

                        if (!startRect || !endRect)
                            return false

                        // 检查点是否在笔记区域内
                        if (startRect.y === endRect.y) {
                            return x >= startRect.x
                                    && x <= endRect.x + endRect.width
                                    && y >= startRect.y
                                    && y <= startRect.y + startRect.height
                        } else {
                            // 跨行笔记处理
                            var lineHeight = startRect.height
                            var startY = startRect.y
                            var endY = endRect.y

                            // 检查点是否在笔记区域内
                            if (y >= startY && y <= endY + lineHeight) {
                                if (y === startY) {
                                    return x >= startRect.x
                                } else if (y === endY) {
                                    return x <= endRect.x + endRect.width
                                } else {
                                    return true
                                }
                            }
                        }
                        return false
                    }
                }

                //onLinkActivated: handleLinkClicked(link)
                PropertyAnimation on x {
                    easing.type: Easing.Linear
                    running: isAni
                    from: aniW
                    to: 0
                    duration: 200
                    loops: 1
                }

                ////////////////////////////////////////////////////
            }

            // 滚动条
            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
                width: 8
            }
        }

        // ==============================================
        // 选择渲染层（覆盖在文本层上方）
        Item {
            id: selectionLayer
            anchors.fill: parent
            visible: isSelectionMode

            // 添加滚动视图容器
            ScrollView {
                id: selectionScrollView
                anchors.fill: parent
                contentWidth: selectionListView.contentWidth
                contentHeight: selectionListView.contentHeight
                clip: true

                // 同步滚动位置
                Component.onCompleted: {
                    contentY = contentListView.contentY
                }

                // 文本选择渲染
                ListView {
                    id: selectionListView
                    width: rotateContainer.width
                    height: rotateContainer.height
                    model: textModel
                    spacing: contentListView.spacing // 保持相同的间距
                    contentY: contentListView.contentY // 同步滚动位置

                    delegate: TextEdit {
                        id: selectionText
                        width: rotateContainer.width
                        leftPadding: 10
                        rightPadding: 10
                        textFormat: Text.RichText
                        text: model.text
                        wrapMode: Text.Wrap
                        font.pixelSize: FontSize
                        font.family: FontName
                        color: myTextColor
                        renderType: Text.QtRendering

                        // 可编辑模式
                        readOnly: true
                        selectByMouse: true

                        // 文本选择高亮
                        Rectangle {
                            visible: selectionText.selectedText.length > 0
                            color: selectionColor
                            opacity: 0.4
                            x: selectionText.positionToRectangle(
                                   selectionText.selectionStart).x
                            y: selectionText.positionToRectangle(
                                   selectionText.selectionStart).y
                            width: selectionText.positionToRectangle(
                                       selectionText.selectionEnd).x
                                   + selectionText.positionToRectangle(
                                       selectionText.selectionEnd).width - x
                            height: selectionText.positionToRectangle(
                                        selectionText.selectionStart).height
                        }

                        MouseArea {
                            id: textMouseArea1
                            anchors.fill: parent
                            acceptedButtons: Qt.LeftButton
                            propagateComposedEvents: true

                            onPressAndHold: function (mouse) {

                                console.log("渲染层长按已经启动...")
                            }
                        }
                    }
                }
            }

            // 笔记操作工具栏
            Row {
                anchors.top: parent.top
                anchors.right: parent.right
                spacing: 10
                padding: 10
                z: 100

                Button {
                    text: "添加笔记"
                    onClicked: {
                        // 获取当前选择的文本和位置
                        var selectedText = ""
                        var startPos = -1
                        var endPos = -1

                        // 查找当前选择的文本块
                        for (var i = 0; i < selectionListView.contentItem.children.length; i++) {
                            var child = selectionListView.contentItem.children[i]
                            if (child.selectedText
                                    && child.selectedText.length > 0) {
                                selectedText = child.selectedText
                                startPos = child.selectionStart
                                endPos = child.selectionEnd
                                break
                            }
                        }

                        if (selectedText.length > 0) {
                            noteDialog.showDialog(selectedText,
                                                  startPos, endPos)
                        }
                    }
                }

                Button {
                    text: "取消"
                    onClicked: {
                        root.isSelectionMode = false
                    }
                }
            }
        }

        // ==============================================
        // 【新增结束】
        // ==============================================
    }

    // 笔记详情弹窗
    Popup {
        id: notePopup
        width: 320
        height: 360
        font.pointSize: FontSize
        modal: true
        focus: true
        anchors.centerIn: Overlay.overlay

        background: Rectangle {
            color: isDark ? "#333333" : "#DDDDDD"
            border.color: notePopup.palette.windowText
            border.width: 1
            radius: 8
        }

        function showNote(content, index) {
            noteContent.text = content
            currentNoteIndex = index
            open()
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10

            Label {
                font.pointSize: FontSize
                text: qsTr("Note Content:")
                color: isDark ? "#EEEEEE" : "#111111"
                font.bold: true
            }

            Flickable {
                id: flickable
                Layout.fillWidth: true
                Layout.fillHeight: true
                contentWidth: noteContent.width
                contentHeight: noteContent.implicitHeight
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                flickableDirection: Flickable.VerticalFlick

                // 文本内容
                TextEdit {
                    id: noteContent
                    width: flickable.width
                    font.pointSize: FontSize
                    readOnly: true
                    wrapMode: Text.Wrap
                    selectByMouse: false
                    textFormat: Text.PlainText
                    color: isDark ? "#EEEEEE" : "#111111"

                    // 确保文本区域高度正确
                    onTextChanged: {
                        // 强制更新布局
                        noteContent.forceActiveFocus()
                        Qt.callLater(() => {
                                         flickable.contentHeight = noteContent.implicitHeight
                                     })
                    }
                }

                // 垂直滚动条
                ScrollBar.vertical: ScrollBar {
                    id: vbar
                    policy: ScrollBar.AsNeeded
                    width: 8
                    anchors.top: parent.top
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                }
            }

            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                Button {
                    font.pointSize: FontSize
                    text: qsTr("Edit")
                    onClicked: {
                        // 打开编辑弹窗，修改后更新模型
                        notesModel.set(currentNoteIndex, {
                                           "content": noteContent.text
                                       })
                        notePopup.close()
                        m_Reader.setShowNoteValue(false)
                        m_Reader.editBookNote(currentNoteIndex, -1,
                                              noteContent.text)
                    }
                }
                Button {
                    font.pointSize: FontSize
                    text: qsTr("Del")
                    onClicked: {

                        deleteConfirmDialog.open()
                    }
                }
                Button {
                    font.pointSize: FontSize
                    text: qsTr("Close")
                    onClicked: {
                        notePopup.close()
                        m_Reader.setShowNoteValue(false)
                    }
                }
            }
        }

        onClosed: {

            console.log("弹窗已关闭，等待 500ms 后执行")
            timer.start()
        }

        Timer {
            id: timer
            interval: 500 // 延时 500 毫秒
            running: false
            repeat: false
            onTriggered: {
                m_Reader.setShowNoteValue(false)
            }
        }
    }

    Dialog {
        id: deleteConfirmDialog
        font.pointSize: FontSize
        title: qsTr("Delete Confirmation")

        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel

        // 确保对话框居中显示
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2

        Label {
            font.pointSize: FontSize
            text: qsTr("Are you sure you want to delete this note?")
            anchors.centerIn: parent
        }
    }

    // 删除确认后执行删除
    Connections {
        target: deleteConfirmDialog
        function onAccepted() {
            notesModel.remove(currentNoteIndex)
            m_Reader.delReadNote(currentNoteIndex)
            notePopup.close()
            m_Reader.setShowNoteValue(false)
        }
    }

    // 添加新笔记的函数
    function addNote(start, end, color, content) {
        notesModel.append({
                              "start": start,
                              "end": end,
                              "color": color,
                              "content": content
                          })
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
        if (!root.isMoving)
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

                root.needSwipePage = false
            }
        } // 不满足条件
        else {
            pageIndicator.visible = false
            root.needSwipePage = false
        }

        console.log("needSwipePage=" + needSwipePage)
    }

    function handleSwipeGesture() {

        if (root.needSwipePage) {
            if (root.swipeToNextPage) {

                m_Reader.goNextPage()
                console.log("释放：触发下一页")
            } else {

                m_Reader.goUpPage()
                console.log("释放：触发上一页")
            }
            isPressing = false
            isMoving = false
            pageIndicator.visible = false
        } else {
            root.setX(0)
        }

        // 重置翻页意图
        root.needSwipePage = false
    }

    function showBookPageNext() {

        pageIndicator.showPage(currentPage + 1, totalPages)
    }

    function showBookPageUp() {

        pageIndicator.showPage(currentPage - 1, totalPages)
    }

    function closeBookPage() {
        pageIndicator.hide()
    }

    Item {
        id: pageIndicator
        width: 150
        height: 60
        anchors.centerIn: parent
        opacity: 0 // 初始透明，不隐藏

        Rectangle {
            anchors.fill: parent
            color: "#3A362E"
            opacity: 0.7
            radius: 4
            rotation: isLandscape ? 90 : 0
            transformOrigin: Item.Center
        }

        Text {
            id: mytext
            anchors.centerIn: parent
            text: "1/1"
            font.pixelSize: 45
            font.bold: true
            color: "#F0E6D2"
            padding: 8
            rotation: isLandscape ? 90 : 0
            transformOrigin: Item.Center
        }

        // 淡入动画
        PropertyAnimation {
            id: fadeInAnim
            target: pageIndicator
            property: "opacity"
            to: 1
            duration: 300
        }

        // 淡出动画
        PropertyAnimation {
            id: fadeOutAnim
            target: pageIndicator
            property: "opacity"
            to: 0
            duration: 300
        }

        // 备用 Timer（默认不启动）
        Timer {
            id: hideTimer
            interval: 3000
            onTriggered: hide()
        }

        function show() {
            fadeOutAnim.stop() // 防止淡出中突然淡入
            fadeInAnim.start()
        }

        function hide() {
            fadeInAnim.stop() // 防止淡入中突然淡出
            fadeOutAnim.start()
        }

        function showPage(current, total) {
            mytext.text = current
            show()
        }
    }

    ListModel {
        id: notesModel
    }

    // 自动刷新笔记模型数据
    Connections {
        target: m_Reader
        function onNotesLoaded(notes) {
            notesModel.clear()
            for (var i = 0; i < notes.length; i++) {
                var n = notes[i]
                notesModel.append({
                                      "start": n.start,
                                      "end": n.end,
                                      "color": n.color,
                                      "content": n.content
                                  })
            }
        }
    }

    ///////////////////////////自定义选择手柄层//////////////////////////////////////
    // ==================== 起点手柄 ====================
    Item {
        id: startHandle
        width: 24
        height: 24
        z: 10001
        visible: root.selectionStart !== -1 && root.selectionEnd !== -1
                 && !isLandscape

        Rectangle {
            anchors.fill: parent
            radius: 12
            color: root.selectionColor
            border.color: "white"
            border.width: 2
        }

        MouseArea {
            id: startHandleArea
            anchors.fill: parent
            drag.target: startHandle
            drag.axis: Drag.XAndYAxis
            drag.minimumX: 0
            drag.minimumY: 0
            drag.maximumX: root.width - startHandle.width
            drag.maximumY: root.height - startHandle.height

            property real lastX: startHandle.x
            property real lastY: startHandle.y
            property int logCounter: 0

            onPressed: {
                console.log("起点手柄按下")
                startHandle.z = 10001
                lastX = startHandle.x
                lastY = startHandle.y
                logCounter = 0
            }

            onPositionChanged: function (mouse) {
                logCounter++
                if (logCounter % 5 === 0) {
                    console.log("起点手柄拖动中 - X:", startHandle.x.toFixed(1), "Y:",
                                startHandle.y.toFixed(1),
                                "ΔX:", (startHandle.x - lastX).toFixed(1),
                                "ΔY:", (startHandle.y - lastY).toFixed(1))
                    lastX = startHandle.x
                    lastY = startHandle.y

                    // 严格遵循可拖动矩形的方法获取字符位置
                    // 1. 将手柄位置映射到 contentListView
                    var listViewPos = contentListView.mapFromItem(startHandle,
                                                                  mouse.x,
                                                                  mouse.y)

                    // 2. 考虑滚动偏移
                    var contentYPos = listViewPos.y + contentListView.contentY

                    // 3. 获取列表项索引
                    var itemIndex = contentListView.indexAt(listViewPos.x,
                                                            contentYPos)
                    console.log("列表项索引:", itemIndex)

                    if (itemIndex >= 0) {
                        var item = contentListView.itemAtIndex(itemIndex)
                        if (item) {
                            // 4. 将鼠标位置映射到该 item
                            var itemPos = item.mapFromItem(startHandle,
                                                           mouse.x, mouse.y)

                            // 5. 获取字符索引
                            var charIndex = item.positionAt(itemPos.x,
                                                            itemPos.y)
                            console.log("起点手柄位置 - 字符索引:", charIndex)

                            // 确保新位置在有效范围内
                            if (charIndex >= 0
                                    && charIndex <= root.selectionEnd) {
                                console.log("更新 selectionStart 前:",
                                            root.selectionStart)
                                root.selectionStart = charIndex
                                console.log("更新 selectionStart 后:",
                                            root.selectionStart)

                                // 刷新文本选择
                                item.select(root.selectionStart,
                                            root.selectionEnd)
                                root.selectionText = item.selectedText

                                // 通知外部组件更新
                                if (root.hasOwnProperty("selectionText")) {
                                    m_Reader.setEditText(root.selectionText,
                                                         "left")
                                    m_Reader.setStartEnd(root.selectionStart,
                                                         root.selectionEnd)
                                }
                            }
                        }
                    }
                }
            }

            onReleased: {
                updateHandlePositions()
                console.log("起点手柄释放")
            }
        }
    }

    // ==================== 终点手柄 ====================
    Item {
        id: endHandle
        width: 24
        height: 24
        z: 10001
        visible: root.selectionStart !== -1 && root.selectionEnd !== -1
                 && !isLandscape

        Rectangle {
            anchors.fill: parent
            radius: 12
            color: root.selectionColor
            border.color: "white"
            border.width: 2
        }

        MouseArea {
            id: endHandleArea
            anchors.fill: parent
            drag.target: endHandle
            drag.axis: Drag.XAndYAxis
            drag.minimumX: 0
            drag.minimumY: 0
            drag.maximumX: root.width - endHandle.width
            drag.maximumY: root.height - endHandle.height

            property real lastX: endHandle.x
            property real lastY: endHandle.y
            property int logCounter: 0

            onPressed: {
                console.log("终点手柄按下")
                endHandle.z = 10001
                lastX = endHandle.x
                lastY = endHandle.y
                logCounter = 0
            }

            onPositionChanged: function (mouse) {
                logCounter++
                if (logCounter % 5 === 0) {
                    console.log("终点手柄拖动中 - X:", endHandle.x.toFixed(1), "Y:",
                                endHandle.y.toFixed(1),
                                "ΔX:", (endHandle.x - lastX).toFixed(1),
                                "ΔY:", (endHandle.y - lastY).toFixed(1))
                    lastX = endHandle.x
                    lastY = endHandle.y

                    // 严格遵循可拖动矩形的方法获取字符位置
                    // 1. 将手柄位置映射到 contentListView
                    var listViewPos = contentListView.mapFromItem(endHandle,
                                                                  mouse.x,
                                                                  mouse.y)

                    // 2. 考虑滚动偏移
                    var contentYPos = listViewPos.y + contentListView.contentY

                    // 3. 获取列表项索引
                    var itemIndex = contentListView.indexAt(listViewPos.x,
                                                            contentYPos)
                    console.log("列表项索引:", itemIndex)

                    if (itemIndex >= 0) {
                        var item = contentListView.itemAtIndex(itemIndex)
                        if (item) {
                            // 4. 将鼠标位置映射到该 item
                            var itemPos = item.mapFromItem(endHandle,
                                                           mouse.x, mouse.y)

                            // 5. 获取字符索引
                            var charIndex = item.positionAt(itemPos.x,
                                                            itemPos.y)
                            console.log("终点手柄位置 - 字符索引:", charIndex)

                            // 确保新位置在有效范围内
                            if (charIndex >= root.selectionStart
                                    && charIndex <= item.text.length) {
                                console.log("更新 selectionEnd 前:",
                                            root.selectionEnd)
                                root.selectionEnd = charIndex
                                console.log("更新 selectionEnd 后:",
                                            root.selectionEnd)

                                // 刷新文本选择
                                item.select(root.selectionStart,
                                            root.selectionEnd)
                                root.selectionText = item.selectedText

                                // 通知外部组件更新
                                if (root.hasOwnProperty("selectionText")) {
                                    m_Reader.setEditText(root.selectionText,
                                                         "right")
                                    m_Reader.setStartEnd(root.selectionStart,
                                                         root.selectionEnd)
                                }
                            }
                        }
                    }
                }
            }

            onReleased: {
                updateHandlePositions()
                console.log("终点手柄释放")
            }
        }
    }

    // ==================== 手柄位置更新函数 ====================
    function updateHandlePositions() {
        if (!root.currentSelectedTextEdit || root.selectionStart === -1
                || root.selectionEnd === -1)
            return

        var textEdit = root.currentSelectedTextEdit

        // 更新起点手柄位置
        var startRect = textEdit.positionToRectangle(root.selectionStart)
        if (startRect) {
            // 起点手柄显示在 selectionStart 前一个字符位置（解决压字问题）
            if (root.selectionStart > 0) {
                var prevRect = textEdit.positionToRectangle(
                            root.selectionStart - 1)
                startHandle.x = prevRect.x
                startHandle.y = prevRect.y - contentListView.contentY
            } else {
                startHandle.x = startRect.x
                startHandle.y = startRect.y - contentListView.contentY
            }
        }

        // 更新终点手柄位置
        var endRect = textEdit.positionToRectangle(root.selectionEnd)
        if (endRect) {
            endHandle.x = endRect.x
            endHandle.y = endRect.y - contentListView.contentY
        }
    }

    // ==================== 监听事件 ====================
    Connections {
        target: root
        function onSelectionStartChanged() {
            updateHandlePositions()
        }
        function onSelectionEndChanged() {
            updateHandlePositions()
        }
    }

    Connections {
        target: contentListView
        function onContentYChanged() {
            updateHandlePositions()
        }
    }

    ////////////////////////////////test///////////////////////////////////////
    Item {
        id: movableItem
        x: 50
        y: 50
        width: 30
        height: 30
        z: 10000
        visible: false

        Rectangle {
            anchors.fill: parent
            color: "blue"
            radius: 10
            opacity: 0.8
            border.width: 3
            border.color: "white"
        }

        Text {
            anchors.centerIn: parent
            text: "拖"
            color: "white"
            font.bold: true
            font.pixelSize: 12
        }

        MouseArea {
            id: dragArea
            anchors.fill: parent
            drag.target: movableItem
            drag.axis: Drag.XAndYAxis
            drag.minimumX: 0
            drag.minimumY: 0
            drag.maximumX: root.width - movableItem.width
            drag.maximumY: root.height - movableItem.height

            // 添加位置变化跟踪
            property real lastX: movableItem.x
            property real lastY: movableItem.y
            property int logCounter: 0

            onPressed: {
                console.log("开始拖动 movableItem")
                movableItem.z = 10001
                // 记录初始位置
                lastX = movableItem.x
                lastY = movableItem.y
                logCounter = 0
            }

            onPositionChanged: function (mouse) {
                // 减少日志输出频率，每5次位置变化记录一次
                logCounter++
                if (logCounter % 5 === 0) {
                    console.log("拖动中 - X:", movableItem.x.toFixed(1), "Y:",
                                movableItem.y.toFixed(1),
                                "ΔX:", (movableItem.x - lastX).toFixed(1),
                                "ΔY:", (movableItem.y - lastY).toFixed(1))
                    lastX = movableItem.x
                    lastY = movableItem.y

                    // 新增：获取鼠标在 m_text 中的位置
                    var listViewPos = contentListView.mapFromItem(movableItem,
                                                                  mouse.x,
                                                                  mouse.y)
                    var contentYPos = listViewPos.y + contentListView.contentY
                    var itemIndex = contentListView.indexAt(listViewPos.x,
                                                            contentYPos)

                    if (itemIndex >= 0) {
                        var item = contentListView.itemAtIndex(itemIndex)
                        if (item) {
                            // 将鼠标位置映射到该 item
                            var itemPos = item.mapFromItem(movableItem,
                                                           mouse.x, mouse.y)
                            var charIndex = item.positionAt(itemPos.x,
                                                            itemPos.y)
                            console.log("当前字符位置:", charIndex)
                        }
                    }
                }
            }

            onReleased: {
                console.log("拖动结束 - 最终位置: X:", movableItem.x.toFixed(1), "Y:",
                            movableItem.y.toFixed(1))
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    Item {
        id: selectionControlPanel
        visible: root.currentSelectedTextEdit && root.selectionStart !== -1
                 && root.selectionEnd !== -1
        z: 3000

        width: 300
        height: 55
        x: (root.width - width) / 2
        y: 10

        Rectangle {
            anchors.fill: parent
            color: isDark ? "#555555" : "#EEEEEE"
            border.color: "#E0E0E0"
            border.width: 1
            radius: 6
        }

        // 外层布局：确保垂直居中，且上下有间隙
        RowLayout {
            anchors.fill: parent
            anchors.margins: 8
            spacing: 15
            Layout.alignment: Qt.AlignCenter // 水平+垂直都居中

            // 起点控制组
            RowLayout {
                spacing: 6
                Layout.alignment: Qt.AlignVCenter

                Text {
                    text: qsTr("Start:")
                    font.pixelSize: 13
                    color: isDark ? "#DDDDDD" : "#333333"
                    Layout.preferredWidth: 40
                    Layout.maximumWidth: 40
                    Layout.alignment: Qt.AlignVCenter
                }

                Button {
                    text: "-"
                    Layout.preferredWidth: 40
                    Layout.maximumWidth: 40
                    Layout.preferredHeight: 40
                    Layout.maximumHeight: 40
                    width: 40
                    height: 40
                    font.pointSize: uiFontSize
                    flat: false
                    Layout.alignment: Qt.AlignVCenter

                    autoRepeat: true
                    autoRepeatDelay: 300
                    autoRepeatInterval: 100

                    onClicked: {
                        if (root.currentSelectedTextEdit
                                && root.selectionStart > 0) {
                            root.selectionStart -= 1
                            root.currentSelectedTextEdit.select(
                                        root.selectionStart, root.selectionEnd)
                            if (root.hasOwnProperty("selectionText")) {
                                root.selectionText = root.currentSelectedTextEdit.selectedText
                                m_Reader.setEditText(root.selectionText, "left")
                                m_Reader.setStartEnd(root.selectionStart,
                                                     root.selectionEnd)
                            }
                        }
                    }
                }

                Button {
                    text: "+"
                    Layout.preferredWidth: 40
                    Layout.maximumWidth: 40
                    Layout.preferredHeight: 40
                    Layout.maximumHeight: 40
                    width: 40
                    height: 40
                    font.pointSize: uiFontSize
                    flat: false
                    Layout.alignment: Qt.AlignVCenter

                    autoRepeat: true
                    autoRepeatDelay: 300
                    autoRepeatInterval: 100

                    onClicked: {
                        if (root.currentSelectedTextEdit
                                && root.selectionStart < root.selectionEnd - 1) {
                            root.selectionStart += 1
                            root.currentSelectedTextEdit.select(
                                        root.selectionStart, root.selectionEnd)
                            if (root.hasOwnProperty("selectionText")) {
                                root.selectionText = root.currentSelectedTextEdit.selectedText
                                m_Reader.setEditText(root.selectionText, "left")
                                m_Reader.setStartEnd(root.selectionStart,
                                                     root.selectionEnd)
                            }
                        }
                    }
                }
            }

            // 终点控制组
            RowLayout {
                spacing: 6
                Layout.alignment: Qt.AlignVCenter

                Text {
                    text: qsTr("End:")
                    font.pixelSize: 13
                    color: isDark ? "#DDDDDD" : "#333333"
                    Layout.preferredWidth: 40
                    Layout.maximumWidth: 40
                    Layout.alignment: Qt.AlignVCenter
                }

                Button {
                    text: "-"
                    Layout.preferredWidth: 40
                    Layout.maximumWidth: 40
                    Layout.preferredHeight: 40
                    Layout.maximumHeight: 40
                    width: 40
                    height: 40
                    font.pointSize: uiFontSize
                    flat: false
                    Layout.alignment: Qt.AlignVCenter

                    autoRepeat: true
                    autoRepeatDelay: 300
                    autoRepeatInterval: 100

                    onClicked: {
                        if (root.currentSelectedTextEdit
                                && root.selectionEnd > root.selectionStart + 1) {
                            root.selectionEnd -= 1
                            root.currentSelectedTextEdit.select(
                                        root.selectionStart, root.selectionEnd)
                            if (root.hasOwnProperty("selectionText")) {
                                root.selectionText = root.currentSelectedTextEdit.selectedText
                                m_Reader.setEditText(root.selectionText,
                                                     "right")
                                m_Reader.setStartEnd(root.selectionStart,
                                                     root.selectionEnd)
                            }
                        }
                    }
                }

                Button {
                    text: "+"
                    Layout.preferredWidth: 40
                    Layout.maximumWidth: 40
                    Layout.preferredHeight: 40
                    Layout.maximumHeight: 40
                    width: 40
                    height: 40
                    font.pointSize: uiFontSize
                    flat: false
                    Layout.alignment: Qt.AlignVCenter

                    autoRepeat: true
                    autoRepeatDelay: 300
                    autoRepeatInterval: 100

                    onClicked: {
                        if (root.currentSelectedTextEdit
                                && root.selectionEnd < root.currentSelectedTextEdit.text.length) {
                            root.selectionEnd += 1
                            root.currentSelectedTextEdit.select(
                                        root.selectionStart, root.selectionEnd)
                            if (root.hasOwnProperty("selectionText")) {
                                root.selectionText = root.currentSelectedTextEdit.selectedText
                                m_Reader.setEditText(root.selectionText,
                                                     "right")
                                m_Reader.setStartEnd(root.selectionStart,
                                                     root.selectionEnd)
                            }
                        }
                    }
                }
            }
        }

        // 监听root状态
        Connections {
            target: root
            function onSelectionStartChanged() {
                if (root.currentSelectedTextEdit) {
                    root.currentSelectedTextEdit.select(root.selectionStart,
                                                        root.selectionEnd)
                }
                selectionControlPanel.visible = (root.currentSelectedTextEdit
                                                 && root.selectionStart !== -1
                                                 && root.selectionEnd !== -1)
            }

            function onSelectionEndChanged() {
                if (root.currentSelectedTextEdit) {
                    root.currentSelectedTextEdit.select(root.selectionStart,
                                                        root.selectionEnd)
                }
                selectionControlPanel.visible = (root.currentSelectedTextEdit
                                                 && root.selectionStart !== -1
                                                 && root.selectionEnd !== -1)
            }

            function onCurrentSelectedTextEditChanged() {
                selectionControlPanel.visible = (root.currentSelectedTextEdit
                                                 && root.selectionStart !== -1
                                                 && root.selectionEnd !== -1)
            }
        }
    }
}
