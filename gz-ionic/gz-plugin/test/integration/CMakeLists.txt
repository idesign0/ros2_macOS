# This is to make test_config.h visible
include_directories("${CMAKE_BINARY_DIR}")

gz_get_sources(tests)

gz_build_tests(
  TYPE INTEGRATION
  SOURCES ${tests}
  LIB_DEPS
    ${PROJECT_LIBRARY_TARGET_NAME}-loader
  TEST_LIST test_targets)

foreach(test ${test_targets})

  foreach(plugin_target
      GzBadPluginAlign
      GzBadPluginAPIVersionNew
      GzBadPluginAPIVersionOld
      GzBadPluginNoInfo
      GzBadPluginSize
      GzDummyPlugins
      GzFactoryPlugins
      GzTemplatedPlugins
      GzInstanceCounter)

    target_compile_definitions(${test} PRIVATE
      "${plugin_target}_LIB=\"$<TARGET_FILE:${plugin_target}>\"")

  endforeach()

endforeach()

foreach(test
    INTEGRATION_EnablePluginFromThis_TEST
    INTEGRATION_factory
    INTEGRATION_plugin
    INTEGRATION_plugin_unload_with_nodelete
    INTEGRATION_plugin_unload_without_nodelete
    INTEGRATION_WeakPluginPtr)

  if(TARGET ${test})
    target_link_libraries(${test} ${DL_TARGET})
  endif()

endforeach()

# The linker option -force_load (for Clang) or --whole-archive (for GNU)
# or /WHOLEARCHIVE: (for MSVC) needs to be specified to ensure that the
# global structs specified in the static plugin module get loaded even
# without any explicit reference to the loaded symbols
# (only the interfaces are referenced).
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # MSVC link flag doesn't work with generator expressions
  # TODO(mjcarroll) When CMake 3.24 is genrally available, use
  # linking generator expressions as described here:
  # https://cmake.org/cmake/help/latest/manual/cmake-generator-expressions.7.html#genex:LINK_LIBRARY
  target_link_libraries(INTEGRATION_static_plugins -WHOLEARCHIVE:$<TARGET_FILE:GzDummyStaticPlugin>)
  # The whole-archive invocation doesn't correctly compute dependencies,
  # so explicitly require the plugin before the test can build.
  add_dependencies(INTEGRATION_static_plugins GzDummyStaticPlugin)
else()
  target_link_libraries(INTEGRATION_static_plugins
      $<$<CXX_COMPILER_ID:GNU>:-Wl,--whole-archive>
      $<$<CXX_COMPILER_ID:Clang>:-force_load>
      $<$<CXX_COMPILER_ID:AppleClang>:-force_load> GzDummyStaticPlugin
      $<$<CXX_COMPILER_ID:GNU>:-Wl,--no-whole-archive>)
endif()
