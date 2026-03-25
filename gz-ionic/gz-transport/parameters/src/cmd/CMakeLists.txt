#===============================================================================
# Generate the ruby script for internal testing.
# Note that the major version of the library is included in the name.
# TODO(anyone): We do not currently have tests for the `gz param` command, so this is currently going unused.
set(cmd_param_ruby_test_dir "${CMAKE_BINARY_DIR}/param/test/lib/ruby/gz")
set(cmd_param_script_generated_test "${cmd_param_ruby_test_dir}/cmdparam${PROJECT_VERSION_MAJOR}.rb")
set(cmd_param_script_configured_test "${cmd_param_script_generated_test}.configured")

# Set the param_library_location variable to the full path of the library file
# within the build directory
set(param_library_location "$<TARGET_FILE:${param_lib_target}>")

configure_file(
  "cmdparam.rb.in"
  "${cmd_param_script_configured_test}"
  @ONLY)

file(GENERATE
  OUTPUT "${cmd_param_script_generated_test}"
  INPUT  "${cmd_param_script_configured_test}")

# Used for internal testing.
set(gz_param_ruby_path "${cmd_param_script_generated_test}")

configure_file(
  "transportparam.yaml.in"
  "${cmd_param_ruby_test_dir}/transportparam${PROJECT_VERSION_MAJOR}.yaml")


#===============================================================================
# Used for the installed version.
# Generate the ruby script that gets installed.
# Note that the major version of the library is included in the name.
set(cmd_param_script_generated "${CMAKE_CURRENT_BINARY_DIR}/cmdparam${PROJECT_VERSION_MAJOR}.rb")
set(cmd_param_script_configured "${cmd_param_script_generated}.configured")

# Set the param_library_location variable to the relative path to the library file
# within the install directory structure.
set(param_library_location "../../../${CMAKE_INSTALL_LIBDIR}/$<TARGET_FILE_NAME:${param_lib_target}>")

configure_file(
  "cmdparam.rb.in"
  "${cmd_param_script_configured}"
  @ONLY)

file(GENERATE
  OUTPUT "${cmd_param_script_generated}"
  INPUT  "${cmd_param_script_configured}")

install(FILES ${cmd_param_script_generated} DESTINATION lib/ruby/gz)

# Used for the installed version.
set(gz_param_ruby_path "${CMAKE_INSTALL_PREFIX}/lib/ruby/gz/cmdparam${PROJECT_VERSION_MAJOR}")

set(transportparam_configured "${CMAKE_CURRENT_BINARY_DIR}/transportparam${PROJECT_VERSION_MAJOR}.yaml")
configure_file(
  "transportparam.yaml.in"
  ${transportparam_configured})

install(FILES ${transportparam_configured} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/gz/)
