cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)

#============================================================================
# Initialize the project
#============================================================================
project(gz-cmake4 VERSION 4.2.1)

#--------------------------------------
# Initialize the GZ_CMAKE_DIR variable with the location of the cmake
# directory that sits next to this find-module.
set(GZ_CMAKE_DIR "${CMAKE_CURRENT_LIST_DIR}/cmake")

#--------------------------------------
# Add the location of this package's cmake directory to the CMAKE_MODULE_PATH
list(APPEND CMAKE_MODULE_PATH "${GZ_CMAKE_DIR}")

#--------------------------------------
# include the master GzCMake module
include(GzCMake)

#--------------------------------------
# Set up the project
gz_configure_project(VERSION_SUFFIX)

#--------------------------------------
# Set project-specific options
option(BUILDSYSTEM_TESTING "Enable extended buildsystem testing" FALSE)

#--------------------------------------
# Install the Gazebo documentation files
# Note: This is not actually creating a doc target for gz-cmake; this is just
# installing files that are useful for generating the documentation of other
# Gazebo projects.
add_subdirectory(doc)

#--------------------------------------
# Install the benchmark files
install(DIRECTORY benchmark/
        DESTINATION ${GZ_DATA_INSTALL_DIR}/benchmark
        USE_SOURCE_PERMISSIONS)

#--------------------------------------
# Install the codecheck files
install(DIRECTORY codecheck/
        DESTINATION ${GZ_DATA_INSTALL_DIR}/codecheck
        USE_SOURCE_PERMISSIONS)

#--------------------------------------
# Install the tools files
install(DIRECTORY tools/
        DESTINATION ${GZ_DATA_INSTALL_DIR}/tools
        USE_SOURCE_PERMISSIONS)

#============================================================================
# Configure the package to be installed
#============================================================================

#--------------------------------------
# Create configuration and installation variables
set(gz_config_input  "${CMAKE_CURRENT_SOURCE_DIR}/config/gz-cmake-config.cmake.in")
set(gz_config_output "${PROJECT_NAME_LOWER}-config.cmake")
set(gz_version_output "${PROJECT_NAME_LOWER}-config-version.cmake")
set(gz_config_install_dir "${CMAKE_INSTALL_DATAROOTDIR}/cmake/${PROJECT_NAME_LOWER}")
set(gz_pkgconfig_input "${CMAKE_CURRENT_SOURCE_DIR}/config/gz-cmake.pc.in")
set(gz_pkgconfig_output "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc")
set(gz_pkgconfig_install_dir "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
set(gz_pkgconfig_abs_install_dir "${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig")
set(gz_utilities_target ${PROJECT_EXPORT_NAME}-utilities)
set(gz_utilities_import_target_name ${PROJECT_EXPORT_NAME}::${gz_utilities_target})
set(gz_utilities_target_output_filename "${gz_utilities_target}-targets.cmake")
set(simple_utilities_import_name ${PROJECT_EXPORT_NAME}::utilities)

#--------------------------------------
# Configure and install the config file
configure_package_config_file(
  ${gz_config_input}
  ${gz_config_output}
  INSTALL_DESTINATION ${gz_config_install_dir}
  PATH_VARS GZ_DATA_INSTALL_DIR
  NO_CHECK_REQUIRED_COMPONENTS_MACRO)

#--------------------------------------
# Configure and install the version file
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/${gz_version_output}
  VERSION "${PROJECT_VERSION_FULL_NO_SUFFIX}"
  COMPATIBILITY SameMajorVersion)

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${gz_config_output}
    ${CMAKE_CURRENT_BINARY_DIR}/${gz_version_output}
  DESTINATION ${gz_config_install_dir}
  COMPONENT cmake)

#--------------------------------------
# Configure and install the pkgconfig file (needed for utilities headers)
file(RELATIVE_PATH
  GZ_PC_CONFIG_RELATIVE_PATH_TO_PREFIX
  "${gz_pkgconfig_abs_install_dir}"
  "${CMAKE_INSTALL_PREFIX}"
)

# Use cmake_path(APPEND) to construct path variables used in pkg-config file.
# Since the first argument to this function must be a cmake variable (not a
# string constant, initialize the variable to the first path component.
set(GZ_PC_LIBDIR "\${prefix}")
cmake_path(APPEND GZ_PC_LIBDIR "${CMAKE_INSTALL_LIBDIR}")
set(GZ_PC_INCLUDEDIR "\${prefix}")
cmake_path(APPEND GZ_PC_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}" "${GZ_INCLUDE_INSTALL_DIR_POSTFIX}")

configure_file(${gz_pkgconfig_input} ${gz_pkgconfig_output} @ONLY)

install(
  FILES ${gz_pkgconfig_output}
  DESTINATION ${gz_pkgconfig_install_dir}
  COMPONENT pkgconfig)

#============================================================================
# Create and install the utilities component
#============================================================================
# Deprecated: Remove the utilities component in gz-cmake4
add_library(${gz_utilities_target} INTERFACE)
target_include_directories(${gz_utilities_target}
  INTERFACE
    $<INSTALL_INTERFACE:${GZ_INCLUDE_INSTALL_DIR_FULL}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>)

# Export and install the interface target
install(
  TARGETS ${gz_utilities_target}
  EXPORT ${gz_utilities_target}
  COMPONENT interface)

export(
  EXPORT ${gz_utilities_target}
  FILE ${gz_utilities_target_output_filename}
  NAMESPACE ${PROJECT_EXPORT_NAME}::)

install(
  EXPORT ${gz_utilities_target}
  DESTINATION ${gz_config_install_dir}
  FILE ${gz_utilities_target_output_filename}
  NAMESPACE ${PROJECT_EXPORT_NAME}::)

# Install the header directory
# Note: The trailing slash after "include" is necessary
install(
  DIRECTORY include/
  DESTINATION ${GZ_INCLUDE_INSTALL_DIR_FULL}
  COMPONENT headers)

#============================================================================
# Install the files for this package
#============================================================================
set(gz_modules_install_dir "${gz_config_install_dir}/cmake${PROJECT_VERSION_MAJOR}")

file(GLOB modules "cmake/*.cmake")
file(GLOB templates "cmake/*.in")

install(
  FILES ${modules} ${templates}
  DESTINATION ${gz_modules_install_dir}
  COMPONENT modules)

file(GLOB pkgconfig_templates "cmake/pkgconfig/*.in")

install(
  FILES ${pkgconfig_templates}
  DESTINATION ${gz_modules_install_dir}/pkgconfig
  COMPONENT modules)

message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")

include(CTest)
if (BUILD_TESTING)
  add_subdirectory(test)
endif()

if (BUILD_TESTING AND BUILDSYSTEM_TESTING)
  #============================================================================
  # Build examples
  #============================================================================
  # Do a fake install of gz-cmake in order to test the examples.
  # Copy or symlink the config.cmake files and cmake folder
  set(FAKE_BUILD_DIRECTORY "${CMAKE_BINARY_DIR}/fake/build")
  set(FAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/fake/install")

  file(MAKE_DIRECTORY ${FAKE_BUILD_DIRECTORY})
  file(MAKE_DIRECTORY ${FAKE_INSTALL_PREFIX})

  include(ExternalProject)
  ExternalProject_Add(
    FAKE_INSTALL

    SOURCE_DIR "${CMAKE_SOURCE_DIR}"
    # BUILD_ALWAYS needed since cmake doesn't notice when
    # example files change.
    # See alternate approach in a2113e0997c9 if this becomes too slow
    BUILD_ALWAYS 1
    CMAKE_ARGS
      "-DBUILD_TESTING=OFF"
      "-DCMAKE_INSTALL_PREFIX=${FAKE_INSTALL_PREFIX}"
  )

  add_subdirectory(examples)
endif()

# Codecheck
set(CPPCHECK_DIRS
  ${CMAKE_SOURCE_DIR}/examples
)
set(CPPCHECK_INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/examples
)
set(GZ_CMAKE_CODECHECK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/codecheck")
include(GzCodeCheck)
_gz_setup_target_for_codecheck()

# Docs
set(GZ_CMAKE_DOXYGEN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/doc/doxygen")

configure_file(${CMAKE_SOURCE_DIR}/api.md.in ${CMAKE_BINARY_DIR}/api.md)
configure_file(${CMAKE_SOURCE_DIR}/tutorials.md.in ${CMAKE_BINARY_DIR}/tutorials.md)
gz_create_docs(
  API_MAINPAGE_MD "${CMAKE_BINARY_DIR}/api.md"
  TUTORIALS_MAINPAGE_MD "${CMAKE_BINARY_DIR}/tutorials.md"
  )

# Workaround to avoid warnings when using CMAKE_BUILD_TYPE in colcon
# to build other projects and gz-cmake together on Windows
if(DEFINED CMAKE_BUILD_TYPE)
  set(_dummy_var "${CMAKE_BUILD_TYPE}")
endif()
