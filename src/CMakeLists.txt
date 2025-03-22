# Copyright (c) 2024-present inunix3.
# This file is distributed under the MIT license (https://opensource.org/license/mit/)

set(INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include/monolog")
set(SRC_DIR "${PROJECT_SOURCE_DIR}/src")

set(HEADERS
    "${INCLUDE_DIR}/lexer.h"
    "${INCLUDE_DIR}/vector.h"
)

set(SOURCES
    "${SRC_DIR}/lexer.c"
    "${SRC_DIR}/vector.c"
)

add_library(monolog-obj OBJECT ${HEADERS} ${SOURCES})
add_executable(monolog-bin "${SRC_DIR}/main.c" $<TARGET_OBJECTS:monolog-obj>)

if (COMPILER STREQUAL "gcc" OR COMPILER STREQUAL "clang")
  set(COMPILE_OPTIONS
      -Wall -Wextra -Wconversion -Wsign-conversion -Wshadow -fstack-clash-protection
  )

  set(COMPILE_OPTIONS ${COMPILE_OPTIONS} $<$<CONFIG:Debug>:-Og>)
endif()

if (USE_ASAN)
  set(COMPILE_OPTIONS ${COMPILE_OPTIONS} -fsanitize=address)
  target_link_options(monolog-obj PRIVATE -fsanitize=address)
  target_link_options(monolog-bin PRIVATE -fsanitize=address)
elseif (USE_UBSAN)
  set(COMPILE_OPTIONS ${COMPILE_OPTIONS} -fsanitize=undefined)
  target_link_options(monolog-obj PRIVATE -fsanitize=undefined)
  target_link_options(monolog-bin PRIVATE -fsanitize=undefined)
endif()

if (WIN32)
    set_target_properties(monolog-bin PROPERTIES WIN32_EXECUTABLE $<IF:$<CONFIG:Release>,ON,OFF>)
endif()

target_compile_options(monolog-obj PRIVATE ${COMPILE_OPTIONS})
target_include_directories(monolog-obj PRIVATE "${PROJECT_SOURCE_DIR}/include")

target_compile_options(monolog-bin PRIVATE ${COMPILE_OPTIONS})
target_include_directories(monolog-bin PRIVATE "${PROJECT_SOURCE_DIR}/include")
set_target_properties(monolog-bin PROPERTIES OUTPUT_NAME "monolog")
