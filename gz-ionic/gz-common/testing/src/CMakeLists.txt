set(sources
  BazelTestPaths.cc
  CMakeTestPaths.cc
  RedirectConsoleStream.cc
  TestPaths.cc
  Utils.cc
)

set(test_sources
  AutoLogFixture_TEST.cc
  CMakeTestPaths_TEST.cc
  RedirectConsoleStream_TEST.cc
  Utils_TEST.cc
)

gz_add_component(testing SOURCES ${sources} GET_TARGET_NAME testing_target)

gz_build_tests(TYPE UNIT SOURCES ${test_sources}
  LIB_DEPS ${testing_target})
