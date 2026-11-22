# ============================================================
# lib/sci : Scintilla(Qt 前端 ScintillaEdit) + Lexilla 纯源码静态集成
# ============================================================

set(SCI_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/scintilla)
set(LEX_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/lexilla)

if(NOT EXISTS ${SCI_ROOT}/qt/ScintillaEdit/ScintillaEdit.cpp)
    message(FATAL_ERROR "[sci] scintilla qt frontend not found: ${SCI_ROOT}/qt/ScintillaEdit")
endif()
if(NOT EXISTS ${LEX_ROOT}/src/Lexilla.cxx)
    message(FATAL_ERROR "[sci] lexilla not found: ${LEX_ROOT}/src/Lexilla.cxx")
endif()

# ------------------------------------------------------------
# 1) lexilla：词法器库
# ------------------------------------------------------------
file(GLOB LEXILLA_SRC CONFIGURE_DEPENDS
    ${LEX_ROOT}/src/*.cxx
    ${LEX_ROOT}/lexers/*.cxx
    ${LEX_ROOT}/lexlib/*.cxx
)

add_library(lexilla STATIC ${LEXILLA_SRC})
target_include_directories(lexilla PUBLIC
    ${LEX_ROOT}/include
    ${SCI_ROOT}/include       # Lexilla 需要 Scintilla.h
)
target_include_directories(lexilla PRIVATE
    ${LEX_ROOT}/lexlib
    ${LEX_ROOT}/src
)
target_compile_features(lexilla PUBLIC cxx_std_17)

# ------------------------------------------------------------
# 2) scintilla_qt：核心 + Qt 平台层 + ScintillaEdit 控件
# ------------------------------------------------------------
file(GLOB SCINTILLA_CORE_SRC CONFIGURE_DEPENDS
    ${SCI_ROOT}/src/*.cxx
    ${SCI_ROOT}/libsrc/*.cxx
)
file(GLOB SCINTILLA_QT_SRC CONFIGURE_DEPENDS
    ${SCI_ROOT}/qt/ScintillaEdit/*.cpp
    ${SCI_ROOT}/qt/ScintillaEditBase/*.cpp
)

add_library(scintilla_qt STATIC ${SCINTILLA_CORE_SRC} ${SCINTILLA_QT_SRC})

target_include_directories(scintilla_qt PUBLIC
    ${SCI_ROOT}/include
    ${SCI_ROOT}/src
    ${SCI_ROOT}/qt/ScintillaEdit
    ${SCI_ROOT}/qt/ScintillaEditBase
)
target_include_directories(scintilla_qt PRIVATE
    ${SCI_ROOT}/src
    ${SCI_ROOT}/libsrc
)

# PUBLIC：影响 app 侧包含的 Scintilla.h / ScintillaEdit.h 导出与接口形态
target_compile_definitions(scintilla_qt PUBLIC SCINTILLA_QT SCI_LEXER SCI_QT6)

# ⚠️ AUTOMOC 必须在 target_link_libraries 之前或之后均可，
#    但必须在 add_library 之后
set_target_properties(scintilla_qt PROPERTIES AUTOMOC ON)
target_compile_features(scintilla_qt PUBLIC cxx_std_17)

# ✅ Qt 依赖（Core5Compat 放在 add_library 之后）
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets PrintSupport Core5Compat)
target_link_libraries(scintilla_qt PUBLIC
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::PrintSupport
)
# ✅ Core5Compat 用 PRIVATE：仅 scintilla_qt 内部 .cpp 使用 QTextCodec，
#    你的业务代码不 include <QTextCodec>，无需传播
target_link_libraries(scintilla_qt PRIVATE Qt6::Core5Compat)

# scintilla_qt 链接 lexilla（Lexilla 注册机制需要）
target_link_libraries(scintilla_qt PUBLIC lexilla)

# ------------------------------------------------------------
# 3) 平台微调
# ------------------------------------------------------------
if(MSVC)
    target_compile_definitions(lexilla      PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_definitions(scintilla_qt PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# ------------------------------------------------------------
# 4) 诊断输出
# ------------------------------------------------------------
get_target_property(_sci_srcs scintilla_qt SOURCES)
list(LENGTH _sci_srcs _sci_n)
message(STATUS "[sci] scintilla_qt sources: ${_sci_n}")
list(LENGTH LEXILLA_SRC _lex_n)
message(STATUS "[sci] lexilla sources: ${_lex_n}")