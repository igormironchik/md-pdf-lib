# SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
# SPDX-License-Identifier: GPL-3.0-or-later

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

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
    set(MD_PDF_SKIA_BUILD_PATH "${MD_PDF_SKIA_BUILD_PATH}" PARENT_SCOPE)
endif()

link_directories(${MD_PDF_SKIA_BUILD_PATH})

if (UNIX)
    find_package(ICU REQUIRED COMPONENTS uc)
    find_package(WebP REQUIRED)
    find_package(harfbuzz REQUIRED)

    list(APPEND PLATFORM_LIBS WebP::webpdemux WebP::webp harfbuzz::harfbuzz harfbuzz::subset)

    set (ICU_LIBS ICU::uc)
else()
    set(ICU_LIBS icu)

    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        string(APPEND SKIA_CFLAGS "\"/MD\"")
    else()
        string(APPEND SKIA_CFLAGS "\"/MDd\"")
    endif()

    set(SKIA_ARGS_GN_FILE "${MD_PDF_SKIA_BUILD_PATH}/args.gn")

    file(GENERATE 
        OUTPUT "${SKIA_ARGS_GN_FILE}"
        CONTENT "is_official_build = true
skia_use_system_expat = false
skia_use_system_harfbuzz = false
skia_use_system_icu = false
skia_use_system_libjpeg_turbo = false
skia_use_system_libpng = false
skia_use_system_libwebp = false
skia_use_system_zlib = true
skia_enable_tools=false
extra_cflags = [ \"-I$<JOIN:$<TARGET_PROPERTY:ZLIB::ZLIB,INTERFACE_INCLUDE_DIRECTORIES>,\", \"-I>\", ${SKIA_CFLAGS} ]
extra_ldflags = [ \"$<TARGET_PROPERTY:ZLIB::ZLIB,INTERFACE_LINK_DIRECTORIES>/zlib.lib\" ]
"
    )
endif()

add_library(md-pdf-lib STATIC ${LIB_SRC})

target_link_libraries(md-pdf-lib
    md4qt::md4qt
    LaTeX
    KF${KF_MAJOR_VERSION}::SyntaxHighlighting
    Qt6::Network
    Qt6::Core
    bentleyottmann
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
    ${ICU_LIBS}
    ${PLATFORM_LIBS}
    ZLIB::ZLIB)

target_include_directories(md-pdf-lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/Skia
    ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty
    ${CMAKE_CURRENT_SOURCE_DIR})

if(BUILD_TEST_LIBRARY)
    find_package(Qt6 REQUIRED COMPONENTS Test)

    add_library(md-pdf-lib-test STATIC ${LIB_SRC})

    target_compile_definitions(md-pdf-lib-test PUBLIC MD_PDF_TESTING)

    set(NORMAL_FONT ${CMAKE_CURRENT_SOURCE_DIR}/../tests/fonts/DroidSerif/droid_serif.ttf)
    set(ITALIC_FONT ${CMAKE_CURRENT_SOURCE_DIR}/../tests/fonts/DroidSerif/droid_serif_italic.ttf)
    set(BOLD_FONT ${CMAKE_CURRENT_SOURCE_DIR}/../tests/fonts/DroidSerif/droid_serif_bold.ttf)
    set(BOLD_ITALIC_FONT ${CMAKE_CURRENT_SOURCE_DIR}/../tests/fonts/DroidSerif/droid_serif_italic_bold.ttf)
    set(MONO_FONT ${CMAKE_CURRENT_SOURCE_DIR}/../tests/fonts/SpaceMono/SpaceMono-Regular.ttf)
    set(MONO_ITALIC_FONT ${CMAKE_CURRENT_SOURCE_DIR}/../tests/fonts/SpaceMono/SpaceMono-Italic.ttf)
    set(MONO_BOLD_FONT ${CMAKE_CURRENT_SOURCE_DIR}/../tests/fonts/SpaceMono/SpaceMono-Bold.ttf)
    set(MONO_BOLD_ITALIC_FONT ${CMAKE_CURRENT_SOURCE_DIR}/../tests/fonts/SpaceMono/SpaceMono-BoldItalic.ttf)
    set(WORKING_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/../tests/auto/test_render)

    configure_file(test_const.h.in test_const.h @ONLY)

    target_include_directories(md-pdf-lib-test INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    )

    target_link_libraries(md-pdf-lib-test
        md4qt::md4qt
        LaTeX
        KF${KF_MAJOR_VERSION}::SyntaxHighlighting
        Qt6::Network
        Qt6::Core
        Qt6::Test
        bentleyottmann
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
        ${ICU_LIBS}
        ${PLATFORM_LIBS}
        ZLIB::ZLIB)

    target_include_directories(md-pdf-lib-test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/Skia
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty
        ${CMAKE_CURRENT_SOURCE_DIR})

    add_dependencies(md-pdf-lib-test md-pdf-lib)
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

if(WIN32)
    add_custom_command(
        TARGET md-pdf-lib
        PRE_BUILD
        COMMAND ${FETCH_GN_COMMAND}
        COMMAND "${GN_EXEC}" gen "${MD_PDF_SKIA_BUILD_PATH}"
        COMMAND ninja -C "${MD_PDF_SKIA_BUILD_PATH}" skia
        COMMAND ninja -C "${MD_PDF_SKIA_BUILD_PATH}" modules
        COMMENT "Configuring and building Skia..."
        VERBATIM
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/Skia"
    )

    install(FILES "${MD_PDF_SKIA_BUILD_PATH}/icudtl.dat" DESTINATION ${CMAKE_INSTALL_BINDIR})
else()
    add_custom_command(
        TARGET md-pdf-lib
        PRE_BUILD
        COMMAND ${FETCH_GN_COMMAND}
        COMMAND "${GN_EXEC}" gen "${MD_PDF_SKIA_BUILD_PATH}" "--args=is_official_build=true skia_enable_tools=false skia_use_system_expat=true skia_use_system_harfbuzz=true skia_use_system_icu=true skia_use_system_libjpeg_turbo=true skia_use_system_libpng=true skia_use_system_libwebp=true skia_use_system_zlib=true extra_cflags=[${SKIA_CFLAGS}] extra_ldflags=[${SKIA_LDFLAGS}]"
        COMMAND ninja -C "${MD_PDF_SKIA_BUILD_PATH}" skia
        COMMAND ninja -C "${MD_PDF_SKIA_BUILD_PATH}" modules
        COMMENT "Configuring and building Skia..."
        VERBATIM
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/Skia"
    )
endif()
