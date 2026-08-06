# lib/qsci/CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(qscintilla LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets PrintSupport)

# AUTOMOC 仅对本目标生效，干净隔离
set(CMAKE_AUTOMOC ON)

# ============================================================
# 1. Scintilla 核心引擎
# ============================================================
file(GLOB SCINTILLA_SRC    CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../scintilla/src/*.cpp)
file(GLOB SCINTILLA_LEXLIB CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../scintilla/lexlib/*.cpp)
file(GLOB SCINTILLA_LEXERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../scintilla/lexers/Lex*.cpp)

list(FILTER SCINTILLA_SRC EXCLUDE REGEX ".*/ExternalLexer\\.cpp$")

# ============================================================
# 2. QScintilla Qt 封装层
# ============================================================
file(GLOB QSCI_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
# ✅ 同时扫描根目录和 Qsci 子目录的头文件
file(GLOB QSCI_HEADERS
    CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Qsci/*.h     # ← 关键！
)

# ============================================================
# 3. 构建静态库
# ============================================================
add_library(qscintilla STATIC
    ${SCINTILLA_SRC}
    ${SCINTILLA_LEXLIB}
    ${SCINTILLA_LEXERS}
    ${QSCI_SOURCES}
    ${QSCI_HEADERS}
)

# ============================================================
# 4. 头文件路径 (PUBLIC → 自动传递给链接者)
# ============================================================
target_include_directories(qscintilla PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Qsci
    ${CMAKE_CURRENT_SOURCE_DIR}/../scintilla/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../scintilla/lexlib
    ${CMAKE_CURRENT_SOURCE_DIR}/../scintilla/src
)

# ============================================================
# 5. 编译定义
# ============================================================
target_compile_definitions(qscintilla PRIVATE
    SCI_LEXER
    LEXER_MARKDOWN_INCLUDED
    QSCINTILLA_HAVE_MARKDOWNLEXER
)

# ============================================================
# 6. MSVC: UTF-8 + win32_undef_macros.h 强制注入
# ============================================================
if(MSVC)
    target_compile_options(qscintilla PRIVATE /utf-8)
    # ✅ QSci 自身编译也需要此文件来避免 INDIC_* 宏与 enum 冲突
    target_compile_options(qscintilla PRIVATE
        /FI${CMAKE_CURRENT_SOURCE_DIR}/win32_undef_macros.h
    )
endif()

# ============================================================
# 7. 链接 Qt
# ============================================================
target_link_libraries(qscintilla PUBLIC
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::PrintSupport
)
