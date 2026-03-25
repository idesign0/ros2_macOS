gz_get_libsources_and_unittests(sources gtest_sources)

gz_add_component(log
  SOURCES ${sources}
  INDEPENDENT_FROM_PROJECT_LIB
  GET_TARGET_NAME gz_utils_log_target_name)

target_link_libraries(${gz_utils_log_target_name}
  PUBLIC
    spdlog::spdlog)

gz_build_tests(TYPE UNIT
  SOURCES ${gtest_sources}
  LIB_DEPS ${gz_utils_log_target_name}
)
