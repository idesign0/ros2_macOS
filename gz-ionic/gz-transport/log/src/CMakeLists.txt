gz_get_libsources_and_unittests(sources gtest_sources)
list(APPEND sources cmd/LogCommandAPI.cc)

gz_add_component(log SOURCES ${sources} GET_TARGET_NAME log_lib_target)

target_link_libraries(${log_lib_target}
  PRIVATE SQLite3::SQLite3)

if (MSVC)
  # Warning #4251 is the "dll-interface" warning that tells you when types used
  # by a class are not being exported. These generated source files have private
  # members that don't get exported, so they trigger this warning. However, the
  # warning is not important since those members do not need to be interfaced
  # with.
  set_source_files_properties(${sources} ${gtest_sources} COMPILE_FLAGS "/wd4251 /wd4146")
endif()

# Unit tests
gz_build_tests(
  TYPE "UNIT"
  SOURCES ${gtest_sources}
  LIB_DEPS ${log_lib_target} ${EXTRA_TEST_LIB_DEPS} test_config
  TEST_LIST logging_tests
)

foreach(test_target ${logging_tests})
  set_tests_properties(${test_target} PROPERTIES
    ENVIRONMENT GZ_TRANSPORT_LOG_SQL_PATH=${PROJECT_SOURCE_DIR}/log/sql)
  target_compile_definitions(${test_target} PRIVATE
    "CORRUPT_DB_TEST_PATH=\"${CMAKE_SOURCE_DIR}/log/test/data/state.tlog\""
  )

endforeach()

if(MSVC)
  # The Recorder class uses Discovery, which is a header-only class
  # that makes calls to the native Windows socket library when compiled
  # on Windows. The easiest way to link to the correct socket library is
  # to link to the ZeroMQ target and then get the socket library
  # dependency transitively.
  target_link_libraries(${log_lib_target} PRIVATE
    ${ZeroMQ_TARGET})
endif()

# Set install instructions for the sql schema, and configure the build to use it
set(SCHEMA_INSTALL_BASE ${GZ_DATA_INSTALL_DIR})
install(DIRECTORY ../sql DESTINATION ${SCHEMA_INSTALL_BASE})
set(SCHEMA_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/${SCHEMA_INSTALL_BASE}/sql)
configure_file(build_config.hh.in build_config.hh @ONLY)

target_include_directories(${log_lib_target}
  PUBLIC
    # Add this component's include directory to the build interface include
    # directories so the headers are visible while building.
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>"
  PRIVATE
    # Add the current binary directory as a private include directory while building
    # the logging library. This allows the logging library to see build_config.hh
    # while being built.
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>")

if(NOT WIN32)
  add_subdirectory(cmd)
endif()
