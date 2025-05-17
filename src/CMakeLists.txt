# Copyright (c) 2025-present inunix3
#
# This file is licensed under the MIT License (Expat)
# (see LICENSE.md in the root of project).

set(INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include/monolog")
set(SRC_DIR "${PROJECT_SOURCE_DIR}/src")

set(HEADERS
    "${INCLUDE_DIR}/ast.h"
    "${INCLUDE_DIR}/builtin_funcs.h"
    "${INCLUDE_DIR}/cli.h"
    "${INCLUDE_DIR}/diagnostic.h"
    "${INCLUDE_DIR}/environment.h"
    "${INCLUDE_DIR}/expr_result.h"
    "${INCLUDE_DIR}/function.h"
    "${INCLUDE_DIR}/hashmap.h"
    "${INCLUDE_DIR}/interp.h"
    "${INCLUDE_DIR}/lexer.h"
    "${INCLUDE_DIR}/parser.h"
    "${INCLUDE_DIR}/scope.h"
    "${INCLUDE_DIR}/semck.h"
    "${INCLUDE_DIR}/src_info.h"
    "${INCLUDE_DIR}/stmt_result.h"
    "${INCLUDE_DIR}/strbuf.h"
    "${INCLUDE_DIR}/type.h"
    "${INCLUDE_DIR}/utils.h"
    "${INCLUDE_DIR}/value.h"
    "${INCLUDE_DIR}/variable.h"
    "${INCLUDE_DIR}/vector.h"
)

set(SOURCES
    "${SRC_DIR}/ast.c"
    "${SRC_DIR}/diagnostic.c"
    "${SRC_DIR}/environment.c"
    "${SRC_DIR}/function.c"
    "${SRC_DIR}/hashmap.c"
    "${SRC_DIR}/interp.c"
    "${SRC_DIR}/lexer.c"
    "${SRC_DIR}/parser.c"
    "${SRC_DIR}/scope.c"
    "${SRC_DIR}/semck.c"
    "${SRC_DIR}/strbuf.c"
    "${SRC_DIR}/type.c"
    "${SRC_DIR}/utils.c"
    "${SRC_DIR}/vector.c"
)

add_library(monolog-obj OBJECT ${HEADERS} ${SOURCES})
add_executable(monolog-bin "${SRC_DIR}/main.c" $<TARGET_OBJECTS:monolog-obj>)

if (COMPILER STREQUAL "gcc" OR COMPILER STREQUAL "clang")
  set(COMPILE_OPTIONS
      -Wall -Wextra -Wconversion -Wsign-conversion -Wshadow -fstack-clash-protection
      -Wno-missing-braces
  )

  set(COMPILE_OPTIONS ${COMPILE_OPTIONS} $<$<CONFIG:Debug>:-O0>)
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

target_compile_options(monolog-obj PRIVATE ${COMPILE_OPTIONS})
target_include_directories(monolog-obj PRIVATE "${PROJECT_SOURCE_DIR}/include")

target_compile_options(monolog-bin PRIVATE ${COMPILE_OPTIONS})
target_include_directories(monolog-bin PRIVATE "${PROJECT_SOURCE_DIR}/include")
set_target_properties(monolog-bin PROPERTIES OUTPUT_NAME "monolog")

target_link_libraries(monolog-bin PRIVATE isocline)
