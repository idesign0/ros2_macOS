# Used only for internal testing.
set(gz_library_path "${CMAKE_BINARY_DIR}/test/lib/$<CONFIG>/ruby/gz/cmd${GZ_DESIGNATION}${PROJECT_VERSION_MAJOR}")

# Generate a configuration file for internal testing.
# Note that the major version of the library is included in the name.
# Ex: plugin3.yaml
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
# Ex: plugin3.yaml
configure_file(
  "${GZ_DESIGNATION}.yaml.in"
  "${CMAKE_CURRENT_BINARY_DIR}/${GZ_DESIGNATION}${PROJECT_VERSION_MAJOR}.yaml" @ONLY)

# Install the yaml configuration files in an unversioned location.
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${GZ_DESIGNATION}${PROJECT_VERSION_MAJOR}.yaml
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/gz/)

# Tack version onto and install the bash completion script
configure_file(
  "plugin.bash_completion.sh"
    "${CMAKE_CURRENT_BINARY_DIR}/plugin${PROJECT_VERSION_MAJOR}.bash_completion.sh" @ONLY)
install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/plugin${PROJECT_VERSION_MAJOR}.bash_completion.sh
  DESTINATION
    ${CMAKE_INSTALL_DATAROOTDIR}/gz/gz${GZ_TOOLS_VER}.completion.d)
