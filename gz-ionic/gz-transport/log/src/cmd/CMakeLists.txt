#===============================================================================
# Generate the ruby script for internal testing.
# Note that the major version of the library is included in the name.
# TODO(anyone): We do not currently have tests for the `gz log` command, so this is currently going unused.
set(cmd_log_ruby_test_dir "${CMAKE_BINARY_DIR}/log/test/lib/ruby/gz")
set(cmd_log_script_generated_test "${cmd_log_ruby_test_dir}/cmdlog${PROJECT_VERSION_MAJOR}.rb")
set(cmd_log_script_configured_test "${cmd_log_script_generated_test}.configured")

# Set the log_library_location variable to the full path of the library file
# within the build directory
set(log_library_location "$<TARGET_FILE:${log_lib_target}>")

configure_file(
  "cmdlog.rb.in"
  "${cmd_log_script_configured_test}"
  @ONLY)

file(GENERATE
  OUTPUT "${cmd_log_script_generated_test}"
  INPUT  "${cmd_log_script_configured_test}")

# Used for internal testing.
set(gz_log_ruby_path "${cmd_log_script_generated_test}")

configure_file(
  "transportlog.yaml.in"
  "${cmd_log_ruby_test_dir}/transportlog${PROJECT_VERSION_MAJOR}.yaml")


#===============================================================================
# Used for the installed version.
# Generate the ruby script that gets installed.
# Note that the major version of the library is included in the name.
set(cmd_log_script_generated "${CMAKE_CURRENT_BINARY_DIR}/cmdlog${PROJECT_VERSION_MAJOR}.rb")
set(cmd_log_script_configured "${cmd_log_script_generated}.configured")

# Set the log_library_location variable to the relative path to the library file
# within the install directory structure.
set(log_library_location "../../../${CMAKE_INSTALL_LIBDIR}/$<TARGET_FILE_NAME:${log_lib_target}>")

configure_file(
  "cmdlog.rb.in"
  "${cmd_log_script_configured}"
  @ONLY)

file(GENERATE
  OUTPUT "${cmd_log_script_generated}"
  INPUT  "${cmd_log_script_configured}")

install(FILES ${cmd_log_script_generated} DESTINATION lib/ruby/gz)

# Used for the installed version.
set(gz_log_ruby_path "${CMAKE_INSTALL_PREFIX}/lib/ruby/gz/cmdlog${PROJECT_VERSION_MAJOR}")

set(transportlog_configured "${CMAKE_CURRENT_BINARY_DIR}/transportlog${PROJECT_VERSION_MAJOR}.yaml")
configure_file(
  "transportlog.yaml.in"
  ${transportlog_configured})

install(FILES ${transportlog_configured} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/gz/)
