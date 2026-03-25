set(TEST_TYPE "INTEGRATION")

add_subdirectory(plugins)

link_directories(${PROJECT_BINARY_DIR}/test)
include_directories(${PROJECT_SOURCE_DIR}/test)
include_directories(${PROJECT_SOURCE_DIR}/src)

if(NOT WIN32)
  # Disable test on Windows, it deals with UNIX signals
  set(tests faulty_plugins.cc)

  gz_build_tests(TYPE INTEGRATION
                 SOURCES ${tests}
                 TEST_LIST test_targets
                 ENVIRONMENT GZ_LAUNCH_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX})
  foreach(test ${test_targets})
    target_compile_definitions(${test} PRIVATE
        "bad_plugins_LIB=\"$<TARGET_FILE:bad_plugins>\"")
  endforeach()
endif()
