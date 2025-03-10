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
    "${SRC_DIR}/main.c"
    "${SRC_DIR}/vector.c"
)

add_executable(monolog ${HEADERS} ${SOURCES})

if (COMPILER STREQUAL "gcc" OR COMPILER STREQUAL "clang")
  set(COMPILE_OPTIONS
      -Wall -Wextra -Wconversion -Wsign-conversion -Wshadow -fstack-clash-protection
  )

  set(COMPILE_OPTIONS ${COMPILE_OPTIONS} $<$<CONFIG:Debug>:-Og>)
endif()

if (USE_ASAN)
  set(COMPILE_OPTIONS ${COMPILE_OPTIONS} -fsanitize=address)
  target_link_options(monolog PRIVATE -fsanitize=address)
elseif (USE_UBSAN)
  set(COMPILE_OPTIONS ${COMPILE_OPTIONS} -fsanitize=undefined)
  target_link_options(monolog PRIVATE -fsanitize=undefined)
endif()

if (WIN32)
    set_target_properties(monolog PROPERTIES WIN32_EXECUTABLE $<IF:$<CONFIG:Release>,ON,OFF>)
endif()

target_compile_options(monolog PRIVATE ${COMPILE_OPTIONS})
target_include_directories(monolog PRIVATE "${PROJECT_SOURCE_DIR}/include")
