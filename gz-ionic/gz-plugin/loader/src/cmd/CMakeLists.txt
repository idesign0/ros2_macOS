add_library(gz STATIC gz.cc)
target_include_directories(gz PUBLIC ${CMAKE_SOURCE_DIR}/loader/include)
target_link_libraries(gz PUBLIC
  ${PROJECT_LIBRARY_TARGET_NAME}
)

set(plugin_executable gz-plugin)
add_executable(${plugin_executable} plugin_main.cc)
target_link_libraries(${plugin_executable}
  gz
  gz-utils${GZ_UTILS_VER}::cli
  ${loader}
)

set(EXE_INSTALL_DIR "${CMAKE_INSTALL_LIBEXECDIR}/gz/${GZ_DESIGNATION}${PROJECT_VERSION_MAJOR}")

install(
  TARGETS
  ${plugin_executable}
  DESTINATION
  ${EXE_INSTALL_DIR}
)

#===============================================================================
# Generate the ruby script for internal testing.
# Note that the major version of the library is included in the name.
# Ex: cmdplugin3.rb
set(cmd_script_generated_test "${CMAKE_BINARY_DIR}/test/lib/$<CONFIG>/ruby/gz/cmd${GZ_DESIGNATION}${PROJECT_VERSION_MAJOR}.rb")
set(cmd_script_configured_test "${CMAKE_CURRENT_BINARY_DIR}/test_cmd${GZ_DESIGNATION}${PROJECT_VERSION_MAJOR}.rb.configured")

# Set the library_location variable to the full path of the library file within
# the build directory.
set(plugin_exe_location "$<TARGET_FILE:${plugin_executable}>")

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
# Ex: cmdplugin3.rb
set(cmd_script_generated "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/cmd${GZ_DESIGNATION}${PROJECT_VERSION_MAJOR}.rb")
set(cmd_script_configured "${CMAKE_CURRENT_BINARY_DIR}/cmd${GZ_DESIGNATION}${PROJECT_VERSION_MAJOR}.rb.configured")

# Set the library_location variable to the relative path to the library file
# within the install directory structure.
set(plugin_exe_location "../../../${EXE_INSTALL_DIR}/$<TARGET_FILE_NAME:${plugin_executable}>")

configure_file(
  "cmd${GZ_DESIGNATION}.rb.in"
  "${cmd_script_configured}"
  @ONLY)

file(GENERATE
  OUTPUT "${cmd_script_generated}"
  INPUT  "${cmd_script_configured}")

# Install the ruby command line library in an unversioned location.
install(FILES ${cmd_script_generated} DESTINATION lib/ruby/gz)
