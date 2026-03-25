gz_get_libsources_and_unittests(sources gtest_sources)

gz_add_component(io SOURCES ${sources} GET_TARGET_NAME io_target)

target_link_libraries(${io_target}
  PUBLIC
    gz-math${GZ_MATH_VER}::gz-math${GZ_MATH_VER})

gz_build_tests(
  TYPE UNIT
  SOURCES ${gtest_sources}
  LIB_DEPS
    ${io_target}
    gz-common${GZ_COMMON_VER}-testing
)
