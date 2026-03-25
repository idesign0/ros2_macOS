
# Collect source files into the "sources" variable and unit test files into the
# "tests" variable
gz_get_libsources_and_unittests(sources tests)

# Disable gz_TEST if gz-tools is not found
if (NOT GZ_TOOLS_PROGRAM)
  list(REMOVE_ITEM tests src/gz_TEST.cc)
endif()

# Create the library target
file(GLOB detail_sources RELATIVE "${CMAKE_CURRENT_LIST_DIR}" "src/detail/*.cc")
gz_add_component(loader
  SOURCES ${sources} ${detail_sources}
  GET_TARGET_NAME loader)

target_link_libraries(${loader}
  PRIVATE ${DL_TARGET})

gz_build_tests(
  TYPE UNIT
  SOURCES ${tests}
  LIB_DEPS
    ${loader}
    ${EXTRA_TEST_LIB_DEPS}
  TEST_LIST test_targets)

foreach(test ${test_targets})

  target_compile_definitions(${test} PRIVATE
    "GZ_PLUGIN_LIB=\"$<TARGET_FILE:${PROJECT_LIBRARY_TARGET_NAME}>\"")

  target_compile_definitions(${test} PRIVATE
    "GZ_PLUGIN_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\"")

  target_compile_definitions(${test} PRIVATE
    "GzDummyPlugins_LIB=\"$<TARGET_FILE:GzDummyPlugins>\"")

  target_compile_definitions(${test} PRIVATE
    "GZ_PATH=\"${GZ_TOOLS_PROGRAM}\"")

  target_compile_definitions(${test} PRIVATE
    "GZ_VERSION_FULL=\"${PROJECT_VERSION_FULL}\"")

endforeach()

if(TARGET UNIT_gz_TEST)
  set(_env_vars)
  list(APPEND _env_vars "GZ_CONFIG_PATH=${CMAKE_BINARY_DIR}/test/conf/$<CONFIG>")

  set_tests_properties(UNIT_gz_TEST PROPERTIES
    ENVIRONMENT "${_env_vars}")
endif()

install(
  DIRECTORY include/
  DESTINATION ${GZ_INCLUDE_INSTALL_DIR_FULL}
  PATTERN "CMakeLists.txt" EXCLUDE)

#============================================================================
# gz command line support
#============================================================================
add_subdirectory(conf)
add_subdirectory(src)
