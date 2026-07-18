CONFIG += c++17 sdk_no_version_check

win32 {
    QMAKE_CXXFLAGS += /utf-8 /std:c++17
    QMAKE_CXXFLAGS_DEBUG += /std:c++17
    QMAKE_CXXFLAGS_RELEASE += /std:c++17

    DEFINES += WIN32_LEAN_AND_MEAN
    DEFINES += VC_EXTRALEAN
    DEFINES += NOMINMAX

}

unix {
    QMAKE_CXXFLAGS += -std=c++17
}


###################################################################################

QT += core gui network printsupport
QT += charts sensors sql
QT += qml quick quickwidgets location
QT += xml svg concurrent



####################################Linux Build Android##############################
# 仅满足两个条件才启用：
# 1. 编译目标平台是 Android
# 2. 当前编译宿主机系统为 Linux（GitHub ubuntu runner）
android:unix:!macx {
    CONFIG += no_pkg_config
    DEFINES += Z_HAVE_UNISTD_H HAVE_FSEEKO

    # 用 ENV 读取运行时环境变量，兼容 CI 动态注入 NDK 路径
    NDK_ROOT = $${ENV.ANDROID_NDK_ROOT}

    !isEmpty(NDK_ROOT) {
        SYSROOT = $${NDK_ROOT}/toolchains/llvm/prebuilt/linux-x86_64/sysroot
        # 校验 sysroot 目录存在再追加参数，避免编译报错
        exists($$SYSROOT) {
            QMAKE_CFLAGS   += --sysroot=$$SYSROOT
            QMAKE_CXXFLAGS += --sysroot=$$SYSROOT
            QMAKE_LFLAGS   += --sysroot=$$SYSROOT
        } else {
            message("警告：Linux 宿主机 Android 构建，NDK sysroot 路径不存在：$$SYSROOT")
        }
    } else {
        message("警告：Linux 宿主机 Android 构建，未设置 ANDROID_NDK_ROOT 环境变量")
    }
}

#####################################################################################

# 在发布构建时禁用调试支持
DEFINES += QT_NO_DEBUG QML_DISABLE_PROFILER


#win32 {
    #QMAKE_CFLAGS += /utf-8
    #QMAKE_CXXFLAGS += /utf-8
#}


# Qt > 5 (Qt6)
greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat #statemachine
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG+=sdk_no_version_check

TRANSLATIONS += src/cn.ts \
    lib/qsci/qscintilla_cs.ts \
    lib/qsci/qscintilla_de.ts \
    lib/qsci/qscintilla_es.ts \
    lib/qsci/qscintilla_fr.ts \
    lib/qsci/qscintilla_pt_br.ts

ICON = res/icon.icns
RC_FILE += win.rc

# 设置QML模块导入路径
QML_IMPORT_PATH += $$PWD/src/qmlsrc

# 可选：如果需要在Qt Creator的设计器中也能识别模块，可添加此配置
QML_DESIGNER_IMPORT_PATH += $$PWD/src/qmlsrc

##################### 隔离第三方库的编译警告 ################################
# 1. 为第三方库创建单独的变量
#THIRD_PARTY_PATH = $$PWD/lib
#THIRD_PARTY_INCLUDE = $$THIRD_PARTY_PATH/cppjieba \
#                      $$THIRD_PARTY_PATH/qsci \
#                      $$THIRD_PARTY_PATH/qsci/Qsci \
#                      $$THIRD_PARTY_PATH/quazip \
#                      $$THIRD_PARTY_PATH/zlib \
#                      $$THIRD_PARTY_PATH/cmark-gfm/include \
#                      $$THIRD_PARTY_PATH/scintilla/include

# 2. 根据不同编译器设置隔离选项
#win32 {
    # MSVC - 使用外部包含指令
#    QMAKE_CXXFLAGS += -external:anglebrackets -external:W0
#    INCLUDEPATH += $$THIRD_PARTY_INCLUDE
#}
#clang|gcc {
    # GCC/Clang - 使用 -isystem
    # QMAKE_CXXFLAGS += -isystem $$THIRD_PARTY_INCLUDE
#}

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

unix:!macx:!android {
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
    lib/diff/diff_match_patch.cpp \
    lib/llama.cpp/common/unicode_common.cpp \
    lib/llama.cpp/ggml/src/ggml-cpu/ggml-cpu_2.cpp \
    lib/llama.cpp/ggml/src/ggml-cpu/quants_m.c \
    lib/llama.cpp/ggml/src/ggml-cpu/repack_m.cpp \
    lib/llama.cpp/src/models/llama_models.cpp \
    lib/llama.cpp/src/unicode.cpp \
    lib/llama.cpp/common/arg.cpp \
    lib/llama.cpp/common/chat-auto-parser-generator.cpp \
    lib/llama.cpp/common/chat-auto-parser-helpers.cpp \
    lib/llama.cpp/common/chat-diff-analyzer.cpp \
    lib/llama.cpp/common/chat-peg-parser.cpp \
    lib/llama.cpp/common/chat.cpp \
    lib/llama.cpp/common/common.cpp \
    lib/llama.cpp/common/console.cpp \
    lib/llama.cpp/common/debug.cpp \
    lib/llama.cpp/common/download.cpp \
    lib/llama.cpp/common/fit.cpp \
    lib/llama.cpp/common/hf-cache.cpp \
    lib/llama.cpp/common/imatrix-loader.cpp \
    lib/llama.cpp/common/jinja/caps.cpp \
    lib/llama.cpp/common/jinja/lexer.cpp \
    lib/llama.cpp/common/jinja/parser.cpp \
    lib/llama.cpp/common/jinja/runtime.cpp \
    lib/llama.cpp/common/jinja/string.cpp \
    lib/llama.cpp/common/jinja/value.cpp \
    lib/llama.cpp/common/json-schema-to-grammar.cpp \
    lib/llama.cpp/common/llguidance.cpp \
    lib/llama.cpp/common/log.cpp \
    lib/llama.cpp/common/build-info.cpp \
    lib/llama.cpp/common/ngram-cache.cpp \
    lib/llama.cpp/common/ngram-map.cpp \
    lib/llama.cpp/common/ngram-mod.cpp \
    lib/llama.cpp/common/peg-parser.cpp \
    lib/llama.cpp/common/preset.cpp \
    lib/llama.cpp/common/reasoning-budget.cpp \
    lib/llama.cpp/common/sampling.cpp \
    lib/llama.cpp/common/speculative.cpp \
    lib/llama.cpp/ggml/src/ggml-alloc.c \
    lib/llama.cpp/ggml/src/ggml-backend-dl.cpp \
    lib/llama.cpp/ggml/src/ggml-backend-meta.cpp \
    lib/llama.cpp/ggml/src/ggml-backend-reg.cpp \
    lib/llama.cpp/ggml/src/ggml-backend.cpp \
    lib/llama.cpp/ggml/src/ggml-cpu/binary-ops.cpp \
    lib/llama.cpp/ggml/src/ggml-cpu/ggml-cpu.c \
    lib/llama.cpp/ggml/src/ggml-cpu/hbm.cpp \
    lib/llama.cpp/ggml/src/ggml-cpu/ops.cpp \
    lib/llama.cpp/ggml/src/ggml-cpu/traits.cpp \
    lib/llama.cpp/ggml/src/ggml-cpu/unary-ops.cpp \
    lib/llama.cpp/ggml/src/ggml-cpu/vec.cpp \
    lib/llama.cpp/ggml/src/ggml-opt.cpp \
    lib/llama.cpp/ggml/src/ggml-quants.c \
    lib/llama.cpp/ggml/src/ggml-threading.cpp \
    lib/llama.cpp/ggml/src/ggml.c \
    lib/llama.cpp/ggml/src/ggml.cpp \
    lib/llama.cpp/ggml/src/gguf.cpp \
    lib/llama.cpp/src/llama-adapter.cpp \
    lib/llama.cpp/src/llama-arch.cpp \
    lib/llama.cpp/src/llama-batch.cpp \
    lib/llama.cpp/src/llama-chat.cpp \
    lib/llama.cpp/src/llama-context.cpp \
    lib/llama.cpp/src/llama-cparams.cpp \
    lib/llama.cpp/src/llama-grammar.cpp \
    lib/llama.cpp/src/llama-graph.cpp \
    lib/llama.cpp/src/llama-hparams.cpp \
    lib/llama.cpp/src/llama-impl.cpp \
    lib/llama.cpp/src/llama-io.cpp \
    lib/llama.cpp/src/llama-kv-cache-dsa.cpp \
    lib/llama.cpp/src/llama-kv-cache-dsv4.cpp \
    lib/llama.cpp/src/llama-kv-cache-iswa.cpp \
    lib/llama.cpp/src/llama-kv-cache.cpp \
    lib/llama.cpp/src/llama-memory-hybrid-iswa.cpp \
    lib/llama.cpp/src/llama-memory-hybrid.cpp \
    lib/llama.cpp/src/llama-memory-recurrent.cpp \
    lib/llama.cpp/src/llama-memory.cpp \
    lib/llama.cpp/src/llama-mmap.cpp \
    lib/llama.cpp/src/llama-model-loader.cpp \
    lib/llama.cpp/src/llama-model-saver.cpp \
    lib/llama.cpp/src/llama-model.cpp \
    lib/llama.cpp/src/llama-quant.cpp \
    lib/llama.cpp/src/llama-sampler.cpp \
    lib/llama.cpp/src/llama-vocab.cpp \
    lib/llama.cpp/src/llama.cpp \
    lib/llama.cpp/src/models/afmoe.cpp \
    lib/llama.cpp/src/models/apertus.cpp \
    lib/llama.cpp/src/models/arcee.cpp \
    lib/llama.cpp/src/models/arctic.cpp \
    lib/llama.cpp/src/models/arwkv7.cpp \
    lib/llama.cpp/src/models/baichuan.cpp \
    lib/llama.cpp/src/models/bailingmoe.cpp \
    lib/llama.cpp/src/models/bailingmoe2.cpp \
    lib/llama.cpp/src/models/bert.cpp \
    lib/llama.cpp/src/models/bitnet.cpp \
    lib/llama.cpp/src/models/bloom.cpp \
    lib/llama.cpp/src/models/chameleon.cpp \
    lib/llama.cpp/src/models/chatglm.cpp \
    lib/llama.cpp/src/models/codeshell.cpp \
    lib/llama.cpp/src/models/cogvlm.cpp \
    lib/llama.cpp/src/models/cohere2.cpp \
    lib/llama.cpp/src/models/cohere2moe.cpp \
    lib/llama.cpp/src/models/command-r.cpp \
    lib/llama.cpp/src/models/dbrx.cpp \
    lib/llama.cpp/src/models/deci.cpp \
    lib/llama.cpp/src/models/deepseek.cpp \
    lib/llama.cpp/src/models/deepseek2.cpp \
    lib/llama.cpp/src/models/deepseek2ocr.cpp \
    lib/llama.cpp/src/models/deepseek32.cpp \
    lib/llama.cpp/src/models/deepseek4.cpp \
    lib/llama.cpp/src/models/delta-net-base.cpp \
    lib/llama.cpp/src/models/dflash.cpp \
    lib/llama.cpp/src/models/dots1.cpp \
    lib/llama.cpp/src/models/dream.cpp \
    lib/llama.cpp/src/models/eagle3.cpp \
    lib/llama.cpp/src/models/ernie4-5-moe.cpp \
    lib/llama.cpp/src/models/ernie4-5.cpp \
    lib/llama.cpp/src/models/eurobert.cpp \
    lib/llama.cpp/src/models/exaone-moe.cpp \
    lib/llama.cpp/src/models/exaone.cpp \
    lib/llama.cpp/src/models/exaone4.cpp \
    lib/llama.cpp/src/models/falcon-h1.cpp \
    lib/llama.cpp/src/models/falcon.cpp \
    lib/llama.cpp/src/models/gemma-embedding.cpp \
    lib/llama.cpp/src/models/gemma.cpp \
    lib/llama.cpp/src/models/gemma2.cpp \
    lib/llama.cpp/src/models/gemma3.cpp \
    lib/llama.cpp/src/models/gemma3n.cpp \
    lib/llama.cpp/src/models/gemma4-assistant.cpp \
    lib/llama.cpp/src/models/gemma4.cpp \
    lib/llama.cpp/src/models/glm-dsa.cpp \
    lib/llama.cpp/src/models/glm4-moe.cpp \
    lib/llama.cpp/src/models/glm4.cpp \
    lib/llama.cpp/src/models/gpt2.cpp \
    lib/llama.cpp/src/models/gptneox.cpp \
    lib/llama.cpp/src/models/granite-hybrid.cpp \
    lib/llama.cpp/src/models/granite-moe.cpp \
    lib/llama.cpp/src/models/granite.cpp \
    lib/llama.cpp/src/models/grok.cpp \
    lib/llama.cpp/src/models/grovemoe.cpp \
    lib/llama.cpp/src/models/hunyuan-dense.cpp \
    lib/llama.cpp/src/models/hunyuan-moe.cpp \
    lib/llama.cpp/src/models/hunyuan-vl.cpp \
    lib/llama.cpp/src/models/hy-v3.cpp \
    lib/llama.cpp/src/models/internlm2.cpp \
    lib/llama.cpp/src/models/jais.cpp \
    lib/llama.cpp/src/models/jais2.cpp \
    lib/llama.cpp/src/models/jamba.cpp \
    lib/llama.cpp/src/models/jina-bert-v2.cpp \
    lib/llama.cpp/src/models/jina-bert-v3.cpp \
    lib/llama.cpp/src/models/kimi-linear.cpp \
    lib/llama.cpp/src/models/lfm2.cpp \
    lib/llama.cpp/src/models/lfm2moe.cpp \
    lib/llama.cpp/src/models/llada-moe.cpp \
    lib/llama.cpp/src/models/llada.cpp \
    lib/llama.cpp/src/models/llama-embed.cpp \
    lib/llama.cpp/src/models/llama4.cpp \
    lib/llama.cpp/src/models/maincoder.cpp \
    lib/llama.cpp/src/models/mamba-base.cpp \
    lib/llama.cpp/src/models/mamba.cpp \
    lib/llama.cpp/src/models/mamba2.cpp \
    lib/llama.cpp/src/models/mellum.cpp \
    lib/llama.cpp/src/models/mimo2.cpp \
    lib/llama.cpp/src/models/minicpm.cpp \
    lib/llama.cpp/src/models/minicpm3.cpp \
    lib/llama.cpp/src/models/minimax-m2.cpp \
    lib/llama.cpp/src/models/mistral3.cpp \
    lib/llama.cpp/src/models/mistral4.cpp \
    lib/llama.cpp/src/models/modern-bert.cpp \
    lib/llama.cpp/src/models/mpt.cpp \
    lib/llama.cpp/src/models/nemotron-h-moe.cpp \
    lib/llama.cpp/src/models/nemotron-h.cpp \
    lib/llama.cpp/src/models/nemotron.cpp \
    lib/llama.cpp/src/models/neo-bert.cpp \
    lib/llama.cpp/src/models/nomic-bert-moe.cpp \
    lib/llama.cpp/src/models/nomic-bert.cpp \
    lib/llama.cpp/src/models/olmo.cpp \
    lib/llama.cpp/src/models/olmo2.cpp \
    lib/llama.cpp/src/models/olmoe.cpp \
    lib/llama.cpp/src/models/openai-moe.cpp \
    lib/llama.cpp/src/models/openelm.cpp \
    lib/llama.cpp/src/models/orion.cpp \
    lib/llama.cpp/src/models/paddleocr.cpp \
    lib/llama.cpp/src/models/pangu-embed.cpp \
    lib/llama.cpp/src/models/phi2.cpp \
    lib/llama.cpp/src/models/phi3.cpp \
    lib/llama.cpp/src/models/phimoe.cpp \
    lib/llama.cpp/src/models/plamo.cpp \
    lib/llama.cpp/src/models/plamo2.cpp \
    lib/llama.cpp/src/models/plamo3.cpp \
    lib/llama.cpp/src/models/plm.cpp \
    lib/llama.cpp/src/models/qwen.cpp \
    lib/llama.cpp/src/models/qwen2.cpp \
    lib/llama.cpp/src/models/qwen2moe.cpp \
    lib/llama.cpp/src/models/qwen2vl.cpp \
    lib/llama.cpp/src/models/qwen3.cpp \
    lib/llama.cpp/src/models/qwen35.cpp \
    lib/llama.cpp/src/models/qwen35moe.cpp \
    lib/llama.cpp/src/models/qwen3moe.cpp \
    lib/llama.cpp/src/models/qwen3next.cpp \
    lib/llama.cpp/src/models/qwen3vl.cpp \
    lib/llama.cpp/src/models/qwen3vlmoe.cpp \
    lib/llama.cpp/src/models/refact.cpp \
    lib/llama.cpp/src/models/rnd1.cpp \
    lib/llama.cpp/src/models/rwkv6-base.cpp \
    lib/llama.cpp/src/models/rwkv6.cpp \
    lib/llama.cpp/src/models/rwkv6qwen2.cpp \
    lib/llama.cpp/src/models/rwkv7-base.cpp \
    lib/llama.cpp/src/models/rwkv7.cpp \
    lib/llama.cpp/src/models/seed-oss.cpp \
    lib/llama.cpp/src/models/smallthinker.cpp \
    lib/llama.cpp/src/models/smollm3.cpp \
    lib/llama.cpp/src/models/stablelm.cpp \
    lib/llama.cpp/src/models/starcoder.cpp \
    lib/llama.cpp/src/models/starcoder2.cpp \
    lib/llama.cpp/src/models/step35.cpp \
    lib/llama.cpp/src/models/t5.cpp \
    lib/llama.cpp/src/models/t5encoder.cpp \
    lib/llama.cpp/src/models/talkie.cpp \
    lib/llama.cpp/src/models/wavtokenizer-dec.cpp \
    lib/llama.cpp/src/models/xverse.cpp \
    lib/llama.cpp/src/unicode-data.cpp \
    lib/llama.cpp/vendor/cpp-httplib/httplib.cpp \
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
    src/AI/AiModelDeployer.cpp \
    src/AI/EmbeddingEngine.cpp \
    src/AI/GlobalAI.cpp \
    src/AI/VectorDb.cpp \
    src/AboutThis.cpp \
    src/AutoUpdate.cpp \
    src/CategoryList.cpp \
    src/CloudBackup.cpp \
    src/Comm/DatePicker.cpp \
    src/Comm/GeoAddressResolver.cpp \
    src/Comm/Method.cpp \
    src/Comm/ReceiveShare.cpp \
    src/Comm/ShowMessage.cpp \
    src/Comm/TextEditToolbar.cpp \
    src/Comm/Time24Picker.cpp \
    src/Comm/WheelWidget.cpp \
    src/Comm/enhancedcolorpicker.cpp \
    src/Comm/loglogger.cpp \
    src/Comm/qaesencryption.cpp \
    src/DataManager.cpp \
    src/DateSelector.cpp \
    src/EditRecord.cpp \
    src/Exercise/CompassWidget.cpp \
    src/Exercise/CustomChartView.cpp \
    src/Exercise/DrawSportsFreq.cpp \
    src/Exercise/ShowSportChart.cpp \
    src/Exercise/StepHillChart.cpp \
    src/Exercise/WeatherFetcher.cpp \
    src/JavaToQtBridge.cpp \
    src/LoadPic.cpp \
    src/MainHelper.cpp \
    src/MainWindow.cpp \
    src/MainWindow_AI.cpp \
    src/MainWindow_Btn.cpp \
    src/MainWindow_Init.cpp \
    src/MainWindow_Menu.cpp \
    src/MyThread.cpp \
    src/Notes/ColorDialog.cpp \
    src/Notes/MarkdownToHtml.cpp \
    src/Notes/MoveTo.cpp \
    src/Notes/NoteDiffManager.cpp \
    src/Notes/NoteListModel.cpp \
    src/Notes/Notes.cpp \
    src/Notes/NotesList.cpp \
    src/Notes/NotesList_Diff.cpp \
    src/Notes/NotesList_Event.cpp \
    src/Notes/NotesList_Graph.cpp \
    src/Notes/NotesList_Menu.cpp \
    src/Notes/NotesList_Search.cpp \
    src/Notes/NotesList_Utils.cpp \
    src/Notes/NotesList_WebDAV.cpp \
    src/Notes/Notes_Android.cpp \
    src/Notes/Notes_Diff.cpp \
    src/Notes/Notes_Editor.cpp \
    src/Notes/Notes_FileIO.cpp \
    src/Notes/Notes_Image.cpp \
    src/Notes/Notes_IndexManager.cpp \
    src/Notes/Notes_LocalAI.cpp \
    src/Notes/Notes_Sync.cpp \
    src/Notes/Notes_UI.cpp \
    src/Notes/Notes_Utils.cpp \
    src/Notes/PrintPDF.cpp \
    src/Notes/database_manager.cpp \
    src/Notes/note_graph.cpp \
    src/Notes/note_index_manager.cpp \
    src/Notes/qtreewidgetproxymodel.cpp \
    src/Notes/search_model.cpp \
    src/Notes/titlegenerator.cpp \
    src/Preferences.cpp \
    src/Reader/DocumentHandler.cpp \
    src/Reader/Reader.cpp \
    src/Reader/ReaderSet.cpp \
    src/Reader/Reader_Bookmarks.cpp \
    src/Reader/Reader_Cata.cpp \
    src/Reader/Reader_Notes.cpp \
    src/Reader/Reader_PDF.cpp \
    src/Reader/Reader_QML.cpp \
    src/Reader/Reader_TTS.cpp \
    src/Reader/epubreader.cpp \
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
    src/native_msg_host.cpp

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
    lib/diff/diff_match_patch.h \
    lib/llama.cpp/common/arg.h \
    lib/llama.cpp/common/base64.hpp \
    lib/llama.cpp/common/build-info.h \
    lib/llama.cpp/common/chat-auto-parser-helpers.h \
    lib/llama.cpp/common/chat-auto-parser.h \
    lib/llama.cpp/common/chat-peg-parser.h \
    lib/llama.cpp/common/chat.h \
    lib/llama.cpp/common/common.h \
    lib/llama.cpp/common/console.h \
    lib/llama.cpp/common/debug.h \
    lib/llama.cpp/common/download.h \
    lib/llama.cpp/common/fit.h \
    lib/llama.cpp/common/hf-cache.h \
    lib/llama.cpp/common/http.h \
    lib/llama.cpp/common/imatrix-loader.h \
    lib/llama.cpp/common/jinja/caps.h \
    lib/llama.cpp/common/jinja/lexer.h \
    lib/llama.cpp/common/jinja/parser.h \
    lib/llama.cpp/common/jinja/runtime.h \
    lib/llama.cpp/common/jinja/string.h \
    lib/llama.cpp/common/jinja/utils.h \
    lib/llama.cpp/common/jinja/value.h \
    lib/llama.cpp/common/json-schema-to-grammar.h \
    lib/llama.cpp/common/log.h \
    lib/llama.cpp/common/ngram-cache.h \
    lib/llama.cpp/common/ngram-map.h \
    lib/llama.cpp/common/ngram-mod.h \
    lib/llama.cpp/common/peg-parser.h \
    lib/llama.cpp/common/preset.h \
    lib/llama.cpp/common/reasoning-budget.h \
    lib/llama.cpp/common/sampling.h \
    lib/llama.cpp/common/speculative.h \
    lib/llama.cpp/common/unicode.h \
    lib/llama.cpp/ggml/include/ggml-alloc.h \
    lib/llama.cpp/ggml/include/ggml-backend.h \
    lib/llama.cpp/ggml/include/ggml-blas.h \
    lib/llama.cpp/ggml/include/ggml-cann.h \
    lib/llama.cpp/ggml/include/ggml-cpp.h \
    lib/llama.cpp/ggml/include/ggml-cpu.h \
    lib/llama.cpp/ggml/include/ggml-cuda.h \
    lib/llama.cpp/ggml/include/ggml-et.h \
    lib/llama.cpp/ggml/include/ggml-hexagon.h \
    lib/llama.cpp/ggml/include/ggml-metal.h \
    lib/llama.cpp/ggml/include/ggml-opencl.h \
    lib/llama.cpp/ggml/include/ggml-openvino.h \
    lib/llama.cpp/ggml/include/ggml-opt.h \
    lib/llama.cpp/ggml/include/ggml-rpc.h \
    lib/llama.cpp/ggml/include/ggml-sycl.h \
    lib/llama.cpp/ggml/include/ggml-virtgpu.h \
    lib/llama.cpp/ggml/include/ggml-vulkan.h \
    lib/llama.cpp/ggml/include/ggml-webgpu.h \
    lib/llama.cpp/ggml/include/ggml-zdnn.h \
    lib/llama.cpp/ggml/include/ggml-zendnn.h \
    lib/llama.cpp/ggml/include/ggml.h \
    lib/llama.cpp/ggml/include/gguf.h \
    lib/llama.cpp/ggml/src/ggml-backend-dl.h \
    lib/llama.cpp/ggml/src/ggml-backend-impl.h \
    lib/llama.cpp/ggml/src/ggml-common.h \
    lib/llama.cpp/ggml/src/ggml-cpu/arch-fallback.h \
    lib/llama.cpp/ggml/src/ggml-cpu/binary-ops.h \
    lib/llama.cpp/ggml/src/ggml-cpu/common.h \
    lib/llama.cpp/ggml/src/ggml-cpu/ggml-cpu-impl.h \
    lib/llama.cpp/ggml/src/ggml-cpu/hbm.h \
    lib/llama.cpp/ggml/src/ggml-cpu/ops.h \
    lib/llama.cpp/ggml/src/ggml-cpu/quants.h \
    lib/llama.cpp/ggml/src/ggml-cpu/repack.h \
    lib/llama.cpp/ggml/src/ggml-cpu/simd-gemm.h \
    lib/llama.cpp/ggml/src/ggml-cpu/simd-mappings.h \
    lib/llama.cpp/ggml/src/ggml-cpu/traits.h \
    lib/llama.cpp/ggml/src/ggml-cpu/unary-ops.h \
    lib/llama.cpp/ggml/src/ggml-cpu/vec.h \
    lib/llama.cpp/ggml/src/ggml-impl.h \
    lib/llama.cpp/ggml/src/ggml-quants.h \
    lib/llama.cpp/ggml/src/ggml-threading.h \
    lib/llama.cpp/include/llama-cpp.h \
    lib/llama.cpp/include/llama.h \
    lib/llama.cpp/src/llama-adapter.h \
    lib/llama.cpp/src/llama-arch.h \
    lib/llama.cpp/src/llama-batch.h \
    lib/llama.cpp/src/llama-chat.h \
    lib/llama.cpp/src/llama-context.h \
    lib/llama.cpp/src/llama-cparams.h \
    lib/llama.cpp/src/llama-ext.h \
    lib/llama.cpp/src/llama-grammar.h \
    lib/llama.cpp/src/llama-graph.h \
    lib/llama.cpp/src/llama-hparams.h \
    lib/llama.cpp/src/llama-impl.h \
    lib/llama.cpp/src/llama-io.h \
    lib/llama.cpp/src/llama-kv-cache-dsa.h \
    lib/llama.cpp/src/llama-kv-cache-dsv4.h \
    lib/llama.cpp/src/llama-kv-cache-iswa.h \
    lib/llama.cpp/src/llama-kv-cache.h \
    lib/llama.cpp/src/llama-kv-cells.h \
    lib/llama.cpp/src/llama-memory-hybrid-iswa.h \
    lib/llama.cpp/src/llama-memory-hybrid.h \
    lib/llama.cpp/src/llama-memory-recurrent.h \
    lib/llama.cpp/src/llama-memory.h \
    lib/llama.cpp/src/llama-mmap.h \
    lib/llama.cpp/src/llama-model-loader.h \
    lib/llama.cpp/src/llama-model-saver.h \
    lib/llama.cpp/src/llama-model.h \
    lib/llama.cpp/src/llama-quant.h \
    lib/llama.cpp/src/llama-sampler.h \
    lib/llama.cpp/src/llama-vocab.h \
    lib/llama.cpp/src/models/models.h \
    lib/llama.cpp/src/unicode-data.h \
    lib/llama.cpp/src/unicode.h \
    lib/llama.cpp/vendor/cpp-httplib/httplib.h \
    lib/llama.cpp/vendor/miniaudio/miniaudio.h \
    lib/llama.cpp/vendor/nlohmann/json.hpp \
    lib/llama.cpp/vendor/nlohmann/json_fwd.hpp \
    lib/llama.cpp/vendor/sheredom/subprocess.h \
    lib/llama.cpp/vendor/stb/stb_image.h \
    lib/sqlite/sqlite3.h \
    lib/sqlite/sqlite3ext.h \
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
    src/AI/AiModelDeployer.h \
    src/AI/BaseEmbeddingEngine.h \
    src/AI/EmbeddingEngine.h \
    src/AI/GlobalAI.h \
    src/AI/VectorDb.h \
    src/AboutThis.h \
    src/AutoUpdate.h \
    src/CategoryList.h \
    src/CloudBackup.h \
    src/Comm/DatePicker.h \
    src/Comm/GeoAddressResolver.h \
    src/Comm/Method.h \
    src/Comm/ReceiveShare.h \
    src/Comm/ShowMessage.h \
    src/Comm/TextEditToolbar.h \
    src/Comm/Time24Picker.h \
    src/Comm/WheelWidget.h \
    src/Comm/enhancedcolorpicker.h \
    src/Comm/loglogger.h \
    src/Comm/qaesencryption.h \
    src/DataManager.h \
    src/DateSelector.h \
    src/DelWebDAVFiles.h \
    src/EditRecord.h \
    src/Exercise/CompassWidget.h \
    src/Exercise/CustomChartView.h \
    src/Exercise/DrawSportsFreq.h \
    src/Exercise/StepHillChart.h \
    src/Exercise/WeatherFetcher.h \
    src/LoadPic.h \
    src/MainHelper.h \
    src/MainWindow.h \
    src/MyThread.h \
    src/Notes/ColorDialog.h \
    src/Notes/MoveTo.h \
    src/Notes/NoteDiffManager.h \
    src/Notes/NoteListModel.h \
    src/Notes/Notes.h \
    src/Notes/NotesList.h \
    src/Notes/PrintPDF.h \
    src/Notes/database_manager.h \
    src/Notes/note_graph.h \
    src/Notes/note_index_manager.h \
    src/Notes/qtreewidgetproxymodel.h \
    src/Notes/search_model.h \
    src/Notes/titlegenerator.h \
    src/Preferences.h \
    src/Reader/DocumentHandler.h \
    src/Reader/Reader.h \
    src/Reader/ReaderSet.h \
    src/Reader/epubreader.h \
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
    src/defines.h \
    src/native_msg_host.h \
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
    src/Notes/Notes.ui \
    src/Notes/NotesList.ui \
    src/Notes/PrintPDF.ui \
    src/Preferences.ui \
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
    lib/llama.cpp/vendor/cpp-httplib/CMakeLists.txt \
    lib/llama.cpp/vendor/cpp-httplib/LICENSE \
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


############################## sqlite3 ####################################
# 向量检索总开关，true启用sqlite3+sqlite-vec向量库，false完全跳过
VECTOR_SEARCH = true

equals(VECTOR_SEARCH, true) {
    # 统一头文件路径，全局可识别
    INCLUDEPATH += $$PWD/lib/sqlite $$PWD/lib/sqlite_vec

    SOURCES += $$PWD/lib/sqlite/sqlite3.c $$PWD/lib/sqlite_vec/sqlite-vec.c

    # 静态编译vec扩展宏
    DEFINES += SQLITE_VEC_STATIC
    DEFINES += VECTOR_SEARCH

    win32 {
        LIBS += -lshell32
        #DEFINES += SQLITE_API=__declspec(dllexport)

    }
}

########################## OpenSSL ########################################

contains(ANDROID_TARGET_ARCH,arm64-v8a) {
    ANDROID_EXTRA_LIBS = \
        $$PWD/android-openssl/ssl_3/v8a/libcrypto_3.so \
        $$PWD/android-openssl/ssl_3/v8a/libssl_3.so
}

############################### llama.cpp ############################################

#===================== 重名文件改名提示 =================================================
# ggml/src/ggml-cpu/repack.cpp
# ggml/src/ggml-cpu/quants.c
# src/models/llama.cpp
# common/unicode.cpp
# ggml/src/ggml-cpu/ggml-cpp.cpp(同目录存在ggml-cpp.c文件会生成同一个obj导致链接混乱）

# ===================== ggml quants.c 关键适配提醒 =====================================
# ggml/src/ggml-cpu/quants.c 文件末尾必须保留以下两段条件包含代码，不要被上游源码升级覆盖：

# #ifdef GGML_USE_AVX2
# #include "arch/x86/quants.c"
# #endif
# #ifdef GGML_USE_NEON
# #include "arch/arm/quants.c"
# #endif

# 原理：
# 1.官方CMake会自动追加上述代码；qmake不会自动注入
# 2.arch内部函数为static，只能通过quants.c内嵌#include编译进同一obj，不能把arch下c文件单独加入SOURCES编译

# ====================ggml/src/ggml-cpu/repack.cpp 同quants.c，升级源码后必须检查末尾保留===========

#ifdef GGML_USE_AVX2
#define NEAREST_INT
#include "arch/x86/repack.cpp"
#undef NEAREST_INT
#endif

# #ifdef GGML_USE_NEON
# #include "arch/arm/repack.cpp"
# #endif

# 缺失会导致矩阵乘、量化矩阵相关大量链接缺失

# repack.cpp 内嵌arch说明：
# 1. arch/x86/repack.cpp 存在nearest_int，引入前必须#define NEAREST_INT 避免和上层重定义
# 2. arch/arm/repack.cpp 无nearest_int，无需宏隔离，直接#include即可
# ===========================================================================================

LLAMA_ROOT = $$PWD/lib/llama.cpp

# 头文件路径全局统一覆盖
INCLUDEPATH += \
    $$LLAMA_ROOT/common \
    $$LLAMA_ROOT/include \
    $$LLAMA_ROOT/src \
    $$LLAMA_ROOT/ggml/include \
    $$LLAMA_ROOT/ggml/src \
    $$LLAMA_ROOT/ggml/src/ggml-cpu \
    $$LLAMA_ROOT/vendor \
    $$LLAMA_ROOT/vendor/nlohmann \
    $$LLAMA_ROOT/vendor/cpp-httplib

# 全局宏
DEFINES += \
    GGML_VERSION=\\\"0\\\" \
    GGML_COMMIT=\\\"b10041\\\" \
    GGML_NO_CUDA GGML_NO_METAL GGML_NO_OPENCL GGML_NO_VULKAN \
    GGML_NO_HEXAGON GGML_NO_ET GGML_NO_RPC GGML_NO_SYCL GGML_STATIC GGML_NO_KLEIDIAI \
    LLAMA_BUILD_INFO LLAMA_NO_SERVER LLAMA_NO_CLI LLAMA_ARCH \
    LLAMA_GRAPH LLAMA_MODEL GGML_ALLOCATOR LLAMA_IMPL LLAMA_EMBED

# 开启CPU额外buffer类型，才能生成 ggml_backend_cpu_get_extra_buffer_types
DEFINES += GGML_ALLOW_EXTRA_BUFFERS

# 全平台统一开启GGML_CPU（核心，所有CPU后端依赖）
DEFINES += GGML_CPU

# Windows x86
win32 {
    DEFINES += GGML_WIN32 GGML_USE_AVX2 GGML_F16C __AVX2__
}

# Linux x86 / Mac Intel（x86_64）
unix:!android:!macx|macx:!arm64 {
    DEFINES += GGML_USE_AVX2 GGML_F16C
}

# Apple Silicon / Android ARM 完全不加AVX宏
macx:arm64|android {
    DEFINES += GGML_USE_NEON GGML_NO_AVX GGML_USE_DOTPROD GGML_USE_FP16_VECTOR_ARITHMETIC
}

# 分平台追加对应arch架构完整算子（x86/arm递归全部量化文件）
win32-msvc {
    QMAKE_CXXFLAGS += /arch:AVX2 /utf-8 /std:c++17
    QMAKE_CFLAGS += /arch:AVX2 /utf-8
    # 保留函数不优化删除
    QMAKE_LFLAGS += /OPT:NOREF /OPT:NOICF
    # 强制要求链接器引入ggml_backend_cpu_reg符号，自动拉入ggml-cpu_2.obj
    QMAKE_LFLAGS += /INCLUDE:ggml_backend_cpu_reg

    # Windows 注册表检测逻辑，检查cpu类型
    LIBS += -ladvapi32
}

# Linux平台
unix:!android:!macx {
    LIBS += -pthread
    QMAKE_CXXFLAGS += -mavx2 -mf16c -std=c++17 -DGGML_CPU -DGGML_USE_AVX2
    QMAKE_CFLAGS += -mavx2 -mf16c -DGGML_CPU -DGGML_USE_AVX2 -mfma
}

# Mac Intel平台
macx:!arm64 {
    LIBS += -pthread
    QMAKE_CXXFLAGS += -mavx2 -mf16c -std=c++17 -DGGML_CPU -DGGML_USE_AVX2
    QMAKE_CFLAGS += -mavx2 -mf16c -DGGML_CPU -DGGML_USE_AVX2 -mfma
}

macx:arm64 {
    LIBS += -pthread
    # 无需AVX编译参数，仅靠全局DEFINES GGML_CPU/GGML_USE_NEON生效
}

android {
    LIBS += -pthread

}


