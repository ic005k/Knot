cmake_minimum_required(VERSION 3.16)
project(Knot VERSION 26.08.30 LANGUAGES C CXX)

# ============================================================
# ✅ 全局 MSVC 编译选项（必须在 add_library / add_executable 之前）
# ============================================================
if(MSVC)
    add_compile_definitions(
        NOMINMAX
        WIN32_LEAN_AND_MEAN
    )
    add_compile_options(/utf-8)
endif()

# ============================================================
# 全局标准与 Qt 自动化
# ============================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# ✅ 告诉 AUTOUIC 去 src/ 目录下搜索 .ui 文件
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/src)

# ============================================================
# 1. Qt 模块查找
# ============================================================
find_package(Qt6 REQUIRED COMPONENTS
    Core Gui Widgets Network PrintSupport Charts Sensors Sql
    Xml Svg Concurrent LinguistTools Core5Compat Positioning
)

# ✅ 设置排除虚拟键盘插件
if(ANDROID)
    set(QT_ANDROID_EXCLUDE_PLUGINS
        platforminputcontexts/qtvirtualkeyboardplugin
        position/qtposition_nmea              # 可选：如果也不用 NMEA 串口定位
    )
endif()

# ============================================================
# 2. 第三方库: llama.cpp (通过 add_subdirectory 构建)
# ============================================================
set(LLAMA_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/lib/llama.cpp)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(LLAMA_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(LLAMA_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(LLAMA_BUILD_SERVER OFF CACHE BOOL "" FORCE)
set(LLAMA_BUILD_CLI OFF CACHE BOOL "" FORCE)
set(GGML_CUDA OFF CACHE BOOL "" FORCE)
set(GGML_METAL OFF CACHE BOOL "" FORCE)
set(GGML_VULKAN OFF CACHE BOOL "" FORCE)
set(GGML_OPENCL OFF CACHE BOOL "" FORCE)
set(GGML_SYCL OFF CACHE BOOL "" FORCE)
set(GGML_BLAS OFF CACHE BOOL "" FORCE)
set(LLAMA_EMBED ON CACHE BOOL "" FORCE)

# ✅ 【注意】强制锁定 ggml 指令集，防止 CI 新机器自动启用 AVX-512/AMX
# 必须在 add_subdirectory 之前设置！
if(NOT APPLE AND NOT ANDROID)
    # 显式关闭所有高级指令集
    set(GGML_NATIVE        OFF CACHE BOOL "" FORCE)  # 禁止 native 优化
    set(GGML_AVX512        OFF CACHE BOOL "" FORCE)  # 禁止 AVX-512
    set(GGML_AMX           OFF CACHE BOOL "" FORCE)  # 禁止 AMX
    set(GGML_BMI2          OFF CACHE BOOL "" FORCE)  # 可选：BMI2 也有兼容性问题

    # 显式开启安全基线
    set(GGML_AVX2          ON  CACHE BOOL "" FORCE)
    set(GGML_FMA           ON  CACHE BOOL "" FORCE)
    set(GGML_F16C          ON  CACHE BOOL "" FORCE)

    # ✅ CI 日志可见性
    message(STATUS "🔒 ggml ISA locked: AVX2+FMA+F16C (AVX-512/AMX/BMI2/NATIVE disabled)")
endif()

add_subdirectory(${LLAMA_ROOT} ${CMAKE_BINARY_DIR}/llama.cpp)

# 确保 ggml/llama 目标也获得正确的编译选项
if(NOT APPLE AND NOT ANDROID AND NOT WIN32)
    # ✅ 确保 ggml 和 llama 库本身也用 AVX2 编译（而非默认的 baseline 或过高的 AVX-512）
    foreach(_ggml_target ggml llama)
        if(TARGET ${_ggml_target})
            target_compile_options(${_ggml_target} PRIVATE -mavx2 -mf16c -mfma)
            target_compile_definitions(${_ggml_target} PRIVATE
                GGML_USE_AVX2 GGML_F16C
            )
        endif()
    endforeach()
endif()

# Windows MSVC 也需要约束最高指令集
if(WIN32 AND MSVC)
    foreach(_ggml_target ggml llama)
        if(TARGET ${_ggml_target})
            # MSVC 下通过 /arch:AVX2 限定最高指令集（不会自动升到 AVX-512）
            target_compile_options(${_ggml_target} PRIVATE /arch:AVX2)
        endif()
    endforeach()
endif()

# llama.cpp common 工具库 (未被 llama 目标包含的部分)
file(GLOB LLAMA_COMMON_SRC CONFIGURE_DEPENDS ${LLAMA_ROOT}/common/*.cpp)
add_library(llama_common STATIC ${LLAMA_COMMON_SRC})
target_include_directories(llama_common PUBLIC
    ${LLAMA_ROOT}/common
    ${LLAMA_ROOT}/include
    ${LLAMA_ROOT}/vendor
    ${LLAMA_ROOT}/vendor/nlohmann
)
target_link_libraries(llama_common PUBLIC llama)

# ✅ 关键：MSVC 下强制 UTF-8 源码+执行字符集
if(MSVC)
    target_compile_options(llama_common PRIVATE /utf-8)
endif()

# ============================================================
# 3. 第三方库: zlib + QuaZip + cmark-gfm + sqlite
# ============================================================
# --- zlib ---
file(GLOB ZLIB_SOURCES CONFIGURE_DEPENDS lib/zlib/*.c)
add_library(knot_zlib STATIC ${ZLIB_SOURCES})
target_include_directories(knot_zlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib/zlib)

# ✅ 非 Windows 平台需要 unistd.h 提供 lseek/read/close
if(NOT WIN32)
    target_compile_definitions(knot_zlib PRIVATE Z_HAVE_UNISTD_H)
endif()

# --- QuaZip (AUTOMOC 必须在添加源文件前设置) ---
add_library(knot_quazip STATIC)
set_target_properties(knot_quazip PROPERTIES AUTOMOC ON)

# ✅ 关键：强制静态模式，禁止 dllimport/dllexport
target_compile_definitions(knot_quazip PRIVATE
    QUAZIP_STATIC          # 告诉 QuaZip 内部代码不要使用 __declspec
    QUAZIP_BUILD_STATIC    # 部分版本使用此宏
)

file(GLOB QUAZIP_SOURCES CONFIGURE_DEPENDS lib/quazip/*.cpp lib/quazip/*.c)
target_sources(knot_quazip PRIVATE ${QUAZIP_SOURCES})
target_include_directories(knot_quazip PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib/quazip)
target_link_libraries(knot_quazip PUBLIC knot_zlib Qt6::Core)

# --- cmark-gfm ---
file(GLOB CMARK_SRC CONFIGURE_DEPENDS lib/cmark-gfm/src/*.c)
file(GLOB CMARK_EXT CONFIGURE_DEPENDS lib/cmark-gfm/extensions/*.c)
list(FILTER CMARK_EXT EXCLUDE REGEX ".*/(subscript|superscript)\\.c$")
add_library(knot_cmark_gfm STATIC ${CMARK_SRC} ${CMARK_EXT})
target_include_directories(knot_cmark_gfm PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/cmark-gfm/include
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/cmark-gfm/src
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/cmark-gfm/extensions
)

# --- SQLite + vec ---
add_library(knot_sqlite STATIC lib/sqlite/sqlite3.c lib/sqlite_vec/sqlite-vec.c)
target_include_directories(knot_sqlite PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/sqlite
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/sqlite_vec
)
if(WIN32)
    target_link_libraries(knot_sqlite PUBLIC shell32)
endif()

# ============================================================
# 4. Scintilla / QScintilla (独立静态库)
# ============================================================
if(NOT ANDROID)
    add_subdirectory(lib/qsci)
endif()

# ============================================================
# 5. 业务源码收集 (不再包含 llama.cpp 源码，避免重复编译)
# ============================================================
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
    src/*.cpp
    src/*.h
    lib/diff/*.cpp
)

file(GLOB_RECURSE APP_FORMS CONFIGURE_DEPENDS src/*.ui)

# set(APP_RESOURCES knotqml.qrc res.qrc)
set(APP_RESOURCES res.qrc)

set(APP_TRANSLATIONS
    src/cn.ts
    lib/qsci/qscintilla_cs.ts
    lib/qsci/qscintilla_de.ts
    lib/qsci/qscintilla_es.ts
    lib/qsci/qscintilla_fr.ts
    lib/qsci/qscintilla_pt_br.ts
)

# ============================================================
# 6. 主目标
# ============================================================

# --- 6.1 收集平台特定的资源文件与选项 ---
set(PLATFORM_RESOURCES "")
set(PLATFORM_OPTIONS "")

if(APPLE)
    # macOS (arm64 / x86_64 通用，由 CMAKE_OSX_ARCHITECTURES 决定)
    set(MACOSX_ICON "${CMAKE_CURRENT_SOURCE_DIR}/res/icon.icns")
    if(EXISTS "${MACOSX_ICON}")
        list(APPEND PLATFORM_RESOURCES ${MACOSX_ICON})
        set_source_files_properties(${MACOSX_ICON} PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources"
        )
        message(STATUS "✅ macOS icon configured: ${MACOSX_ICON}")
    else()
        message(WARNING "⚠️ macOS icon not found: ${MACOSX_ICON}, app will use default icon.")
    endif()
    list(APPEND PLATFORM_OPTIONS MACOSX_BUNDLE)

elseif(WIN32)
    # Windows
    set(WINDOWS_RC "${CMAKE_CURRENT_SOURCE_DIR}/win.rc")
    if(EXISTS "${WINDOWS_RC}")
        list(APPEND PLATFORM_RESOURCES ${WINDOWS_RC})
        message(STATUS "✅ Windows icon resource configured: ${WINDOWS_RC}")
    else()
        message(WARNING "⚠️ Windows RC file not found: ${WINDOWS_RC}, exe will have no icon.")
    endif()
    list(APPEND PLATFORM_OPTIONS WIN32)  # GUI 子系统，不弹终端
endif()
# Linux: 无需特殊选项，普通 ELF 可执行文件即可
# Android: qt_add_executable 自动处理为 MODULE 库，忽略 WIN32/MACOSX_BUNDLE

# --- 6.2 创建目标（统一使用 qt_add_executable） ---
qt_add_executable(${PROJECT_NAME}
    ${PLATFORM_OPTIONS}
    ${APP_SOURCES}
    ${APP_FORMS}
    ${APP_RESOURCES}
    ${PLATFORM_RESOURCES}
)

# ✅ 版本注入
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/version.h.in"
    "${CMAKE_BINARY_DIR}/generated/version.h"
    @ONLY
)
target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_BINARY_DIR}/generated"
)

# --- 6.3 平台特定目标属性 ---
if(ANDROID)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android"
    )

    # ✅ 👇 Android 版本号从 CMake VERSION 自动获取
    math(EXPR ANDROID_VERSION_CODE
        "${PROJECT_VERSION_MAJOR} * 10000 + ${PROJECT_VERSION_MINOR} * 100 + ${PROJECT_VERSION_PATCH}"
    )
    set_target_properties(${PROJECT_NAME} PROPERTIES
        QT_ANDROID_VERSION_NAME "${PROJECT_VERSION}"
        QT_ANDROID_VERSION_CODE "${ANDROID_VERSION_CODE}"
    )
    message(STATUS "📱 Android Version: name=${PROJECT_VERSION}, code=${ANDROID_VERSION_CODE}")

elseif(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE_ICON_FILE "icon.icns"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.knot.app"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
    )
endif()
# Windows 无需额外属性，WIN32 选项已通过 PLATFORM_OPTIONS 传入
# Linux 无需额外属性

# ✅ Unity Build 必须在目标创建之后
# 开启 Unity Build
#set_target_properties(${PROJECT_NAME} PROPERTIES
#    UNITY_BUILD ON
#    UNITY_BUILD_BATCH_SIZE 8
#)

# 第三方静态库也开启 Unity Build
#foreach(lib knot_zlib knot_quazip knot_cmark_gfm knot_sqlite llama_common)
#    set_target_properties(${lib} PROPERTIES UNITY_BUILD ON UNITY_BUILD_BATCH_SIZE 16)
#endforeach()

# ============================================================
# 7. 头文件路径 (全平台公共，与平台无关)
# ============================================================
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}               # openssl/, lib/quazip/ 等平铺头文件
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/diff
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/scintilla/include
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/scintilla/lexlib
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/scintilla/src
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/qsci/QSci
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/qsci
    ${LLAMA_ROOT}/include
    ${LLAMA_ROOT}/ggml/include
    ${LLAMA_ROOT}/vendor
    ${LLAMA_ROOT}/vendor/nlohmann
    ${LLAMA_ROOT}/vendor/cpp-httplib
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/zlib
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/cmark-gfm/include
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/sqlite
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/sqlite_vec
)

# ============================================================
# 8. 编译定义与选项 (target 级别，不污染第三方库)
# ============================================================
target_compile_definitions(${PROJECT_NAME} PRIVATE
    SCI_LEXER
    LEXER_MARKDOWN_INCLUDED
    QSCINTILLA_HAVE_MARKDOWNLEXER
    HAVE_FSEEKO
    QUAZIP_STATIC
    SQLITE_VEC_STATIC
    VECTOR_SEARCH
    $<$<CONFIG:Release>:QT_NO_DEBUG>

)

if(NOT WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE Z_HAVE_UNISTD_H)
endif()

if(WIN32)
    target_compile_options(${PROJECT_NAME} PRIVATE
            /FI${CMAKE_CURRENT_SOURCE_DIR}/lib/qsci/win32_undef_macros.h
        )

elseif(APPLE)
        # ✅ 使用 CMake 原生变量，替代 execute_process(uname -m)
        # 在 arm64 CI 上 → CMAKE_SYSTEM_PROCESSOR == "arm64"
        # 在 x86_64 CI 上 → CMAKE_SYSTEM_PROCESSOR == "x86_64"
        if(CMAKE_OSX_ARCHITECTURES MATCHES "arm64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
            message(STATUS "✅ Targeting macOS ARM64 (NEON)")
            target_compile_definitions(${PROJECT_NAME} PRIVATE
                GGML_USE_NEON GGML_NO_AVX GGML_USE_DOTPROD GGML_USE_FP16_VECTOR_ARITHMETIC)
        else()
            message(STATUS "ℹ️ Targeting macOS x86_64 (AVX2)")
            target_compile_definitions(${PROJECT_NAME} PRIVATE GGML_USE_AVX2 GGML_F16C)
            target_compile_options(${PROJECT_NAME} PRIVATE -mavx2 -mf16c -mfma)
        endif()
elseif(ANDROID)
    target_compile_definitions(${PROJECT_NAME} PRIVATE GGML_USE_NEON GGML_NO_AVX GGML_USE_DOTPROD GGML_USE_FP16_VECTOR_ARITHMETIC)
elseif(UNIX)
    target_compile_definitions(${PROJECT_NAME} PRIVATE GGML_USE_AVX2 GGML_F16C GGML_NO_SCHED_BATCH _GNU_SOURCE)
    target_compile_options(${PROJECT_NAME} PRIVATE -mavx2 -mf16c -mfma)
endif()

# ============================================================
# 9. 链接库
# ============================================================
# 全平台公共链接
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network Qt6::PrintSupport
    Qt6::Charts Qt6::Sensors Qt6::Sql
    Qt6::Xml Qt6::Svg Qt6::Concurrent
    Qt6::Positioning
    Qt6::Core5Compat
    llama ggml llama_common
    knot_zlib knot_quazip knot_cmark_gfm knot_sqlite
    # ✅ 仅非 Android 平台链接 qscintilla
    $<$<NOT:$<BOOL:${ANDROID}>>:qscintilla>
)

# 平台特定链接
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        advapi32
        ${CMAKE_CURRENT_SOURCE_DIR}/openssl/lib/libcrypto.lib
        ${CMAKE_CURRENT_SOURCE_DIR}/openssl/lib/libssl.lib
    )
    if(MSVC)
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:/OPT:NOREF /OPT:NOICF>
            $<$<CONFIG:Release>:/INCLUDE:ggml_backend_cpu_reg>
            $<$<CONFIG:Release>:/INCLUDE:ggml_backend_cpu_init>
        )
    endif()
elseif(APPLE)
    find_package(OpenSSL REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        OpenSSL::SSL OpenSSL::Crypto

    )
elseif(ANDROID)
    # ✅ Android Bionic 已将 pthread 合入 libc，无需单独链接
    target_link_libraries(${PROJECT_NAME} PRIVATE dl ssl crypto)
    if(CMAKE_ANDROID_ARCH_ABI STREQUAL "arm64-v8a")
        target_link_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/android-openssl/ssl_3/v8a)
    elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL "x86_64")
        target_link_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/android-openssl/ssl_3/x86_64)
    endif()
elseif(UNIX)
    find_package(OpenSSL REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        OpenSSL::SSL OpenSSL::Crypto
        pthread dl
    )
endif()

# ============================================================
# 10. QML 导入路径
# ============================================================
# set_target_properties(${PROJECT_NAME} PROPERTIES
#    QT_QML_IMPORT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src/qmlsrc"
# )

# ============================================================
# 11. Android 部署配置 (✅ 已修复 libomp 查找逻辑)
# ============================================================
if(ANDROID)
    set(_ndk_libomp_path "")

    # ✅ 方法1: 遍历编译器隐式链接目录（最可靠，跨平台通用）
    foreach(_dir ${CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES})
        if(EXISTS "${_dir}/libomp.so")
            set(_ndk_libomp_path "${_dir}/libomp.so")
            message(STATUS "✅ Found libomp.so via implicit link dirs: ${_ndk_libomp_path}")
            break()
        endif()
    endforeach()

    # ✅ 方法2: Fallback - 通过 ANDROID_NDK 变量拼接（兼容所有 host 平台）
    if(NOT EXISTS "${_ndk_libomp_path}" AND DEFINED ANDROID_NDK)
        # 自动检测 host 平台，不再硬编码 windows-x86_64
        if(CMAKE_HOST_WIN32)
            set(_prebuilt_dir "windows-x86_64")
        elseif(CMAKE_HOST_APPLE)
            set(_prebuilt_dir "darwin-x86_64")
        else()
            set(_prebuilt_dir "linux-x86_64")
        endif()

        set(_candidate "${ANDROID_NDK}/toolchains/llvm/prebuilt/${_prebuilt_dir}/sysroot/usr/lib/${CMAKE_ANDROID_ARCH_TRIPLE}/libomp.so")
        if(EXISTS "${_candidate}")
            set(_ndk_libomp_path "${_candidate}")
            message(STATUS "✅ Found libomp.so via NDK fallback: ${_ndk_libomp_path}")
        endif()
    endif()

    # ✅ 关键：找不到时立即在 configure 阶段报错，而非等到运行时崩溃
    if(NOT EXISTS "${_ndk_libomp_path}")
        message(FATAL_ERROR
            "❌ Cannot find libomp.so for ABI=${CMAKE_ANDROID_ARCH_ABI}!\n"
            "   Searched implicit dirs: ${CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES}\n"
            "   ANDROID_NDK=${ANDROID_NDK}\n"
            "   Please verify your NDK installation includes OpenMP support."
        )
    endif()

    # ✅ 验证 OpenSSL 库也存在
    set(_openssl_crypto "${CMAKE_CURRENT_SOURCE_DIR}/android-openssl/ssl_3/v8a/libcrypto_3.so")
    set(_openssl_ssl "${CMAKE_CURRENT_SOURCE_DIR}/android-openssl/ssl_3/v8a/libssl_3.so")
    if(NOT EXISTS "${_openssl_crypto}" OR NOT EXISTS "${_openssl_ssl}")
        message(FATAL_ERROR "❌ Android OpenSSL libraries not found at: android-openssl/ssl_3/v8a/")
    endif()

    # ✅ 安全追加 QT_ANDROID_EXTRA_LIBS，兼容所有 Qt6 版本
    get_target_property(_existing_extra_libs ${PROJECT_NAME} QT_ANDROID_EXTRA_LIBS)
    if(NOT _existing_extra_libs OR _existing_extra_libs STREQUAL "_existing_extra_libs-NOTFOUND")
        set(_existing_extra_libs "")
    endif()

    set_target_properties(${PROJECT_NAME} PROPERTIES
        QT_ANDROID_EXTRA_LIBS "${_existing_extra_libs};${_openssl_crypto};${_openssl_ssl};${_ndk_libomp_path}"
    )

    message(STATUS "📦 Android Extra Libs configured:")
    message(STATUS "   - ${_openssl_crypto}")
    message(STATUS "   - ${_openssl_ssl}")
    message(STATUS "   - ${_ndk_libomp_path}")
endif()

# ============================================================
# 12. 输出目录
# ============================================================
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}/bin/debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin/release"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}/bin/debug"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin/release"
)

# ============================================================
# 13. 翻译
# ============================================================

# ✅ 关键：显式收集所有 QML/JS 源文件
# qt_add_translations 不会自动递归扫描目录寻找 QML 文件
# file(GLOB_RECURSE APP_QML_SOURCES CONFIGURE_DEPENDS
#    src/*.qml
#    src/*.js
# )

qt_add_translations(${PROJECT_NAME}
    TS_FILES ${APP_TRANSLATIONS}
    QM_FILES_OUTPUT_VARIABLE QM_FILES
    # ✅ 将 QML/JS 文件作为额外源文件传入，使 lupdate 能够提取其中的 qsTr()
    # SOURCES ${APP_QML_SOURCES}
)

# ============================================================
# 更新翻译 (便捷别名)
# ============================================================
if(TARGET Knot_update_translations)
    # 创建一个纯别名 target，不执行任何额外命令
    # 构建 update_translations 等价于构建 Knot_update_translations
    add_custom_target(update_translations
        DEPENDS Knot_update_translations
        COMMENT "🔄 Updating translation files (.ts)..."
    )
endif()

# ============================================================
# 14. 最终化目标 (⚠️ 必须在所有 QT_ANDROID_* 属性设置之后)
# ============================================================
if(ANDROID)
    qt_finalize_target(${PROJECT_NAME})
endif()