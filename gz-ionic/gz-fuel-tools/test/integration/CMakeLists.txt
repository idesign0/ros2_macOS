set(TEST_TYPE "INTEGRATION")

set(tests
  zip.cc
)

include_directories(SYSTEM ${CMAKE_BINARY_DIR}/test/)
link_directories(${PROJECT_BINARY_DIR}/test)

gz_build_tests(TYPE INTEGRATION
                SOURCES ${tests}
                LIB_DEPS gz-common${GZ_COMMON_VER}::gz-common${GZ_COMMON_VER}
)
