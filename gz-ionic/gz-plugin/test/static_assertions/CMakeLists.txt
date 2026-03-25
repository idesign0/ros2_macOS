set(static_assert_tests
  plugin_bad_const_construct
  plugin_bad_const_assignment
  plugin_no_base_class
  plugin_wrong_base_class
  )

if (UNIX)
  set(flag -Wno-unused-private-field)
  include(CheckCXXCompilerFlag)
  CHECK_CXX_COMPILER_FLAG(${flag} Rflag)
  # Make tests that try to build these files
  foreach (test ${static_assert_tests})
    add_executable(${test} ${test}.cc)
    target_link_libraries(${test} ${PROJECT_LIBRARY_TARGET_NAME}-register)
    set_target_properties(${test} PROPERTIES
      EXCLUDE_FROM_ALL TRUE
      EXCLUDE_FROM_DEFAULT_BUILD TRUE)
    if (Rflag)
      target_compile_options(${test} PRIVATE ${flag})
    endif()
    add_test(NAME STATIC_ASSERT_${test}
      COMMAND ${CMAKE_SOURCE_DIR}/test/static_assertions/testrunner.bash ${CMAKE_COMMAND} --build . --target ${test} --config $<CONFIGURATION>
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
  endforeach()

  # Tests pass if the output contains the expected error

  set_tests_properties(STATIC_ASSERT_plugin_bad_const_construct PROPERTIES
    PASS_REGULAR_EXPRESSION "The requested PluginPtr cast would discard const qualifiers")

  set_tests_properties(STATIC_ASSERT_plugin_bad_const_assignment PROPERTIES
    PASS_REGULAR_EXPRESSION "The requested PluginPtr cast would discard const qualifiers")

  set_tests_properties(STATIC_ASSERT_plugin_no_base_class PROPERTIES
    PASS_REGULAR_EXPRESSION "BUT THE INTERFACE IS NOT A BASE CLASS OF THE")

  set_tests_properties(STATIC_ASSERT_plugin_wrong_base_class PROPERTIES
    PASS_REGULAR_EXPRESSION "BUT THE INTERFACE IS NOT A BASE CLASS OF THE")
endif()
