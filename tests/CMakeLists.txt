# Copyright (c) 2025-present inunix3
#
# This file is licensed under the MIT License (Expat)
# (see LICENSE.md in the root of project).

add_executable(lexer_test $<TARGET_OBJECTS:monolog-obj> lexer.c)
target_compile_options(lexer_test PRIVATE ${COMPILE_OPTIONS})
target_include_directories(lexer_test PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(lexer_test PRIVATE greatest)
