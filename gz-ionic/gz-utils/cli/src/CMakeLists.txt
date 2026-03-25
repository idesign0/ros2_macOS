gz_add_component(
  cli
  INTERFACE
  INDEPENDENT_FROM_PROJECT_LIB
  GET_TARGET_NAME gz_utils_cli_target_name)

# Make sure that the name is visible also in cli/include/CMakeLists.txt
set(gz_utils_cli_target_name ${gz_utils_cli_target_name} PARENT_SCOPE)
if(NOT GZ_UTILS_VENDOR_CLI11)
  target_link_libraries(${gz_utils_cli_target_name} INTERFACE CLI11::CLI11)
endif()

gz_get_libsources_and_unittests(sources gtest_sources)
gz_build_tests(TYPE UNIT
  SOURCES ${gtest_sources}
  LIB_DEPS
    ${gz_utils_cli_target_name}
)
