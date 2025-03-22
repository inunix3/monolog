add_executable(lexer_test $<TARGET_OBJECTS:monolog-obj> lexer.c)
target_compile_options(lexer_test PRIVATE ${COMPILE_OPTIONS})
target_include_directories(lexer_test PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(lexer_test PRIVATE unity)
