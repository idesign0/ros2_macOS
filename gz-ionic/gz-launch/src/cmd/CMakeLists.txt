# Collect source files into the "sources" variable and unit test files into the
# "gtest_sources" variable.
gz_get_libsources_and_unittests(sources gtest_sources)

# Disable tests that need CLI if gz-tools is not found
if (MSVC OR NOT GZ_TOOLS_PROGRAM)
  list(REMOVE_ITEM gtest_sources gz_TEST.cc)
endif()

add_library(gz STATIC gz.cc)
target_include_directories(gz PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(gz PUBLIC
  ${PROJECT_LIBRARY_TARGET_NAME}
  gz-common${GZ_COMMON_VER}::gz-common${GZ_COMMON_VER}
)

set(launch_executable gz-launch)
add_executable(${launch_executable} launch_main.cc)
target_link_libraries(${launch_executable}
  gz
  gz-utils${GZ_UTILS_VER}::cli
  gz-common${GZ_COMMON_VER}::gz-common${GZ_COMMON_VER}
)

install(
  TARGETS
  ${launch_executable}
  DESTINATION
  ${GZ_LIB_INSTALL_DIR}/gz/${GZ_DESIGNATION}${PROJECT_VERSION_MAJOR}/
)

# Build the unit tests.
gz_build_tests(TYPE UNIT SOURCES ${gtest_sources}
  TEST_LIST test_list
  LIB_DEPS ${EXTRA_TEST_LIB_DEPS}
  ENVIRONMENT GZ_LAUNCH_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX})

foreach(test ${test_list})
  target_link_libraries(${test} gz)

  # Inform each test of its output directory so it knows where to call the
  # auxiliary files from. Using a generator expression here is useful for
  # multi-configuration generators, like Visual Studio.
  target_compile_definitions(${test} PRIVATE
    "DETAIL_GZ_TRANSPORT_TEST_DIR=\"$<TARGET_FILE_DIR:${test}>\""
    "GZ_TEST_LIBRARY_PATH=\"$<TARGET_FILE_DIR:${PROJECT_LIBRARY_TARGET_NAME}>\"")

endforeach()

if (TARGET UNIT_gz_TEST)
  set_tests_properties(
    UNIT_gz_TEST
    PROPERTIES
    ENVIRONMENT
      "GZ_CONFIG_PATH=${CMAKE_BINARY_DIR}/test/conf/$<CONFIG>"
  )

  # Running `gz launch` on macOS has problems when run with /usr/bin/ruby
  # due to System Integrity Protection (SIP). Try to find ruby from
  # homebrew as a workaround.
  if (APPLE)
    find_program(BREW_RUBY ruby HINTS /usr/local/opt/ruby/bin)
  endif()

  target_compile_definitions(UNIT_gz_TEST PRIVATE
      "BREW_RUBY=\"${BREW_RUBY} \"")

  target_compile_definitions(UNIT_gz_TEST PRIVATE
      "GZ_PATH=\"${GZ_TOOLS_PROGRAM}\"")
endif()

#===============================================================================
# Bash completion

# Tack version onto and install the bash completion script
configure_file(
  "launch.bash_completion.sh"
    "${CMAKE_CURRENT_BINARY_DIR}/launch${PROJECT_VERSION_MAJOR}.bash_completion.sh" @ONLY)
install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/launch${PROJECT_VERSION_MAJOR}.bash_completion.sh
  DESTINATION
    ${CMAKE_INSTALL_DATAROOTDIR}/gz/gz${GZ_TOOLS_VER}.completion.d)

#===============================================================================
# Generate the ruby script for internal testing.
# Note that the major version of the library is included in the name.
# Ex: cmdlaunch8.rb

set(cmd_script_generated_test "${CMAKE_BINARY_DIR}/test/lib/$<CONFIG>/ruby/gz/cmd${GZ_DESIGNATION}${PROJECT_VERSION_MAJOR}.rb")
set(cmd_script_configured_test "${CMAKE_CURRENT_BINARY_DIR}/test_cmd${GZ_DESIGNATION}${PROJECT_VERSION_MAJOR}.rb.configured")

# Set the library_location variable to the full path of the library file within
# the build directory.
set(launch_exe_location "$<TARGET_FILE:${launch_executable}>")

configure_file(
  "cmd${GZ_DESIGNATION}.rb.in"
  "${cmd_script_configured_test}"
  @ONLY
)

file(GENERATE
  OUTPUT "${cmd_script_generated_test}"
  INPUT  "${cmd_script_configured_test}"
)

#===============================================================================
# Used for the installed version.
# Generate the ruby script that gets installed.
# Note that the major version of the library is included in the name.
# Ex: cmdlaunch8.rb
set(cmd_script_generated "${CMAKE_CURRENT_BINARY_DIR}/cmd${GZ_DESIGNATION}${PROJECT_VERSION_MAJOR}.rb")
set(cmd_script_configured "${cmd_script_generated}.configured")

# Set the library_location variable to the relative path to the library file
# within the install directory structure.
set(launch_exe_location "../../../${CMAKE_INSTALL_LIBDIR}/gz/${GZ_DESIGNATION}${PROJECT_VERSION_MAJOR}/$<TARGET_FILE_NAME:${launch_executable}>")

configure_file(
  "cmd${GZ_DESIGNATION}.rb.in"
  "${cmd_script_configured}"
  @ONLY)

file(GENERATE
  OUTPUT "${cmd_script_generated}"
  INPUT  "${cmd_script_configured}")

set(gz_library_path "${CMAKE_BINARY_DIR}/test/lib/$<CONFIG>/ruby/gz/cmd${GZ_DESIGNATION}${PROJECT_VERSION_MAJOR}")

# Generate a configuration file for internal testing.
# Note that the major version of the library is included in the name.
# Ex: launch8.yaml
configure_file(
  "${GZ_DESIGNATION}.yaml.in"
  "${CMAKE_CURRENT_BINARY_DIR}/${GZ_DESIGNATION}${PROJECT_VERSION_MAJOR}.yaml.configured" @ONLY)

file(GENERATE
  OUTPUT "${CMAKE_BINARY_DIR}/test/conf/$<CONFIG>/${GZ_DESIGNATION}${PROJECT_VERSION_MAJOR}.yaml"
  INPUT "${CMAKE_CURRENT_BINARY_DIR}/${GZ_DESIGNATION}${PROJECT_VERSION_MAJOR}.yaml.configured")

# Used for the installed version.
set(gz_library_path "${CMAKE_INSTALL_PREFIX}/lib/ruby/gz/cmd${GZ_DESIGNATION}${PROJECT_VERSION_MAJOR}")

# Generate the configuration file that is installed.
# Note that the major version of the library is included in the name.
# Ex: launch8.yaml
configure_file(
  "${GZ_DESIGNATION}.yaml.in"
  "${CMAKE_CURRENT_BINARY_DIR}/${GZ_DESIGNATION}${PROJECT_VERSION_MAJOR}.yaml" @ONLY)

# Install the yaml configuration files in an unversioned location.
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${GZ_DESIGNATION}${PROJECT_VERSION_MAJOR}.yaml
DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/gz/)

# Install the ruby command line library in an unversioned location.
install(FILES ${cmd_script_generated} DESTINATION lib/ruby/gz)
