gz_get_sources(tests)

if (NOT HAVE_GZ_TOOLS)
  list(REMOVE_ITEM tests gz_TEST.cc)
endif()

gz_build_tests(TYPE INTEGRATION SOURCES ${tests} TEST_LIST test_targets
               ENVIRONMENT GZ_MSG_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX})

if(TARGET INTEGRATION_gz_TEST)
  target_compile_definitions(INTEGRATION_gz_TEST PRIVATE
    "GZ_MSGS_EXECUTABLE_PATH=\"$<TARGET_FILE:gz-msgs>\""
    "GZ_MSGS_COMPLETION_SCRIPT_PATH=\"${PROJECT_SOURCE_DIR}/core/cmd/msgs.bash_completion.sh\"")
endif()

if(TARGET INTEGRATION_descriptors)
  target_compile_definitions(INTEGRATION_descriptors PRIVATE
    "-DGZ_MSGS_TEST_PATH=\"${PROJECT_SOURCE_DIR}/test\"")
endif()

if(TARGET INTEGRATION_Factory_TEST)
  target_compile_definitions(INTEGRATION_Factory_TEST PRIVATE
    "GZ_MSGS_TEST_PATH=\"${PROJECT_SOURCE_DIR}/test\"")
endif()

if(TARGET INTEGRATION_gz_TEST)
    target_compile_definitions(INTEGRATION_gz_TEST PUBLIC
      "-DDETAIL_GZ_CONFIG_PATH=\"${CMAKE_BINARY_DIR}/test/conf/$<CONFIG>\"")
endif()
