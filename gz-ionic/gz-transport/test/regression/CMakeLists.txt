set(TEST_TYPE "REGRESSION")

gz_get_sources(tests)

gz_build_tests(TYPE REGRESSION SOURCES ${tests}
  LIB_DEPS ${EXTRA_TEST_LIB_DEPS})
