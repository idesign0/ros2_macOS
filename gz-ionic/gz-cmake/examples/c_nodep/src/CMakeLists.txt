gz_get_libsources_and_unittests(sources gtest_sources)
gz_create_core_library(SOURCES ${sources} CXX_STANDARD 11)
gz_build_tests(TYPE UNIT SOURCES ${gtest_sources})