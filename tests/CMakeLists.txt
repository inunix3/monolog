# Copyright (c) 2025-present inunix3
#
# This file is licensed under the MIT License (Expat)
# (see LICENSE.md in the root of project).

add_executable(lexer_test $<TARGET_OBJECTS:monolog-obj> lexer.c)
target_compile_options(lexer_test PRIVATE ${COMPILE_OPTIONS})
target_include_directories(lexer_test PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(lexer_test PRIVATE greatest)

set(PARSER_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/parser/expressions.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/parser/invalid.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/parser/main.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/parser/shared.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/parser/statements.c"
)

add_executable(parser_test $<TARGET_OBJECTS:monolog-obj> ${PARSER_SOURCES})
target_compile_options(parser_test PRIVATE ${COMPILE_OPTIONS})
target_include_directories(parser_test PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(parser_test PRIVATE greatest)

add_custom_command(
    TARGET parser_test
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/golden-files" "${CMAKE_CURRENT_BINARY_DIR}/golden-files"
)
