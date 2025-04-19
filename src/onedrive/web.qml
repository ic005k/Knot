import QtQuick 2.2
import QtQuick.Controls 1.1
import QtWebView 1.1

import MyModel1 1.0


Item {
    id: mywebitem

    property bool showProgress: webViewOne.loading && Qt.platform.os !== "ios"
                                && Qt.platform.os !== "winrt"
    visible: true

    File {
        id: file
    }

    property int m_prog: 0
    function getProg() {
        return m_prog
    }

    WebView {
        id: webViewOne
        anchors.fill: parent
        url: initialUrl

        onLoadProgressChanged: {

            file.prog = webViewOne.loadProgress
            m_prog = webViewOne.loadProgress

            console.log(m_prog)

            if (webViewOne.loadProgress == 100) {
                console.log("loadProgress=100%")
                console.log("CurrentUrl=" + webViewOne.url)
                file.webEnd = webViewOne.url.toString()
            }
        }
    }

    Component.onCompleted: {
        console.log(webViewOne.url)
    }
}
