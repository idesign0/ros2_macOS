# Collect source files into the "sources" variable and unit test files into the
# "gtest_sources" variable
gz_get_libsources_and_unittests(sources gtest_sources)

# Create the library target
gz_create_core_library(
  SOURCES ${sources}
  CXX_STANDARD ${CMAKE_CXX_STANDARD})

if (CMAKE_CXX_COMPILER_ID STREQUAL GNU)
    set(CXX_FILESYSTEM_LIBRARIES stdc++fs)
else()
    set(CXX_FILESYSTEM_LIBRARIES)
endif()

# Link the libraries that we always need
target_link_libraries(${PROJECT_LIBRARY_TARGET_NAME}
  PRIVATE
    ${CXX_FILESYSTEM_LIBRARIES}
  PUBLIC
    gz-utils${GZ_UTILS_VER}::log
    spdlog::spdlog
)

# Handle non-Windows configuration settings
if(NOT WIN32)
  # Link the libraries that we don't expect to find on Windows
  target_link_libraries(${PROJECT_LIBRARY_TARGET_NAME}
    PUBLIC UUID::UUID
    pthread)
else()
  target_link_libraries(${PROJECT_LIBRARY_TARGET_NAME}
    PRIVATE shlwapi)
endif()

# don't build MovingWindowFilter_TEST if we don't have gz-math
if(NOT gz-math${GZ_MATH_VER}_FOUND)
  list(REMOVE_ITEM gtest_sources MovingWindowFilter_TEST.cc)
endif()

# Build the unit tests
gz_build_tests(
  TYPE UNIT
  SOURCES ${gtest_sources}
  LIB_DEPS
    gz-utils${GZ_UTILS_VER}::log
    gz-common${GZ_COMMON_VER}-testing
  INCLUDE_DIRS
    # Used to make internal source file headers visible to the unit tests
    ${CMAKE_CURRENT_SOURCE_DIR})

if(TARGET UNIT_DataFrame_TEST)
  target_include_directories(UNIT_DataFrame_TEST PRIVATE
    ${gz-math${GZ_MATH_VER}_INCLUDE_DIRS})
endif()

if(TARGET UNIT_MovingWindowFilter_TEST)
  target_include_directories(UNIT_MovingWindowFilter_TEST PRIVATE
    ${gz-math${GZ_MATH_VER}_INCLUDE_DIRS})
endif()

# Produce warning on Windows if the user has decided to turn on the symlink
# tests. In order for those tests to work, they will need to run the tests in
# administrative mode, or use some other workaround.
if(WIN32)
  if(GZ_BUILD_SYMLINK_TESTS_ON_WINDOWS)
    message(STATUS "")
    message(STATUS "You have opted to enable symlink tests on a Windows platform.")
    message(STATUS "The test UNIT_Filesystem_TEST will require elevated privileges")
    message(STATUS "in order to succeed. For more information, see the issue")
    message(STATUS "https://github.com/gazebosim/gz-common/issues/21")
    message(STATUS "")
    target_compile_definitions(UNIT_Filesystem_TEST PRIVATE GZ_BUILD_SYMLINK_TESTS_ON_WINDOWS)
  endif()
endif()
