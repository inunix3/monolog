# Copyright (c) 2025-present inunix3
#
# This file is licensed under the MIT License (Expat)
# (see LICENSE.md in the root of project).

cmake_minimum_required(VERSION 3.13)

project(monolog LANGUAGES C VERSION 1.0.0)

option(BUILD_TESTS "build unit tests" OFF)
option(USE_ASAN "use address sanitizer" OFF)
option(USE_UBSAN "use undefined behavior sanitizer" OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(COMPILER "gcc")
elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang" AND CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
    set(COMPILER "clang")
elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang" AND CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    set(COMPILER "clang-cl")
elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    set(COMPILER "msvc")
else()
    message(WARNING "compiler is unknown to ${PROJECT_NAME}")
endif()

add_subdirectory(third-party)
add_subdirectory(src)

if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
