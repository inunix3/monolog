# Copyright (c) 2025-present inunix3
#
# This file is licensed under the MIT License (Expat) (see LICENSE.md in the
# root of project).

macro(create_test NAME)
    set(SOURCES ${ARGN})

    add_executable(${NAME} $<TARGET_OBJECTS:monolog-obj> ${SOURCES})
    target_compile_options(${NAME} PRIVATE ${COMPILE_OPTIONS})
    target_include_directories(${NAME} PRIVATE "${PROJECT_SOURCE_DIR}/include")
    target_link_libraries(${NAME} PRIVATE greatest)

    if(USE_ASAN)
        target_link_options(${NAME} PRIVATE -fsanitize=address)
    elseif(USE_UBSAN)
        target_link_options(${NAME} PRIVATE -fsanitize=undefined)
    endif()
endmacro()

create_test(vector_test vector.c)
create_test(hashmap_test hashmap.c)
create_test(lexer_test lexer.c)

set(PARSER_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/parser/expressions.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/parser/invalid.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/parser/main.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/parser/shared.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/parser/statements.c"
)

create_test(parser_test ${PARSER_SOURCES})

add_custom_command(
  TARGET parser_test
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E create_symlink
    "${CMAKE_CURRENT_SOURCE_DIR}/golden-files"
    "$<TARGET_FILE_DIR:parser_test>/golden-files"
)

set(SEMCK_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/semck/invalid.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/semck/main.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/semck/valid.c"
)

create_test(semck_test ${SEMCK_SOURCES})

set(INTERP_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/interp/valid.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/interp/invalid.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/interp/main.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/interp/shared.h"
)

create_test(interp_test ${INTERP_SOURCES})
