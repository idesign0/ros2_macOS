# Collect source files into the "sources" variable and unit test files into the
# "gtest_sources" variable.
gz_get_libsources_and_unittests(sources gtest_sources)

# Skip command line tests for Windows, see
# https://github.com/gazebosim/gz-transport/issues/104
if (MSVC)
  list(REMOVE_ITEM gtest_sources gz_TEST.cc)
endif()

if (NOT HAVE_GZ_TOOLS)
  list(REMOVE_ITEM gtest_sources gz_TEST.cc)
endif()

# Make a small static lib of command line functions
add_library(gz STATIC gz.cc)
target_link_libraries(gz
  ${PROJECT_LIBRARY_TARGET_NAME}
)

# Build topic CLI executable
set(topic_executable gz-transport-topic)
add_executable(${topic_executable} topic_main.cc)
target_link_libraries(${topic_executable}
  gz
  gz-utils${GZ_UTILS_VER}::cli
  ${PROJECT_LIBRARY_TARGET_NAME}
)
install(TARGETS ${topic_executable} DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}/gz/${GZ_DESIGNATION}${PROJECT_VERSION_MAJOR}/)

# Build service CLI executable
set(service_executable gz-transport-service)
add_executable(${service_executable} service_main.cc)
target_link_libraries(${service_executable}
  gz
  gz-utils${GZ_UTILS_VER}::cli
  ${PROJECT_LIBRARY_TARGET_NAME}
)
install(TARGETS ${service_executable} DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}/gz/${GZ_DESIGNATION}${PROJECT_VERSION_MAJOR}/)

# Build the unit tests.
gz_build_tests(TYPE UNIT SOURCES ${gtest_sources}
  TEST_LIST test_list
  LIB_DEPS
    gz
    gz-utils${GZ_UTILS_VER}::gz-utils${GZ_UTILS_VER}
    ${EXTRA_TEST_LIB_DEPS} test_config)

foreach(test ${test_list})
  # Inform each test of its output directory so it knows where to call the
  # auxiliary files from. Using a generator expression here is useful for
  # multi-configuration generators, like Visual Studio.
  target_compile_definitions(${test} PRIVATE
    "GZ_TEST_LIBRARY_PATH=\"$<TARGET_FILE_DIR:${PROJECT_LIBRARY_TARGET_NAME}>\""
    "PROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\""
    "TWO_PROCS_PUBLISHER_EXE=\"$<TARGET_FILE:twoProcsPublisher_aux>\""
    "TWO_PROCS_SRV_CALL_REPLIER_EXE=\"$<TARGET_FILE:twoProcsSrvCallReplier_aux>\""
    "GZ_EXE=\"${HAVE_GZ_TOOLS}\""
    )
endforeach()

if (TARGET UNIT_gz_TEST)

  set(_env_vars)
  list(APPEND _env_vars "GZ_IP=127.0.0.1")
  list(APPEND _env_vars
      "GZ_CONFIG_PATH=${CMAKE_BINARY_DIR}/test/conf/$<CONFIG>;LD_LIBRARY_PATH=\"$<TARGET_FILE_DIR:${PROJECT_LIBRARY_TARGET_NAME}>\":$ENV{LD_LIBRARY_PATH}"
  )
  set_tests_properties(
    UNIT_gz_TEST
    PROPERTIES
    ENVIRONMENT
      "${_env_vars}"
  )
endif()

if (TARGET UNIT_gz_src_TEST)
  set(_env_vars)
  list(APPEND _env_vars "GZ_IP=127.0.0.1")
  set_tests_properties(UNIT_gz_src_TEST
    PROPERTIES ENVIRONMENT "${_env_vars}"
  )
endif()

#===============================================================================
# Generate the ruby script for internal testing.
# Note that the major version of the library is included in the name.
# Ex: cmdtransport0.rb
set(cmd_script_generated_test "${CMAKE_BINARY_DIR}/test/lib/$<CONFIG>/ruby/gz/cmd${GZ_DESIGNATION}${PROJECT_VERSION_MAJOR}.rb")
set(cmd_script_configured_test "${CMAKE_CURRENT_BINARY_DIR}/test_cmd${GZ_DESIGNATION}${PROJECT_VERSION_MAJOR}.rb.configured")

# Set the library_location variable to the full path of the library file within
# the build directory.
set(service_exe_location "$<TARGET_FILE:${service_executable}>")
set(topic_exe_location "$<TARGET_FILE:${topic_executable}>")

configure_file(
  "cmd${GZ_DESIGNATION}.rb.in"
  "${cmd_script_configured_test}"
  @ONLY)

file(GENERATE
  OUTPUT "${cmd_script_generated_test}"
  INPUT  "${cmd_script_configured_test}")


#===============================================================================
# Used for the installed version.
# Generate the ruby script that gets installed.
# Note that the major version of the library is included in the name.
# Ex: cmdtransport0.rb
set(cmd_script_generated "${CMAKE_CURRENT_BINARY_DIR}/cmd${GZ_DESIGNATION}${PROJECT_VERSION_MAJOR}.rb")
set(cmd_script_configured "${cmd_script_generated}.configured")

# Set the library_location variable to the relative path to the library file
# within the install directory structure.
set(service_exe_location "../../../${CMAKE_INSTALL_LIBEXECDIR}/gz/${GZ_DESIGNATION}${PROJECT_VERSION_MAJOR}/$<TARGET_FILE_NAME:${service_executable}>")
set(topic_exe_location "../../../${CMAKE_INSTALL_LIBEXECDIR}/gz/${GZ_DESIGNATION}${PROJECT_VERSION_MAJOR}/$<TARGET_FILE_NAME:${topic_executable}>")

configure_file(
  "cmd${GZ_DESIGNATION}.rb.in"
  "${cmd_script_configured}"
  @ONLY)

file(GENERATE
  OUTPUT "${cmd_script_generated}"
  INPUT  "${cmd_script_configured}")

# Install the ruby command line library in an unversioned location.
install(FILES ${cmd_script_generated} DESTINATION lib/ruby/gz)

#===============================================================================
# Bash completion

# Tack version onto and install the bash completion script
configure_file(
  "transport.bash_completion.sh"
    "${CMAKE_CURRENT_BINARY_DIR}/transport${PROJECT_VERSION_MAJOR}.bash_completion.sh" @ONLY)
install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/transport${PROJECT_VERSION_MAJOR}.bash_completion.sh
  DESTINATION
    ${CMAKE_INSTALL_DATAROOTDIR}/gz/gz${GZ_TOOLS_VER}.completion.d)
