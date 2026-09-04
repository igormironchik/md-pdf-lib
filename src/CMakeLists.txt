# SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
# SPDX-License-Identifier: GPL-3.0-or-later

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(WIN32)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(SKIA_CFLAGS "/MD")
    else()
        set(SKIA_CFLAGS "/MDd")
    endif()
endif()

set(LIB_SRC skia_paintdevice.h
    skia_paintdevice.cpp
    renderer.h
    renderer.cpp
    renderer_aux.h
    renderer_aux.cpp
    utils.h
    utils.cpp
    syntax.h
    syntax.cpp
    emphasis.h
    emphasis.cpp
    emoji_parser.h
    emoji_parser.cpp
    emoji.h)
	
qt6_add_resources(LIB_SRC resources.qrc)
qt6_add_resources(LIB_SRC ../3rdparty/MicroTeX/latex.qrc)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/MicroTeX/src
	${CMAKE_CURRENT_BINARY_DIR})

add_definitions(-DBUILD_QT)

if (NOT DEFINED MD_PDF_SKIA_BUILD_PATH)
    set(MD_PDF_SKIA_BUILD_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../builds/skia")
endif()

link_directories(${MD_PDF_SKIA_BUILD_PATH})

if (NOT BUILD_TEST_LIBRARY)
    add_library(md-pdf-lib STATIC ${LIB_SRC})

    target_link_libraries(md-pdf-lib
        md4qt::md4qt
        LaTeX
        KF${KF_MAJOR_VERSION}::SyntaxHighlighting
        Qt6::Network
        Qt6::Core
        bentleyottmann
        compression_utils_portable
        skresources
        skcms
        skottie
        skparagraph
        sksg
        svg
        wuffs
        skshaper
        skunicode_icu
        skunicode_core
        skia
        expat
        harfbuzz
        icu
        zlib
        ${PLATFORM_LIBS}
        ZLIB::ZLIB)

    target_include_directories(md-pdf-lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/Skia
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty
        ${CMAKE_CURRENT_SOURCE_DIR})
else()
    find_package(Qt6 REQUIRED COMPONENTS Test)

    add_library(md-pdf-lib-test STATIC ${LIB_SRC})

    target_compile_definitions(md-pdf-lib-test PUBLIC MD_PDF_TESTING)

    target_link_libraries(md-pdf-lib-test
        md4qt::md4qt
        markdown-widgets
        LaTeX
        ${ADDITIONAL_KDE_TARGETS}
        Qt6::Network
        Qt6::Core
        bentleyottmann
        compression_utils_portable
        skresources
        skcms
        skottie
        skparagraph
        sksg
        svg
        wuffs
        skshaper
        skunicode_icu
        skunicode_core
        skia
        expat
        harfbuzz
        icu
        zlib
        ${PLATFORM_LIBS}
        ZLIB::ZLIB
        Qt6::Test
        md-pdf-lib-test-consts)
endif()

find_program(SYSTEM_GN gn)

if(SYSTEM_GN)
    set(GN_EXEC "${SYSTEM_GN}")
    set(FETCH_GN_COMMAND "")

    set(SKIA_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/Skia/bin")
    set(SKIA_GN_TARGET "${SKIA_BIN_DIR}/gn")

    if(NOT EXISTS "${SKIA_GN_TARGET}")
        message(STATUS "Copying system GN: ${SYSTEM_GN} -> ${SKIA_GN_TARGET}")
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E copy "${SYSTEM_GN}" "${SKIA_GN_TARGET}"
            RESULT_VARIABLE COPY_RESULT
        )
        if(NOT COPY_RESULT EQUAL 0)
            message(FATAL_ERROR "Failed to copy system GN")
        endif()
    endif()
else()
    set(GN_EXEC "bin/gn")
    set(FETCH_GN_COMMAND COMMAND python3 bin/fetch-gn)
endif()

add_custom_command(
    TARGET md-pdf-lib
    PRE_BUILD
    COMMAND ${FETCH_GN_COMMAND}
    COMMAND "${GN_EXEC}" gen "${MD_PDF_SKIA_BUILD_PATH}" "--args=is_official_build=true skia_use_system_expat=false skia_use_system_harfbuzz=false skia_use_system_icu=false skia_use_system_libjpeg_turbo=false skia_use_system_libpng=false skia_use_system_libwebp=false skia_use_system_zlib=false extra_cflags=[\"${SKIA_CFLAGS}\"]"
    COMMAND ninja -C "${MD_PDF_SKIA_BUILD_PATH}"
    COMMENT "Configuring and building Skia..."
    VERBATIM
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/Skia"
)
