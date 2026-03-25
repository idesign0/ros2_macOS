# Collect source files into the "sources" variable and unit test files into the
# "gtest_sources" variable.
gz_get_libsources_and_unittests(sources gtest_sources)

# Create the library target.
gz_create_core_library(SOURCES ${sources} CXX_STANDARD 17)

# Link the libraries that we always need.
target_link_libraries(${PROJECT_LIBRARY_TARGET_NAME}
  PUBLIC
    gz-utils${GZ_UTILS_VER}::gz-utils${GZ_UTILS_VER}
    gz-msgs${GZ_MSGS_VER}::gz-msgs${GZ_MSGS_VER}
    CPPZMQ::CPPZMQ
  PRIVATE
    ${ZeroMQ_TARGET}
)

target_include_directories(${PROJECT_LIBRARY_TARGET_NAME}
  SYSTEM PUBLIC
    $<TARGET_PROPERTY:protobuf::libprotobuf,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:CPPZMQ::CPPZMQ,INTERFACE_INCLUDE_DIRECTORIES>)

# Windows system library provides UUID
if (NOT MSVC)
  target_link_libraries(${PROJECT_LIBRARY_TARGET_NAME}
    PUBLIC
      UUID::UUID
  )
endif()

# Build the unit tests.
gz_build_tests(TYPE UNIT SOURCES ${gtest_sources}
  TEST_LIST test_list
  LIB_DEPS ${EXTRA_TEST_LIB_DEPS} test_config)

foreach(test ${test_list})
  set(_env_vars)
  list(APPEND _env_vars "GZ_IP=127.0.0.1")
  list(APPEND _env_vars "GZ_VERBOSE=1")
  set_property(TEST ${test} PROPERTY ENVIRONMENT "${_env_vars}")
endforeach()

if(MSVC)
  # On Windows, UNIT_Discovery_TEST uses some socket functions and therefore
  # needs to link to the Windows socket library. An easy, maintainable way to
  # do that is to tell it to link to the ZeroMQ target, and let it get the
  # socket library dependency transitively.
  if(TARGET UNIT_Discovery_TEST)
    target_link_libraries(UNIT_Discovery_TEST
      ${ZeroMQ_TARGET})
  endif()
endif()

# Command line support.
add_subdirectory(cmd)
