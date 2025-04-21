if(BUILD_TESTS)
    add_library(greatest INTERFACE)
    target_include_directories(greatest INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/greatest")
endif()

add_subdirectory(isocline)
