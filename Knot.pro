QT += core gui network printsupport
QT += charts sensors sql
QT += qml quick quickwidgets location
QT += xml svg concurrent

# 在发布构建时禁用调试支持
DEFINES += QT_NO_DEBUG QML_DISABLE_PROFILER

win32 {
    QMAKE_CFLAGS += /utf-8
    QMAKE_CXXFLAGS += /utf-8
}

!android: {
    # QT += webenginewidgets
}

android {
    #QT += webview

}

# Qt > 5 (Qt6)
greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat #statemachine
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
CONFIG+=sdk_no_version_check

TRANSLATIONS += src/cn.ts \
    lib/qsci/qscintilla_cs.ts \
    lib/qsci/qscintilla_de.ts \
    lib/qsci/qscintilla_es.ts \
    lib/qsci/qscintilla_fr.ts \
    lib/qsci/qscintilla_pt_br.ts

ICON = res/icon.icns
RC_FILE += win.rc

##################### 隔离第三方库的编译警告 ################################
# 1. 为第三方库创建单独的变量
THIRD_PARTY_PATH = $$PWD/lib
THIRD_PARTY_INCLUDE = $$THIRD_PARTY_PATH/cppjieba \
                      $$THIRD_PARTY_PATH/qsci \
                      $$THIRD_PARTY_PATH/qsci/Qsci \
                      $$THIRD_PARTY_PATH/quazip \
                      $$THIRD_PARTY_PATH/zlib \
                      $$THIRD_PARTY_PATH/cmark-gfm/include \
                      $$THIRD_PARTY_PATH/scintilla/include

# 2. 根据不同编译器设置隔离选项
win32 {
    # MSVC - 使用外部包含指令
    QMAKE_CXXFLAGS += -external:anglebrackets -external:W0
    INCLUDEPATH += $$THIRD_PARTY_INCLUDE
}
clang|gcc {
    # GCC/Clang - 使用 -isystem
    # QMAKE_CXXFLAGS += -isystem $$THIRD_PARTY_INCLUDE
}

####################### Qsci ##############################################

# 确保启用 Markdown 支持
DEFINES += SCI_LEXER
DEFINES += LEXER_MARKDOWN_INCLUDED
DEFINES += QSCINTILLA_HAVE_MARKDOWNLEXER

CONFIG += lexer_markdown

INCLUDEPATH += $$PWD/lib/scintilla/include
INCLUDEPATH += $$PWD/lib/scintilla/lexlib
INCLUDEPATH += $$PWD/lib/scintilla/src

INCLUDEPATH += $$PWD/lib/qsci/QSci
INCLUDEPATH += $$PWD/lib/qsci

macx:lessThan(QT_MAJOR_VERSION, 6) {
    QT += macextras
    LIBS += -framework AppKit -framework Cocoa
}

####################### QuaZip ##############################################

INCLUDEPATH += $$PWD/lib/zlib
DEFINES += QUAZIP_STATIC

linux {
    # 强制定义关键宏
    DEFINES += Z_HAVE_UNISTD_H HAVE_FSEEKO

    # 包含系统头文件路径
    INCLUDEPATH += /usr/include
    LIBS += -L/usr/lib
}

macx {
    # 强制定义 CMake 检测所需的宏
    DEFINES += Z_HAVE_UNISTD_H HAVE_FSEEKO

    # 直接指定 macOS SDK 路径（兼容 GitHub Actions 环境）
    SDK_PATH = $$system(xcrun --show-sdk-path)
    INCLUDEPATH += $${SDK_PATH}/usr/include

    # 确保链接器能找到系统库
    LIBS += -L$${SDK_PATH}/usr/lib
}

####################### 添加 cppjieba 和 limonp 头文件路径 ######################

INCLUDEPATH += $$PWD/lib/cppjieba/include
INCLUDEPATH += $$PWD/lib/cppjieba/limonp/include

############################ cmark-gfm ########################################

INCLUDEPATH += $$PWD/lib/cmark-gfm/include

###############################################################################

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#DEFINES += QT_DEPRECATED_WARNINGS \
#           QT_ANGLE_PLATFORM

!android {
SOURCES += \
    lib/qsci/InputMethod.cpp \
    lib/qsci/ListBoxQt.cpp \
    lib/qsci/MacPasteboardMime.cpp \
    lib/qsci/PlatQt.cpp \
    lib/qsci/SciAccessibility.cpp \
    lib/qsci/SciClasses.cpp \
    lib/qsci/ScintillaQt.cpp \
    lib/qsci/qsciabstractapis.cpp \
    lib/qsci/qsciapis.cpp \
    lib/qsci/qscicommand.cpp \
    lib/qsci/qscicommandset.cpp \
    lib/qsci/qscidocument.cpp \
    lib/qsci/qscilexer.cpp \
    lib/qsci/qscilexerasm.cpp \
    lib/qsci/qscilexeravs.cpp \
    lib/qsci/qscilexerbash.cpp \
    lib/qsci/qscilexerbatch.cpp \
    lib/qsci/qscilexercmake.cpp \
    lib/qsci/qscilexercoffeescript.cpp \
    lib/qsci/qscilexercpp.cpp \
    lib/qsci/qscilexercsharp.cpp \
    lib/qsci/qscilexercss.cpp \
    lib/qsci/qscilexercustom.cpp \
    lib/qsci/qscilexerd.cpp \
    lib/qsci/qscilexerdiff.cpp \
    lib/qsci/qscilexeredifact.cpp \
    lib/qsci/qscilexerfortran.cpp \
    lib/qsci/qscilexerfortran77.cpp \
    lib/qsci/qscilexerhex.cpp \
    lib/qsci/qscilexerhtml.cpp \
    lib/qsci/qscilexeridl.cpp \
    lib/qsci/qscilexerintelhex.cpp \
    lib/qsci/qscilexerjava.cpp \
    lib/qsci/qscilexerjavascript.cpp \
    lib/qsci/qscilexerjson.cpp \
    lib/qsci/qscilexerlua.cpp \
    lib/qsci/qscilexermakefile.cpp \
    lib/qsci/qscilexermarkdown.cpp \
    lib/qsci/qscilexermasm.cpp \
    lib/qsci/qscilexermatlab.cpp \
    lib/qsci/qscilexernasm.cpp \
    lib/qsci/qscilexeroctave.cpp \
    lib/qsci/qscilexerpascal.cpp \
    lib/qsci/qscilexerperl.cpp \
    lib/qsci/qscilexerpo.cpp \
    lib/qsci/qscilexerpostscript.cpp \
    lib/qsci/qscilexerpov.cpp \
    lib/qsci/qscilexerproperties.cpp \
    lib/qsci/qscilexerpython.cpp \
    lib/qsci/qscilexerruby.cpp \
    lib/qsci/qscilexerspice.cpp \
    lib/qsci/qscilexersql.cpp \
    lib/qsci/qscilexersrec.cpp \
    lib/qsci/qscilexertcl.cpp \
    lib/qsci/qscilexertekhex.cpp \
    lib/qsci/qscilexertex.cpp \
    lib/qsci/qscilexerverilog.cpp \
    lib/qsci/qscilexervhdl.cpp \
    lib/qsci/qscilexerxml.cpp \
    lib/qsci/qscilexeryaml.cpp \
    lib/qsci/qscimacro.cpp \
    lib/qsci/qsciprinter.cpp \
    lib/qsci/qsciscintilla.cpp \
    lib/qsci/qsciscintillabase.cpp \
    lib/qsci/qscistyle.cpp \
    lib/qsci/qscistyledtext.cpp \
    lib/scintilla/lexers/LexA68k.cpp \
    lib/scintilla/lexers/LexAPDL.cpp \
    lib/scintilla/lexers/LexASY.cpp \
    lib/scintilla/lexers/LexAU3.cpp \
    lib/scintilla/lexers/LexAVE.cpp \
    lib/scintilla/lexers/LexAVS.cpp \
    lib/scintilla/lexers/LexAbaqus.cpp \
    lib/scintilla/lexers/LexAda.cpp \
    lib/scintilla/lexers/LexAsm.cpp \
    lib/scintilla/lexers/LexAsn1.cpp \
    lib/scintilla/lexers/LexBaan.cpp \
    lib/scintilla/lexers/LexBash.cpp \
    lib/scintilla/lexers/LexBasic.cpp \
    lib/scintilla/lexers/LexBatch.cpp \
    lib/scintilla/lexers/LexBibTeX.cpp \
    lib/scintilla/lexers/LexBullant.cpp \
    lib/scintilla/lexers/LexCLW.cpp \
    lib/scintilla/lexers/LexCOBOL.cpp \
    lib/scintilla/lexers/LexCPP.cpp \
    lib/scintilla/lexers/LexCSS.cpp \
    lib/scintilla/lexers/LexCaml.cpp \
    lib/scintilla/lexers/LexCmake.cpp \
    lib/scintilla/lexers/LexCoffeeScript.cpp \
    lib/scintilla/lexers/LexConf.cpp \
    lib/scintilla/lexers/LexCrontab.cpp \
    lib/scintilla/lexers/LexCsound.cpp \
    lib/scintilla/lexers/LexD.cpp \
    lib/scintilla/lexers/LexDMAP.cpp \
    lib/scintilla/lexers/LexDMIS.cpp \
    lib/scintilla/lexers/LexDiff.cpp \
    lib/scintilla/lexers/LexECL.cpp \
    lib/scintilla/lexers/LexEDIFACT.cpp \
    lib/scintilla/lexers/LexEScript.cpp \
    lib/scintilla/lexers/LexEiffel.cpp \
    lib/scintilla/lexers/LexErlang.cpp \
    lib/scintilla/lexers/LexErrorList.cpp \
    lib/scintilla/lexers/LexFlagship.cpp \
    lib/scintilla/lexers/LexForth.cpp \
    lib/scintilla/lexers/LexFortran.cpp \
    lib/scintilla/lexers/LexGAP.cpp \
    lib/scintilla/lexers/LexGui4Cli.cpp \
    lib/scintilla/lexers/LexHTML.cpp \
    lib/scintilla/lexers/LexHaskell.cpp \
    lib/scintilla/lexers/LexHex.cpp \
    lib/scintilla/lexers/LexIndent.cpp \
    lib/scintilla/lexers/LexInno.cpp \
    lib/scintilla/lexers/LexJSON.cpp \
    lib/scintilla/lexers/LexKVIrc.cpp \
    lib/scintilla/lexers/LexKix.cpp \
    lib/scintilla/lexers/LexLPeg.cpp \
    lib/scintilla/lexers/LexLaTeX.cpp \
    lib/scintilla/lexers/LexLisp.cpp \
    lib/scintilla/lexers/LexLout.cpp \
    lib/scintilla/lexers/LexLua.cpp \
    lib/scintilla/lexers/LexMMIXAL.cpp \
    lib/scintilla/lexers/LexMPT.cpp \
    lib/scintilla/lexers/LexMSSQL.cpp \
    lib/scintilla/lexers/LexMagik.cpp \
    lib/scintilla/lexers/LexMake.cpp \
    lib/scintilla/lexers/LexMarkdown.cpp \
    lib/scintilla/lexers/LexMatlab.cpp \
    lib/scintilla/lexers/LexMaxima.cpp \
    lib/scintilla/lexers/LexMetapost.cpp \
    lib/scintilla/lexers/LexModula.cpp \
    lib/scintilla/lexers/LexMySQL.cpp \
    lib/scintilla/lexers/LexNimrod.cpp \
    lib/scintilla/lexers/LexNsis.cpp \
    lib/scintilla/lexers/LexNull.cpp \
    lib/scintilla/lexers/LexOScript.cpp \
    lib/scintilla/lexers/LexOpal.cpp \
    lib/scintilla/lexers/LexPB.cpp \
    lib/scintilla/lexers/LexPLM.cpp \
    lib/scintilla/lexers/LexPO.cpp \
    lib/scintilla/lexers/LexPOV.cpp \
    lib/scintilla/lexers/LexPS.cpp \
    lib/scintilla/lexers/LexPascal.cpp \
    lib/scintilla/lexers/LexPerl.cpp \
    lib/scintilla/lexers/LexPowerPro.cpp \
    lib/scintilla/lexers/LexPowerShell.cpp \
    lib/scintilla/lexers/LexProgress.cpp \
    lib/scintilla/lexers/LexProps.cpp \
    lib/scintilla/lexers/LexPython.cpp \
    lib/scintilla/lexers/LexR.cpp \
    lib/scintilla/lexers/LexRebol.cpp \
    lib/scintilla/lexers/LexRegistry.cpp \
    lib/scintilla/lexers/LexRuby.cpp \
    lib/scintilla/lexers/LexRust.cpp \
    lib/scintilla/lexers/LexSAS.cpp \
    lib/scintilla/lexers/LexSML.cpp \
    lib/scintilla/lexers/LexSQL.cpp \
    lib/scintilla/lexers/LexSTTXT.cpp \
    lib/scintilla/lexers/LexScriptol.cpp \
    lib/scintilla/lexers/LexSmalltalk.cpp \
    lib/scintilla/lexers/LexSorcus.cpp \
    lib/scintilla/lexers/LexSpecman.cpp \
    lib/scintilla/lexers/LexSpice.cpp \
    lib/scintilla/lexers/LexStata.cpp \
    lib/scintilla/lexers/LexTACL.cpp \
    lib/scintilla/lexers/LexTADS3.cpp \
    lib/scintilla/lexers/LexTAL.cpp \
    lib/scintilla/lexers/LexTCL.cpp \
    lib/scintilla/lexers/LexTCMD.cpp \
    lib/scintilla/lexers/LexTeX.cpp \
    lib/scintilla/lexers/LexTxt2tags.cpp \
    lib/scintilla/lexers/LexVB.cpp \
    lib/scintilla/lexers/LexVHDL.cpp \
    lib/scintilla/lexers/LexVerilog.cpp \
    lib/scintilla/lexers/LexVisualProlog.cpp \
    lib/scintilla/lexers/LexYAML.cpp \
    lib/scintilla/lexlib/Accessor.cpp \
    lib/scintilla/lexlib/CharacterCategory.cpp \
    lib/scintilla/lexlib/CharacterSet.cpp \
    lib/scintilla/lexlib/DefaultLexer.cpp \
    lib/scintilla/lexlib/LexerBase.cpp \
    lib/scintilla/lexlib/LexerModule.cpp \
    lib/scintilla/lexlib/LexerNoExceptions.cpp \
    lib/scintilla/lexlib/LexerSimple.cpp \
    lib/scintilla/lexlib/PropSetSimple.cpp \
    lib/scintilla/lexlib/StyleContext.cpp \
    lib/scintilla/lexlib/WordList.cpp \
    lib/scintilla/src/AutoComplete.cpp \
    lib/scintilla/src/CallTip.cpp \
    lib/scintilla/src/CaseConvert.cpp \
    lib/scintilla/src/CaseFolder.cpp \
    lib/scintilla/src/Catalogue.cpp \
    lib/scintilla/src/CellBuffer.cpp \
    lib/scintilla/src/CharClassify.cpp \
    lib/scintilla/src/ContractionState.cpp \
    lib/scintilla/src/DBCS.cpp \
    lib/scintilla/src/Decoration.cpp \
    lib/scintilla/src/Document.cpp \
    lib/scintilla/src/EditModel.cpp \
    lib/scintilla/src/EditView.cpp \
    lib/scintilla/src/Editor.cpp \
    lib/scintilla/src/Indicator.cpp \
    lib/scintilla/src/KeyMap.cpp \
    lib/scintilla/src/LineMarker.cpp \
    lib/scintilla/src/MarginView.cpp \
    lib/scintilla/src/PerLine.cpp \
    lib/scintilla/src/PositionCache.cpp \
    lib/scintilla/src/RESearch.cpp \
    lib/scintilla/src/RunStyles.cpp \
    lib/scintilla/src/ScintillaBase.cpp \
    lib/scintilla/src/Selection.cpp \
    lib/scintilla/src/Style.cpp \
    lib/scintilla/src/UniConversion.cpp \
    lib/scintilla/src/ViewStyle.cpp \
    lib/scintilla/src/XPM.cpp
}

SOURCES += \
    lib/zlib/adler32.c \
    lib/zlib/compress.c \
    lib/zlib/crc32.c \
    lib/zlib/deflate.c \
    lib/zlib/gzclose.c \
    lib/zlib/gzlib.c \
    lib/zlib/gzread.c \
    lib/zlib/gzwrite.c \
    lib/zlib/infback.c \
    lib/zlib/inffast.c \
    lib/zlib/inflate.c \
    lib/zlib/inftrees.c \
    lib/zlib/trees.c \
    lib/zlib/uncompr.c \
    lib/zlib/zutil.c \
    src/AboutThis.cpp \
    src/AutoUpdate.cpp \
    src/CategoryList.cpp \
    src/CloudBackup.cpp \
    src/Comm/DatePicker.cpp \
    src/Comm/Method.cpp \
    src/Comm/ReceiveShare.cpp \
    src/Comm/ShowMessage.cpp \
    src/Comm/Time24Picker.cpp \
    src/Comm/WheelWidget.cpp \
    src/Comm/inputmethodreset.cpp \
    src/Comm/qaesencryption.cpp \
    src/DateSelector.cpp \
    src/EditRecord.cpp \
    src/JavaToQtBridge.cpp \
    src/LoadPic.cpp \
    src/MainHelper.cpp \
    src/MainWindow.cpp \
    src/MyThread.cpp \
    src/Notes/ColorDialog.cpp \
    src/Notes/MarkdownToHtml.cpp \
    src/Notes/MoveTo.cpp \
    src/Notes/NewNoteBook.cpp \
    src/Notes/Notes.cpp \
    src/Notes/NotesList.cpp \
    src/Notes/PrintPDF.cpp \
    src/Notes/database_manager.cpp \
    src/Notes/note_graph.cpp \
    src/Notes/search_model.cpp \
    src/Notes/titlegenerator.cpp \
    src/Preferences.cpp \
    src/Reader/DocumentHandler.cpp \
    src/Reader/File.cpp \
    src/Reader/PageIndicator.cpp \
    src/Reader/Reader.cpp \
    src/Reader/ReaderSet.cpp \
    src/Reader/SetReaderText.cpp \
    src/Report.cpp \
    src/Exercise/Speedometer.cpp \
    src/Exercise/Steps.cpp \
    src/Exercise/StepsOptions.cpp \
    src/Todo/Todo.cpp \
    src/Todo/TodoAlarm.cpp \
    lib/cmark-gfm/extensions/autolink.c \
    lib/cmark-gfm/extensions/core-extensions.c \
    lib/cmark-gfm/extensions/ext_scanners.c \
    lib/cmark-gfm/extensions/strikethrough.c \
    lib/cmark-gfm/extensions/table.c \
    lib/cmark-gfm/extensions/tagfilter.c \
    lib/cmark-gfm/extensions/tasklist.c \
    lib/cmark-gfm/src/arena.c \
    lib/cmark-gfm/src/blocks.c \
    lib/cmark-gfm/src/buffer.c \
    lib/cmark-gfm/src/cmark.c \
    lib/cmark-gfm/src/cmark_ctype.c \
    lib/cmark-gfm/src/commonmark.c \
    lib/cmark-gfm/src/footnotes.c \
    lib/cmark-gfm/src/houdini_href_e.c \
    lib/cmark-gfm/src/houdini_html_e.c \
    lib/cmark-gfm/src/houdini_html_u.c \
    lib/cmark-gfm/src/html.c \
    lib/cmark-gfm/src/inlines.c \
    lib/cmark-gfm/src/iterator.c \
    lib/cmark-gfm/src/latex.c \
    lib/cmark-gfm/src/linked_list.c \
    lib/cmark-gfm/src/man.c \
    lib/cmark-gfm/src/map.c \
    lib/cmark-gfm/src/node.c \
    lib/cmark-gfm/src/plaintext.c \
    lib/cmark-gfm/src/plugin.c \
    lib/cmark-gfm/src/references.c \
    lib/cmark-gfm/src/registry.c \
    lib/cmark-gfm/src/render.c \
    lib/cmark-gfm/src/scanners.c \
    lib/cmark-gfm/src/syntax_extension.c \
    lib/cmark-gfm/src/utf8.c \
    lib/cmark-gfm/src/xml.c \
    src/main.cpp \
    lib/quazip/JlCompress.cpp \
    lib/quazip/qioapi.cpp \
    lib/quazip/quaadler32.cpp \
    lib/quazip/quachecksum32.cpp \
    lib/quazip/quacrc32.cpp \
    lib/quazip/quagzipfile.cpp \
    lib/quazip/quaziodevice.cpp \
    lib/quazip/quazip.cpp \
    lib/quazip/quazipdir.cpp \
    lib/quazip/quazipfile.cpp \
    lib/quazip/quazipfileinfo.cpp \
    lib/quazip/quazipnewinfo.cpp \
    lib/quazip/unzip.c \
    lib/quazip/zip.c \

!android {
HEADERS += \
    lib/qsci/ListBoxQt.h \
    lib/qsci/Qsci/qsciabstractapis.h \
    lib/qsci/Qsci/qsciapis.h \
    lib/qsci/Qsci/qscicommand.h \
    lib/qsci/Qsci/qscicommandset.h \
    lib/qsci/Qsci/qscidocument.h \
    lib/qsci/Qsci/qsciglobal.h \
    lib/qsci/Qsci/qscilexer.h \
    lib/qsci/Qsci/qscilexerasm.h \
    lib/qsci/Qsci/qscilexeravs.h \
    lib/qsci/Qsci/qscilexerbash.h \
    lib/qsci/Qsci/qscilexerbatch.h \
    lib/qsci/Qsci/qscilexercmake.h \
    lib/qsci/Qsci/qscilexercoffeescript.h \
    lib/qsci/Qsci/qscilexercpp.h \
    lib/qsci/Qsci/qscilexercsharp.h \
    lib/qsci/Qsci/qscilexercss.h \
    lib/qsci/Qsci/qscilexercustom.h \
    lib/qsci/Qsci/qscilexerd.h \
    lib/qsci/Qsci/qscilexerdiff.h \
    lib/qsci/Qsci/qscilexeredifact.h \
    lib/qsci/Qsci/qscilexerfortran.h \
    lib/qsci/Qsci/qscilexerfortran77.h \
    lib/qsci/Qsci/qscilexerhex.h \
    lib/qsci/Qsci/qscilexerhtml.h \
    lib/qsci/Qsci/qscilexeridl.h \
    lib/qsci/Qsci/qscilexerintelhex.h \
    lib/qsci/Qsci/qscilexerjava.h \
    lib/qsci/Qsci/qscilexerjavascript.h \
    lib/qsci/Qsci/qscilexerjson.h \
    lib/qsci/Qsci/qscilexerlua.h \
    lib/qsci/Qsci/qscilexermakefile.h \
    lib/qsci/Qsci/qscilexermarkdown.h \
    lib/qsci/Qsci/qscilexermasm.h \
    lib/qsci/Qsci/qscilexermatlab.h \
    lib/qsci/Qsci/qscilexernasm.h \
    lib/qsci/Qsci/qscilexeroctave.h \
    lib/qsci/Qsci/qscilexerpascal.h \
    lib/qsci/Qsci/qscilexerperl.h \
    lib/qsci/Qsci/qscilexerpo.h \
    lib/qsci/Qsci/qscilexerpostscript.h \
    lib/qsci/Qsci/qscilexerpov.h \
    lib/qsci/Qsci/qscilexerproperties.h \
    lib/qsci/Qsci/qscilexerpython.h \
    lib/qsci/Qsci/qscilexerruby.h \
    lib/qsci/Qsci/qscilexerspice.h \
    lib/qsci/Qsci/qscilexersql.h \
    lib/qsci/Qsci/qscilexersrec.h \
    lib/qsci/Qsci/qscilexertcl.h \
    lib/qsci/Qsci/qscilexertekhex.h \
    lib/qsci/Qsci/qscilexertex.h \
    lib/qsci/Qsci/qscilexerverilog.h \
    lib/qsci/Qsci/qscilexervhdl.h \
    lib/qsci/Qsci/qscilexerxml.h \
    lib/qsci/Qsci/qscilexeryaml.h \
    lib/qsci/Qsci/qscimacro.h \
    lib/qsci/Qsci/qsciprinter.h \
    lib/qsci/Qsci/qsciscintilla.h \
    lib/qsci/Qsci/qsciscintillabase.h \
    lib/qsci/Qsci/qscistyle.h \
    lib/qsci/Qsci/qscistyledtext.h \
    lib/qsci/SciAccessibility.h \
    lib/qsci/SciClasses.h \
    lib/qsci/ScintillaQt.h \
    lib/scintilla/include/ILexer.h \
    lib/scintilla/include/ILoader.h \
    lib/scintilla/include/Platform.h \
    lib/scintilla/include/SciLexer.h \
    lib/scintilla/include/Sci_Position.h \
    lib/scintilla/include/Scintilla.h \
    lib/scintilla/include/ScintillaWidget.h \
    lib/scintilla/lexlib/Accessor.h \
    lib/scintilla/lexlib/CharacterCategory.h \
    lib/scintilla/lexlib/CharacterSet.h \
    lib/scintilla/lexlib/DefaultLexer.h \
    lib/scintilla/lexlib/LexAccessor.h \
    lib/scintilla/lexlib/LexerBase.h \
    lib/scintilla/lexlib/LexerModule.h \
    lib/scintilla/lexlib/LexerNoExceptions.h \
    lib/scintilla/lexlib/LexerSimple.h \
    lib/scintilla/lexlib/OptionSet.h \
    lib/scintilla/lexlib/PropSetSimple.h \
    lib/scintilla/lexlib/SparseState.h \
    lib/scintilla/lexlib/StringCopy.h \
    lib/scintilla/lexlib/StyleContext.h \
    lib/scintilla/lexlib/SubStyles.h \
    lib/scintilla/lexlib/WordList.h \
    lib/scintilla/src/AutoComplete.h \
    lib/scintilla/src/CallTip.h \
    lib/scintilla/src/CaseConvert.h \
    lib/scintilla/src/CaseFolder.h \
    lib/scintilla/src/Catalogue.h \
    lib/scintilla/src/CellBuffer.h \
    lib/scintilla/src/CharClassify.h \
    lib/scintilla/src/ContractionState.h \
    lib/scintilla/src/DBCS.h \
    lib/scintilla/src/Decoration.h \
    lib/scintilla/src/Document.h \
    lib/scintilla/src/EditModel.h \
    lib/scintilla/src/EditView.h \
    lib/scintilla/src/Editor.h \
    lib/scintilla/src/ElapsedPeriod.h \
    lib/scintilla/src/ExternalLexer.h \
    lib/scintilla/src/FontQuality.h \
    lib/scintilla/src/Indicator.h \
    lib/scintilla/src/IntegerRectangle.h \
    lib/scintilla/src/KeyMap.h \
    lib/scintilla/src/LineMarker.h \
    lib/scintilla/src/MarginView.h \
    lib/scintilla/src/Partitioning.h \
    lib/scintilla/src/PerLine.h \
    lib/scintilla/src/Position.h \
    lib/scintilla/src/PositionCache.h \
    lib/scintilla/src/RESearch.h \
    lib/scintilla/src/RunStyles.h \
    lib/scintilla/src/ScintillaBase.h \
    lib/scintilla/src/Selection.h \
    lib/scintilla/src/SparseVector.h \
    lib/scintilla/src/SplitVector.h \
    lib/scintilla/src/Style.h \
    lib/scintilla/src/UniConversion.h \
    lib/scintilla/src/UniqueString.h \
    lib/scintilla/src/ViewStyle.h \
    lib/scintilla/src/XPM.h
}

HEADERS += \
    lib/zlib/crc32.h \
    lib/zlib/deflate.h \
    lib/zlib/gzguts.h \
    lib/zlib/inffast.h \
    lib/zlib/inffixed.h \
    lib/zlib/inflate.h \
    lib/zlib/inftrees.h \
    lib/zlib/trees.h \
    lib/zlib/zconf.h \
    lib/zlib/zlib.h \
    lib/zlib/zutil.h \
    src/AboutThis.h \
    src/AutoUpdate.h \
    src/CategoryList.h \
    src/CloudBackup.h \
    src/Comm/DatePicker.h \
    src/Comm/Method.h \
    src/Comm/ReceiveShare.h \
    src/Comm/ShowMessage.h \
    src/Comm/TextEditToolbar.h \
    src/Comm/Time24Picker.h \
    src/Comm/WheelWidget.h \
    src/Comm/inputmethodreset.h \
    src/Comm/qaesencryption.h \
    src/DateSelector.h \
    src/EditRecord.h \
    src/Exercise/WeatherFetcher.h \
    src/LoadPic.h \
    src/MainHelper.h \
    src/MainWindow.h \
    src/MyThread.h \
    src/Notes/ColorDialog.h \
    src/Notes/MoveTo.h \
    src/Notes/NewNoteBook.h \
    src/Notes/Notes.h \
    src/Notes/NotesList.h \
    src/Notes/PrintPDF.h \
    src/Notes/database_manager.h \
    src/Notes/note_graph.h \
    src/Notes/search_model.h \
    src/Notes/titlegenerator.h \
    src/Preferences.h \
    src/Reader/DocumentHandler.h \
    src/Reader/File.h \
    src/Reader/PageIndicator.h \
    src/Reader/Reader.h \
    src/Reader/ReaderSet.h \
    src/Reader/SetReaderText.h \
    src/Report.h \
    src/Exercise/Speedometer.h \
    src/Exercise/Steps.h \
    src/Exercise/StepsOptions.h \
    src/SplashTimer.h \
    src/Todo/Todo.h \
    src/Todo/TodoAlarm.h \
    lib/quazip/JlCompress.h \
    lib/quazip/ioapi.h \
    lib/quazip/minizip_crypt.h \
    lib/quazip/quaadler32.h \
    lib/quazip/quachecksum32.h \
    lib/quazip/quacrc32.h \
    lib/quazip/quagzipfile.h \
    lib/quazip/quaziodevice.h \
    lib/quazip/quazip.h \
    lib/quazip/quazip_global.h \
    lib/quazip/quazip_qt_compat.h \
    lib/quazip/quazipdir.h \
    lib/quazip/quazipfile.h \
    lib/quazip/quazipfileinfo.h \
    lib/quazip/quazipnewinfo.h \
    lib/quazip/unzip.h \
    lib/quazip/zip.h \
    win.rc

FORMS += \
    src/AboutThis.ui \
    src/AutoUpdate.ui \
    src/CategoryList.ui \
    src/CloudBackup.ui \
    src/Comm/ShowMessage.ui \
    src/DateSelector.ui \
    src/MainWindow.ui \
    src/Notes/MoveTo.ui \
    src/Notes/NewNoteBook.ui \
    src/Notes/Notes.ui \
    src/Notes/NotesList.ui \
    src/Notes/PrintPDF.ui \
    src/Preferences.ui \
    src/Reader/PageIndicator.ui \
    src/Reader/SetReaderText.ui \
    src/Exercise/StepsOptions.ui \
    src/Todo/Todo.ui \
    src/Todo/TodoAlarm.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    knotqml.qrc \
    res.qrc \


CONFIG(debug,debug|release) {
    DESTDIR = $$absolute_path($${_PRO_FILE_PWD_}/bin/debug)
} else {
    DESTDIR = $$absolute_path($${_PRO_FILE_PWD_}/bin/release)
}

DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/build.gradle \
    android/gradle.properties \
    android/gradle.properties \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew \
    android/gradlew.bat \
    android/gradlew.bat \
    android/res/values/libs.xml \
    android/res/values/libs.xml \
    android/res/xml/qtprovider_paths.xml \
    lib/lexilla/include/LexicalStyles.iface \
    lib/qsci/features/qscintilla2.prf \
    lib/qsci/features_staticlib/qscintilla2.prf \
    lib/qsci/qscintilla_cs.qm \
    lib/qsci/qscintilla_de.qm \
    lib/qsci/qscintilla_es.qm \
    lib/qsci/qscintilla_fr.qm \
    lib/qsci/qscintilla_pt_br.qm \
    lib/scintilla/include/License.txt \
    lib/scintilla/include/Scintilla.iface \
    lib/scintilla/lexers/License.txt \
    lib/scintilla/lexlib/License.txt \
    lib/scintilla/src/License.txt \
    lib/scintilla/src/SciTE.properties \
    src/cn.qm \
    src/cn.ts \
    lib/quazip/quazip.pc.cmakein


ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

####################### Qsci ##############################################

# 排除扩展语法文件
SOURCES -= $$PWD/lib/scintilla/src/ExternalLexer.cpp

######################### OpenSSL ########################################

# 链接 OpenSSL 库（根据平台配置）
win32 {
    INCLUDEPATH += $$PWD/openssl
    LIBS += -L$$PWD/openssl/lib -llibcrypto -llibssl
}

android: {
    INCLUDEPATH += $$PWD/android-openssl/include

    contains(ANDROID_TARGET_ARCH, x86_64) {
        LIBS += -L$$PWD/android-openssl/ssl_3/x86_64 \
                -lssl -lcrypto
    }
    contains(ANDROID_TARGET_ARCH, arm64-v8a) {
        LIBS += -L$$PWD/android-openssl/ssl_3/v8a \
                -lssl -lcrypto
    }


}

unix:!macx {
    LIBS += -lssl -lcrypto
}

macx {
    isEmpty(OPENSSL_PREFIX) {
        OPENSSL_PREFIX = $$system(brew --prefix openssl)
    }
    INCLUDEPATH += $${OPENSSL_PREFIX}/include
    LIBS += -L$${OPENSSL_PREFIX}/lib -lssl -lcrypto

}

###########################################################################
#Linux
unix:!macx: {

}

win32:{

}

#android: include(/home/zh/文档/android_openssl-master/openssl.pri)

contains(ANDROID_TARGET_ARCH,arm64-v8a) {
    ANDROID_EXTRA_LIBS = \
        $$PWD/android-openssl/ssl_3/v8a/libcrypto_3.so \
        $$PWD/android-openssl/ssl_3/v8a/libssl_3.so
}
#android: include(C:/Users/Administrator/Documents/android_openssl-master/openssl.pri)
