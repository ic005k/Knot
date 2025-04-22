import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls.Fusion

import MyModel2 1.0

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

    property variant stringList: null

    function updateText(str) {
        stringList = str.split('</html>')
    }

    function getText() {
        return textArea.text
    }

    function setText(str) {
        textArea.text = str
    }

    function loadText(str) {
        strText = str
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

        strText = str
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

        document.loadBuffer(strhtml)
        isEPUBText = true
    }

    function setVPos(vpos) {
        flickable.contentY = vpos
        // console.log(vpos)
    }

    function getVPos() {

        return flickable.contentY
    }

    function getVHeight() {

        return textArea.contentHeight
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
        var x = 0
        var y = flickable.contentY

        var start = textArea.positionAt(x, y + FontSize)
        var end = textArea.positionAt(x + textArea.width, y + FontSize * 6)
        var txt = textArea.getText(start, end)

        if (txt === "") {
            txt = "Bookmark"
        }

        console.log("bookmark txt=" + txt + "  x=" + x + "  start=" + start + "  end=" + end)

        return txt
    }

    function setTextAreaCursorPos(nCursorPos) {
        textArea.cursorPosition = nCursorPos
    }

    DocumentHandler {
        id: document
        objectName: "dochandler"
        document: textArea.textDocument
        cursorPosition: textArea.cursorPosition
        selectionStart: textArea.selectionStart
        selectionEnd: textArea.selectionEnd

        onLoaded: function (text) {
            textArea.text = text
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

    Flickable {
        id: flickable
        flickableDirection: Flickable.VerticalFlick
        anchors.fill: parent

        boundsBehavior: Flickable.StopAtBounds
        maximumFlickVelocity: 1500 // 降低滚动速度更顺滑

        // 自带的默认的滚动条，外观一般，不可定制
        // ScrollBar.vertical: ScrollBar {}
        states: State {
            name: "autoscroll"
            PropertyChanges {
                target: flickable
            }
        }

        onMovementEnded: {
            state = "autoscroll"
        }

        TextArea.flickable: TextArea {
            id: textArea

            visible: isEPUBText
            font.pixelSize: FontSize
            font.family: FontName
            font.weight: FontWeight
            font.letterSpacing: 2

            //renderType: Text.NativeRendering
            renderType: Text.QtRendering // 在Qt6下，效果和性能要好很多

            font.hintingPreference: Font.PreferVerticalHinting
            textFormat: Qt.AutoText
            cursorPosition: 0

            wrapMode: TextArea.Wrap
            readOnly: true
            focus: true
            persistentSelection: isSelText
            selectByMouse: isSelText
            smooth: true
            color: myTextColor
            text: strText

            onTextChanged: {
                Qt.callLater(() => {
                                 flickable.contentY = 0
                                 flickable.contentHeight = contentHeight + 20 // 加边距
                             })
            }

            onLinkActivated: {

                console.log("reader htmlPath=" + htmlPath)
                console.log("reader Link=" + link)
                document.setBackDir(link)
                document.parsingLink(link, "reader")
            }


            /*MouseArea {
                id: mouse_area
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.LeftButton
                propagateComposedEvents: true // 是否将事件传递给父元素

                onClicked: {

                    var my = mouseY - flickable.contentY
                    console.log("clicked..." + mouseY + " " + my + myH)

                    if (my < myH / 3) {
                        m_Reader.setPageScroll0()
                    }

                    if (my > myH / 3 && my < (myH * 2) / 3) {
                        m_Reader.setPanelVisible()
                    }

                    if (my > (myH * 2) / 3) {
                        m_Reader.setPageScroll1()
                    }
                }
                onDoubleClicked: {
                    console.log("double...")
                    m_Reader.selectText()
                }
                onPressAndHold: {
                    console.log("press and hold...")
                }
            }*/
            PropertyAnimation on x {
                easing.type: Easing.Linear
                running: isAni
                from: aniW
                to: 0
                duration: 200
                loops: 1
            }

            SequentialAnimation on opacity {
                //应用于透明度上的序列动画
                running: false
                loops: 1 //Animation.Infinite //无限循环
                NumberAnimation {
                    from: 0
                    to: 1
                    duration: 1000
                } //淡出效果
                PauseAnimation {
                    duration: 0
                } //暂停400ms
            }
        }

        ScrollBar.vertical: ScrollBar {
            id: vbar
            policy: ScrollBar.AsNeeded
            interactive: false // 关键！禁止拖动操作
            width: 10

            // 关键：size 绑定到可视区域比例
            size: flickable.visibleArea.heightRatio
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

    ListView {
        id: idContentListView
        model: stringList
        visible: false

        anchors {
            fill: parent
            margins: 2
        }
        delegate: Text {

            anchors {
                left: parent.left
                right: parent.right
            }

            Layout.preferredWidth: parent.width
            textFormat: Qt.AutoText //Text.PlainText
            wrapMode: Text.Wrap
            font.pixelSize: FontSize
            font.family: FontName
            font.weight: FontWeight
            font.letterSpacing: 2
            color: myTextColor

            text: model.modelData
        }
        ScrollBar.vertical: ScrollBar {}
    }
}
