set(TEST_TYPE "REGRESSION")

set(tests
)

link_directories(${PROJECT_BINARY_DIR}/test)

gz_build_tests(TYPE REGRESSION
               SOURCES ${tests}
               ENVIRONMENT GZ_LAUNCH_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX})
