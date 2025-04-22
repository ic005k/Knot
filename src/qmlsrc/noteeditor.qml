/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick
import QtQuick.Controls
import QtQuick.Window
import Qt.labs.platform

import MyModel2 1.0

// TODO:
// - make designer-friendly

Rectangle {
    id: window
    width: 1024
    height: 600
    visible: true

    //property bool isAndroid: true

    function setVPos(vpos) {
        flickable.contentY = vpos
        console.log(vpos)
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

    function getText(){
        return textArea.text
    }

    function insertStr(str){
        var pos = textArea.cursorPosition
        textArea.insert(pos,str)
    }

    function appendStr(str)
    {
        textArea.append(str)
    }

    function getCursorPos()
    {
        return textArea.cursorPosition
    }

    function setCursorPos(pos)
    {
        textArea.moveCursorSelection(pos)
    }

    function redo(){
        textArea.redo()
    }

    function undo(){
        textArea.undo()
    }

    function copy()
    {
        textArea.copy()
    }

    function cut()
    {
        textArea.cut()

    }

    function paste()
    {
        textArea.paste()
    }


    Shortcut {
        sequence: StandardKey.Open
        onActivated: openDialog.open()
    }
    Shortcut {
        sequence: StandardKey.SaveAs
        onActivated: saveDialog.open()
    }
    Shortcut {
        sequence: StandardKey.Quit
        onActivated: close()
    }
    Shortcut {
        sequence: StandardKey.Copy
        onActivated: textArea.copy()
    }
    Shortcut {
        sequence: StandardKey.Cut
        onActivated: textArea.cut()
    }
    Shortcut {
        sequence: StandardKey.Paste
        onActivated: textArea.paste()
    }
    Shortcut {
        sequence: StandardKey.Bold
        onActivated: document.bold = !document.bold
    }
    Shortcut {
        sequence: StandardKey.Italic
        onActivated: document.italic = !document.italic
    }
    Shortcut {
        sequence: StandardKey.Underline
        onActivated: document.underline = !document.underline
    }

    MenuBar {
        Menu {
            title: qsTr("&File")

            MenuItem {
                text: qsTr("&Open")
                onTriggered: openDialog.open()
            }
            MenuItem {
                text: qsTr("&Save As...")
                onTriggered: saveDialog.open()
            }
            MenuItem {
                text: qsTr("&Quit")
                onTriggered: close()
            }
        }

        Menu {
            title: qsTr("&Edit")

            MenuItem {
                text: qsTr("&Copy")
                enabled: textArea.selectedText
                onTriggered: textArea.copy()
            }
            MenuItem {
                text: qsTr("Cu&t")
                enabled: textArea.selectedText
                onTriggered: textArea.cut()
            }
            MenuItem {
                text: qsTr("&Paste")
                enabled: textArea.canPaste
                onTriggered: textArea.paste()
            }
        }

        Menu {
            title: qsTr("F&ormat")

            MenuItem {
                text: qsTr("&Bold")
                checkable: true
                checked: document.bold
                onTriggered: document.bold = !document.bold
            }
            MenuItem {
                text: qsTr("&Italic")
                checkable: true
                checked: document.italic
                onTriggered: document.italic = !document.italic
            }
            MenuItem {
                text: qsTr("&Underline")
                checkable: true
                checked: document.underline
                onTriggered: document.underline = !document.underline
            }
        }
    }

    FileDialog {
        id: openDialog
        fileMode: FileDialog.OpenFile
        selectedNameFilter.index: 1
        nameFilters: ["Text files (*.txt)", "HTML files (*.html *.htm)", "Markdown files (*.md *.markdown)"]
        folder: StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
        onAccepted: document.load(file)
    }

    FileDialog {
        id: saveDialog
        fileMode: FileDialog.SaveFile
        defaultSuffix: document.fileType
        nameFilters: openDialog.nameFilters
        selectedNameFilter.index: document.fileType === "txt" ? 0 : 1
        folder: StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
        onAccepted: document.saveAs(file)
    }

    FontDialog {
        id: fontDialog
        onAccepted: {
            document.fontFamily = font.family;
            document.fontSize = font.pointSize;
        }
    }

    ColorDialog {
        id: colorDialog
        currentColor: "black"
    }

    MessageDialog {
        id: errorDialog
    }

    MessageDialog {
        id : quitDialog
        title: qsTr("Quit?")
        text: qsTr("The file has been modified. Quit anyway?")
        buttons: (MessageDialog.Yes | MessageDialog.No)
        onYesClicked: Qt.quit()
    }


    DocumentHandler {
        id: document
        document: textArea.textDocument
        cursorPosition: textArea.cursorPosition
        selectionStart: textArea.selectionStart
        selectionEnd: textArea.selectionEnd
        textColor: colorDialog.color
        Component.onCompleted: {

        }
        onLoaded: {
            textArea.textFormat = format
            textArea.text = text
        }
        onError: {
            errorDialog.text = message
            errorDialog.visible = true
        }
    }

    Flickable {
        id: flickable
        flickableDirection: Flickable.VerticalFlick
        anchors.fill: parent


        TextArea.flickable: TextArea {
            id: textArea
            textFormat: Qt.AutoText
            wrapMode: TextArea.Wrap
            focus: true
            selectByMouse: isByMouseSelect
            persistentSelection: true

            leftPadding: 6
            rightPadding: 6
            topPadding: 0
            bottomPadding: 0
            background: null
            text: myEditText

            MouseArea {

                propagateComposedEvents: false
                acceptedButtons:Qt.RightButton//  isAndroid? Qt.LeftButton : Qt.RightButton
                anchors.fill: parent


                onClicked:{
                    console.log("isAndroid=" + isAndroid)
                    if(!isAndroid)
                    contextMenu.open()
                }


                onPressAndHold: {
                    if(isAndroid){
                    textArea.selectByMouse = true;
                    m_Notes.showTextSelector()
                    }
                }
            }



            onLinkActivated: Qt.openUrlExternally(link)
        }

        ScrollBar.vertical: ScrollBar {}
    }

    Menu {
        id: contextMenu

        MenuItem {
            text: qsTr("Copy")
            enabled: textArea.selectedText
            onTriggered: textArea.copy()
        }
        MenuItem {
            text: qsTr("Cut")
            enabled: textArea.selectedText
            onTriggered: textArea.cut()
        }
        MenuItem {
            text: qsTr("Paste")
            enabled: textArea.canPaste
            onTriggered: textArea.paste()
        }

        MenuSeparator {}

        MenuItem {
            text: qsTr("Font...")
            onTriggered: fontDialog.open()
        }

        MenuItem {
            text: qsTr("Color...")
            onTriggered: colorDialog.open()
        }
    }


}
