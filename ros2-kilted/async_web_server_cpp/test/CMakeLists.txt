add_executable(test_web_server test_web_server.cpp)
target_compile_definitions(test_web_server
    PRIVATE
        "ASYNC_WEB_SERVER_CPP_TEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\""
)
target_link_libraries(test_web_server PRIVATE ${PROJECT_NAME})

set(generated_test_file "${CMAKE_CURRENT_BINARY_DIR}/tests")
set(TEST_WEB_SERVER_EXECUTABLE $<TARGET_FILE:test_web_server>)
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/tests.py.in"
    "${generated_test_file}.py.genexp"
    @ONLY
)
file(GENERATE
    OUTPUT "${generated_test_file}_$<CONFIG>.py"
    INPUT "${generated_test_file}.py.genexp"
)
ament_add_test(tests
    COMMAND "${generated_test_file}_$<CONFIG>.py"
    GENERATE_RESULT_FOR_RETURN_CODE_ZERO
)
