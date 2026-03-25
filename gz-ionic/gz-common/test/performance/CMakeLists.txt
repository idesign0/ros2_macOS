gz_get_sources(tests)

gz_build_tests(
  TYPE PERFORMANCE
  SOURCES ${tests}
  LIB_DEPS gz-common${GZ_COMMON_VER}-testing
)
