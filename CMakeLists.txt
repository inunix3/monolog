# Copyright (c) 2025-present inunix3
#
# This file is licensed under the MIT License (Expat)
# (see LICENSE.md in the root of project).

cmake_minimum_required(VERSION 3.13)

project(monolog LANGUAGES C VERSION 1.0.0)

option(BUILD_TESTS "build unit tests" OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_subdirectory(third-party)
add_subdirectory(src)

if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
