gz_get_sources(tests)

# plugin_specialization test causes lcov to hang
# see gz-cmake issue 25
if("${CMAKE_BUILD_TYPE_UPPERCASE}" STREQUAL "COVERAGE")
  list(REMOVE_ITEM tests
    plugin_specialization.cc)
endif()

gz_build_tests(
  TYPE PERFORMANCE
  SOURCES ${tests}
  LIB_DEPS
    ${PROJECT_LIBRARY_TARGET_NAME}-loader
   TEST_LIST test_targets)

foreach(test ${test_targets})
  target_compile_definitions(${test} PRIVATE
    "GzDummyPlugin_LIB=\"$<TARGET_FILE:GzDummyPlugins>\"")
endforeach()
