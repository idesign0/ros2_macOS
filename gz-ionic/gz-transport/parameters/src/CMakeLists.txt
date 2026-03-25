
gz_get_libsources_and_unittests(sources gtest_sources)
list(APPEND sources cmd/ParamCommandAPI.cc)
list(APPEND gtest_sources cmd/ParamCommandAPI_TEST.cc)

gz_add_component(
  parameters SOURCES ${sources} GET_TARGET_NAME param_lib_target)

target_link_libraries(${param_lib_target}
  PUBLIC
    gz-utils${GZ_UTILS_VER}::gz-utils${GZ_UTILS_VER})

# Unit tests
gz_build_tests(
  TYPE "UNIT"
  SOURCES ${gtest_sources}
  LIB_DEPS ${param_lib_target} ${EXTRA_TEST_LIB_DEPS}
)

if(NOT WIN32)
  add_subdirectory(cmd)
endif()
