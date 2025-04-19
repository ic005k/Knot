import QtQuick 2.15
import QtQuick.Controls 2.15
import QtWebView 1.1

Item {
    id: mywebitem
    visible: true

    property bool showProgress: webView.loading && Qt.platform.os !== "ios"
                                && Qt.platform.os !== "winrt"

    property string pdfpath: ""
    property bool isVisible: true

    function setPdfPath(pdffile) {
        pdfpath = pdffile
        console.debug("pdfpath=" + pdfpath)
    }

    function setPdfVisible(vv) {
        isVisible = vv
    }

    property int m_prog: 0
    function getProg() {
        return m_prog
    }

    WebView {
        id: webView
        anchors.fill: parent
        visible: isVisible
        url: pdfpath

        onLoadProgressChanged: {

        }
    }

    Component.onCompleted: {
        console.log(webView.url)
    }
}
