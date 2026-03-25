gz_get_libsources_and_unittests(sources gtest_sources)
gz_add_executable(asanfail ${sources})
add_test(asan asanfail)
