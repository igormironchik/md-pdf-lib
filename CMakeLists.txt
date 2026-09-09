# SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.19)

project(md-pdf-lib)

option(BUILD_MD_TESTS "Build tests? Default OFF." OFF)
option(BUILD_TEST_LIBRARY "Build test library? Default OFF." OFF)

if(BUILD_MD_TESTS)
    set(BUILD_TEST_LIBRARY ON)
endif()

set(KF_MIN_VERSION "6")
set(KF_MAJOR_VERSION "6")

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH "${ECM_MODULE_PATH}")

include(ECMEnableSanitizers)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)

if(WIN32)
    set(PLATFORM_LIBS Dwmapi Dwrite libwebp libpng libjpeg jsonreader harfbuzz)
endif()

if(UNIX)
    set(PLATFORM_LIBS freetype fontconfig dng_sdk piex png jpeg compression_utils_portable)
endif()

add_compile_definitions(QT_NO_KEYWORDS)

find_package(Qt6 REQUIRED COMPONENTS Core Network)

find_package(ZLIB REQUIRED)

find_package(KF${KF_MAJOR_VERSION} ${KF_MIN_VERSION} REQUIRED COMPONENTS
    SyntaxHighlighting
)

link_directories(${CMAKE_BINARY_DIR}/lib)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release"
		CACHE STRING "Choose the type of build."
		FORCE)
endif(NOT CMAKE_BUILD_TYPE)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED _DISABLE_INSTALLATION)
  set(_DISABLE_INSTALLATION FALSE)

	macro(install)
		if(NOT _DISABLE_INSTALLATION)
			_install(${ARGN})
		endif()
	endmacro()
endif()

set(_DISABLE_INSTALLATION TRUE)

if (NOT TARGET LaTeX)
    add_subdirectory(3rdparty/MicroTeX)
endif()

if (NOT TARGET md4qt::md4qt)
    set(BUILD_MD4QT_TESTS OFF CACHE INTERNAL "" FORCE)
    set(INSTALL_MD4QT OFF CACHE INTERNAL "" FORCE)
    add_subdirectory(3rdparty/md4qt)

    include_directories(${md4qt_INCLUDE_DIRECTORIES}/../..)

    set(md4qt_VERSION_STRING "${md4qt_VERSION_STRING}" PARENT_SCOPE)
endif()

set(_DISABLE_INSTALLATION FALSE)

add_subdirectory(src)

if(BUILD_MD_TESTS)
	enable_testing()

	add_subdirectory(tests)
endif()
