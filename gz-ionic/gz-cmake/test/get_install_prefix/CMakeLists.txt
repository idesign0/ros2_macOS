# For the tests, we make sure that the relative path in the build location is the same
# of the install, and we make sure that the value returned by the shared library is ${CMAKE_BINARY_DIR}
# while for the static library we make sure that the value returned is ${CMAKE_INSTALL_PREFIX}
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")

include(GzUtils)

# dladdr from dl is a compulsory requirement for
# gz_add_get_install_prefix_impl
gz_find_package(DL REQUIRED)

# shared test
add_library(get-install-prefix-test-shared SHARED)
set(PROJECT_LIBRARY_TARGET_NAME get-install-prefix-test-shared)

gz_add_get_install_prefix_impl(GET_INSTALL_PREFIX_HEADER get_install_prefix_test_shared.h
                               GET_INSTALL_PREFIX_FUNCTION gz::cmake::test::sharedlib::getInstallPrefix
                               OVERRIDE_INSTALL_PREFIX_ENV_VARIABLE GET_INSTALL_PREFIX_TEST_INSTALL_PREFIX)
set_target_properties(get-install-prefix-test-shared PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
target_include_directories(get-install-prefix-test-shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(get-install-prefix-test-shared PRIVATE cxx_std_17)


# static test
add_library(get-install-prefix-test-static STATIC)
set(PROJECT_LIBRARY_TARGET_NAME get-install-prefix-test-static)

gz_add_get_install_prefix_impl(GET_INSTALL_PREFIX_HEADER get_install_prefix_test_static.h
                               GET_INSTALL_PREFIX_FUNCTION gz::cmake::test::staticlib::getInstallPrefix
                               OVERRIDE_INSTALL_PREFIX_ENV_VARIABLE GET_INSTALL_PREFIX_TEST_INSTALL_PREFIX)
target_include_directories(get-install-prefix-test-static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(get-install-prefix-test-static PRIVATE cxx_std_17)


# Write header with CMake variables to check
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/get_install_prefix_test_cmake_variables.h
"#pragma once

#define CMAKE_INSTALL_PREFIX \"${CMAKE_INSTALL_PREFIX}\"
#define CMAKE_BINARY_DIR \"${CMAKE_BINARY_DIR}\"

")



# Add test executable
add_executable(get_install_prefix_test get_install_prefix_test.cc)
target_include_directories(get_install_prefix_test PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(get_install_prefix_test PRIVATE get-install-prefix-test-shared
                                                      get-install-prefix-test-static)
target_compile_features(get_install_prefix_test PRIVATE cxx_std_17)

# Add the test only if GZ_ENABLE_RELOCATABLE_INSTALL is enabled,
# as the test on gz_add_get_install_prefix_impl rely on GZ_ENABLE_RELOCATABLE_INSTALL
# being enabled
if(GZ_ENABLE_RELOCATABLE_INSTALL)
  add_test(NAME get_install_prefix_test COMMAND get_install_prefix_test)
endif()
