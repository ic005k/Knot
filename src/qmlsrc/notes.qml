import QtQuick 2.15
import QtQuick.Controls 2.15
import QtWebChannel 1.0
import QtWebEngine 1.10
import Qt.labs.settings 1.1
import MyModel2 1.0

Item {
    id: textitem
    visible: true

    property string strMDFileName: ""

    function loadHtml(htmlfile) {
        document.load("file://" + htmlfile)
    }

    function loadHtmlBuffer(strhtml) {

        document.loadBuffer(strhtml)
    }

    function setVPos(vpos) {
        if (vpos < 0)
            vpos = 0
        flickable.contentY = vpos
        console.debug("set vpos=" + vpos)
    }

    function getVPos() {
        console.debug("get vpos=" + flickable.contentY)
        return flickable.contentY
    }

    function getVHeight() {

        return flickable.contentHeight
    }

    function move(x0) {
        x = x + x0
        file.curX = x
        console.log(file.curX)
    }

    function setX(x0) {
        x = 0
    }

    function getText() {
        file.source = textArea.text
    }

    function getFileNameWithoutExtension(filePath) {
        // 先找到最后一个斜杠的位置
        const lastSlashIndex = filePath.lastIndexOf('/')
        // 提取文件名部分
        const fileName = lastSlashIndex !== -1 ? filePath.slice(
                                                     lastSlashIndex + 1) : filePath
        // 找到文件名中最后一个点的位置
        const lastDotIndex = fileName.lastIndexOf('.')
        // 提取不包含扩展名的文件名
        return lastDotIndex !== -1 ? fileName.slice(0, lastDotIndex) : fileName
    }

    function setWebViewFile(htmlfile, currentMDFile) {
        webView.url = Qt.resolvedUrl("file:///" + htmlfile)

        strMDFileName = getFileNameWithoutExtension(currentMDFile)
        console.log("strMDFileName=" + strMDFileName)
        console.log("url=" + webView.url)
    }

    function goBack() {
        if (webView.canGoBack) {
            webView.goBack()
        }
    }

    function saveWebScrollPos(mdFileName) {
        strMDFileName = mdFileName
        webView.runJavaScript("window.scrollY;", function (result) {
            appSettings.setValue(strMDFileName, result)
        })

        console.log("strMDFileName=" + strMDFileName)
    }

    DocumentHandler {
        id: document
        objectName: "dochandler"
        document: textArea.textDocument
        cursorPosition: textArea.cursorPosition
        selectionStart: textArea.selectionStart
        selectionEnd: textArea.selectionEnd

        onLoaded: {
            textArea.text = text
            console.debug("Load Notes Done...")
            m_Notes.setVPos()
        }
        onError: {
            errorDialog.text = message
            errorDialog.visible = true
        }
    }

    Settings {
        id: appSettings
        category: "WebViewScroll"
    }

    WebEngineView {
        id: webView
        anchors.fill: parent
        url: "file:///C:/Users/Administrator/.Knot/memo.html"

        // 处理导航请求
        onNavigationRequested: function (request) {
            // 判断是否为用户点击链接
            if (request.navigationType === WebEngineNavigationRequest.LinkClickedNavigation) {
                console.log("拦截链接:", request.url)

                // 阻止WebEngineView内部处理
                request.action = WebEngineNavigationRequest.IgnoreRequest

                // 使用第三方处理链接（例如用系统浏览器打开）
                Qt.openUrlExternally(request.url)

                // 或者调用自定义C++处理函数
                // externalHandler.handleUrl(request.url);
            } else {
                // 允许其他导航（如页面加载、表单提交等）
                request.action = WebEngineNavigationRequest.AcceptRequest
            }
        }

        // 恢复滚动位置
        onLoadingChanged: {
            if (loadRequest.status === WebEngineLoadRequest.LoadSucceededStatus) {
                // 延迟 500ms 确保内容渲染完成
                timer.restart()
            }
        }

        Timer {
            id: timer
            interval: 200
            onTriggered: {
                var savedPosition = appSettings.value(strMDFileName, 0)
                webView.runJavaScript(
                            "window.scrollTo(0, " + savedPosition + ");")
            }
        }
    }

    Flickable {
        id: flickable
        flickableDirection: Flickable.VerticalFlick
        anchors.fill: parent
        clip: true
        visible: false

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
            visible: true

            renderType: Text.NativeRendering
            font.hintingPreference: Font.PreferVerticalHinting
            textFormat: Qt.AutoText
            wrapMode: TextArea.Wrap
            readOnly: true
            focus: true
            persistentSelection: false
            selectByKeyboard: true
            selectByMouse: false
            smooth: true

            color: isDark ? "#FFFFFF" : "#664E30"

            background: Rectangle {
                color: isDark ? "#19232d" : "#FFFFFF"
                radius: 0
            }

            onLinkActivated: {

                document.parsingLink(link, "note")

                console.debug("qml link=" + link)
            }

            Component.onCompleted: {
                console.debug("Load Notes Component Completed...")
            }
        }

        ScrollBar.vertical: ScrollBar {
            id: control
            size: 0.3
            position: 0.2
            width: 7
            active: true
            orientation: Qt.Vertical
            anchors.right: parent.right
            policy: ScrollBar.AsNeeded
        }
    }
}
