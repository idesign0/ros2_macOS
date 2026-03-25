gz_get_sources(tests)

gz_build_tests(TYPE REGRESSION SOURCES ${tests}
               ENVIRONMENT GZ_MSG_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX})
