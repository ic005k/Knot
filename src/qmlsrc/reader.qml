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
                    if (!isMoving)
                        m_Reader.on_SetReaderFunVisible()
                }

                onPressAndHold: {

                    if (!isMoving)
                        mw_one.on_btnSelText_clicked()


                    /*if (!root.isMoving) {
                        // 进入选择模式
                        root.isSelectionMode = true
                        root.selectionStart = -1
                        root.selectionEnd = -1
                    }*/
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

                        // 检查位置是否有效
                        property bool isValidPosition: model.start >= 0
                                                       && model.end <= m_text.length

                        // 计算起始和结束位置
                        property var startRect: isValidPosition ? m_text.positionToRectangle(
                                                                      model.start) : null
                        property var endRect: isValidPosition ? m_text.positionToRectangle(
                                                                    model.end) : null

                        // 确保颜色值有效
                        property color noteColor: model.color ? model.color : "#FFFF0080"

                        // 单行笔记
                        Rectangle {
                            id: noteRect
                            visible: isValidPosition && startRect && endRect
                                     && startRect.y === endRect.y
                            color: noteDelegate.noteColor
                            opacity: 0.4
                            x: startRect ? startRect.x : 0
                            y: startRect ? startRect.y : 0
                            width: startRect
                                   && endRect ? (endRect.x + endRect.width - x) : 0
                            height: startRect ? startRect.height : 0
                            z: 5 // 确保在文本下方
                        }

                        // 跨行笔记
                        Repeater {
                            model: isValidPosition && startRect && endRect
                                   && startRect.y
                                   !== endRect.y ? Math.floor(
                                                       (endRect.y - startRect.y)
                                                       / startRect.height) + 1 : 0

                            delegate: Rectangle {
                                id: multiLineRect
                                color: noteDelegate.noteColor
                                opacity: 0.4
                                x: index === 0 ? (startRect ? startRect.x : 0) : 0
                                y: startRect ? startRect.y + index * startRect.height : 0
                                width: {
                                    if (!startRect || !endRect)
                                        return 0
                                    if (index === 0)
                                        return m_text.width - x // 第一行
                                    if (index === parent.model - 1)
                                        return endRect.x + endRect.width // 最后一行
                                    return m_text.width // 中间行
                                }
                                height: startRect ? startRect.height : 0
                                z: 5 // 确保在文本下方
                            }
                        }
                    }
                }

                MouseArea {
                    id: textMouseArea
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton
                    propagateComposedEvents: true

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
                            //notePopup.showNote(noteContent)
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

            // 关键修复：添加滚动视图
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
                    spacing: contentListView.spacing
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
        width: 300
        height: 300
        font.pointSize: fontSize
        modal: true
        focus: true
        anchors.centerIn: Overlay.overlay

        background: Rectangle {
            color: notePopup.palette.window
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
                text: qsTr("Note Content:")
                font.bold: true
            }

            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true

                TextArea {
                    id: noteContent
                    readOnly: true
                    wrapMode: Text.Wrap
                }
            }

            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                Button {
                    text: qsTr("Edit")
                    onClicked: {
                        // 打开编辑弹窗，修改后更新模型
                        notesModel.set(currentNoteIndex, {
                                           "content": noteContent.text
                                       })
                        notePopup.close()
                        m_Reader.editBookNote(currentNoteIndex,
                                              noteContent.text)
                    }
                }
                Button {
                    text: qsTr("Del")
                    onClicked: {

                        //notesModel.remove(currentNoteIndex)
                        //m_Reader.delReadNote(currentNoteIndex)
                        //notePopup.close()
                        deleteConfirmDialog.open()
                    }
                }
                Button {
                    text: qsTr("Close")
                    onClicked: notePopup.close()
                }
            }
        }
    }

    Dialog {
        id: deleteConfirmDialog
        title: qsTr("Delete Confirmation")
        font.pointSize: fontSize
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel

        // 确保对话框居中显示
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2

        Label {

            text: qsTr("Are you sure you want to delete this note?")
            anchors.centerIn: parent
        }
    }

    // 在根组件添加连接
    Connections {
        target: deleteConfirmDialog
        function onAccepted() {
            notesModel.remove(currentNoteIndex)
            m_Reader.delReadNote(currentNoteIndex)
            notePopup.close()
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


        /*ListElement {
            start: 50
            end: 120
            color: "#FF572280"
            content: "这是第1条笔记的内容"
        }
        ListElement {
            start: 200
            end: 250
            color: "#4CAF5080"
            content: "这是第2条笔记的内容"
        }

        ListElement {
            start: 300
            end: 310
            color: "#8000FF50"
            content: "这是第3条笔记的内容"
        }*/
    }

    // 自动刷新笔记模型数据
    Connections {
        target: m_Reader
        onNotesLoaded: function (notes) {
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
}
