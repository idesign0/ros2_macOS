gz_get_libsources_and_unittests(sources gtest_sources)

gz_add_component(events SOURCES ${sources} GET_TARGET_NAME events_target)

target_link_libraries(${events_target}
  PUBLIC
    gz-math${GZ_MATH_VER}::gz-math${GZ_MATH_VER})

gz_build_tests(
  TYPE UNIT
  SOURCES ${gtest_sources}
  LIB_DEPS
    ${events_target}
    gz-common${GZ_COMMON_VER}-testing
)
