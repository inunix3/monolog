cmake_minimum_required(VERSION 3.11)

project(monolog LANGUAGES C VERSION 1.0.0)

option(BUILD_TESTS "build unit tests" OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_subdirectory(src)

if (BUILD_TESTS)
    add_subdirectory(third-party/Unity)

    enable_testing()
    add_subdirectory(tests)
endif()
