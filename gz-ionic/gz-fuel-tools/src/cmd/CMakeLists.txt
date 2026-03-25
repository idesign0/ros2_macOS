#===============================================================================
# Generate the ruby script for internal testing.
# Note that the major version of the library is included in the name.
# Ex: cmdfuel0.rb
# Unlike other gz libraries, the ruby script for the fuel_tools library is called cmdfuel.rb instead of cmdfuel_tools.rb
set(CMD_NAME cmdfuel)
set(cmd_script_generated_test "${CMAKE_BINARY_DIR}/test/lib/$<CONFIG>/ruby/gz/${CMD_NAME}${PROJECT_VERSION_MAJOR}.rb")
set(cmd_script_configured_test "${CMAKE_CURRENT_BINARY_DIR}/test_${CMD_NAME}${PROJECT_VERSION_MAJOR}.rb.configured")

# Set the library_location variable to the full path of the library file within
# the build directory.
set(library_location "$<TARGET_FILE:${PROJECT_LIBRARY_TARGET_NAME}>")

configure_file(
  "${CMD_NAME}.rb.in"
  "${cmd_script_configured_test}"
  @ONLY)

file(GENERATE
  OUTPUT "${cmd_script_generated_test}"
  INPUT  "${cmd_script_configured_test}")


#===============================================================================
# Used for the installed version.
# Generate the ruby script that gets installed.
# Note that the major version of the library is included in the name.
# Ex: cmdfuel0.rb
set(cmd_script_generated "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/${CMD_NAME}${PROJECT_VERSION_MAJOR}.rb")
set(cmd_script_configured "${CMAKE_CURRENT_BINARY_DIR}/${CMD_NAME}${PROJECT_VERSION_MAJOR}.rb.configured")

# Set the library_location variable to the relative path to the library file
# within the install directory structure.
if (MSVC)
  set(library_location_prefix "${CMAKE_INSTALL_BINDIR}")
else()
  set(library_location_prefix "${CMAKE_INSTALL_LIBDIR}")
endif()

# Set the library_location variable to the relative path to the library file
# within the install directory structure.
set(library_location "../../../${library_location_prefix}/$<TARGET_FILE_NAME:${PROJECT_LIBRARY_TARGET_NAME}>")

configure_file(
  "${CMD_NAME}.rb.in"
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
  "fuel.bash_completion.sh"
    "${CMAKE_CURRENT_BINARY_DIR}/fuel${PROJECT_VERSION_MAJOR}.bash_completion.sh" @ONLY)
install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/fuel${PROJECT_VERSION_MAJOR}.bash_completion.sh
  DESTINATION
    ${CMAKE_INSTALL_DATAROOTDIR}/gz/gz${GZ_TOOLS_VER}.completion.d)
