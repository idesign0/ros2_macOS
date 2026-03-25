set(TEST_TYPE "INTEGRATION")

gz_get_sources(tests)

# Test symbols having the right name on linux only
if (UNIX AND NOT APPLE)
  configure_file(all_symbols_have_version.bash.in ${CMAKE_CURRENT_BINARY_DIR}/all_symbols_have_version.bash @ONLY)
  add_test(NAME INTEGRATION_versioned_symbols
    COMMAND bash ${CMAKE_CURRENT_BINARY_DIR}/all_symbols_have_version.bash $<TARGET_FILE:${PROJECT_LIBRARY_TARGET_NAME}>)
endif()

gz_build_tests(TYPE INTEGRATION SOURCES ${tests})

add_executable(subprocess_executable subprocess/subprocess_main.cc)
target_include_directories(subprocess_executable PUBLIC
  ${PROJECT_SOURCE_DIR}/cli/include/
  ${PROJECT_SOURCE_DIR}/cli/include/vendored-cli/)
target_link_libraries(subprocess_executable ${PROJECT_LIBRARY_TARGET_NAME})

if(TARGET INTEGRATION_subprocess_TEST)
  target_compile_definitions(INTEGRATION_subprocess_TEST PRIVATE
    "SUBPROCESS_EXECUTABLE_PATH=\"$<TARGET_FILE:subprocess_executable>\"")
endif()

add_subdirectory(implptr)
