# Copyright (c) 2025-present inunix3
#
# This file is licensed under the MIT License (Expat)
# (see LICENSE.md in the root of project).

if(BUILD_TESTS)
    add_library(greatest INTERFACE)
    target_include_directories(greatest INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/greatest")
endif()

add_subdirectory(isocline)
