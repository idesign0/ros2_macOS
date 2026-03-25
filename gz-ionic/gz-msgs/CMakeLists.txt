cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)

#============================================================================
# Initialize the project
#============================================================================
project(gz-msgs11 VERSION 11.1.0)

#============================================================================
# Find gz-cmake
#============================================================================
# If you get an error at this line, you need to install gz-cmake
find_package(gz-cmake4 REQUIRED)

#============================================================================
# Configure the project
#============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

gz_configure_project(VERSION_SUFFIX
  CONFIG_EXTRAS "gz-msgs-extras.cmake.in")

#============================================================================
# Set project-specific options
#============================================================================

# Cross-compilation related options
# In a cross-compilation scenario, it is possible that the
# ${PROJECT_NAME}_protoc_plugin
# generator compiled for the target machine cannot be used to generate
# the C++ code corresponding to the .proto definition.
# Similarly, it is possible that Python3::Interpreter and protobuf::protoc
# found via find_package are not executable that can be run during the build.
# To avoid, that the following options can be used as follows.
# First of all, gz-msgs is compiled targeting the host machine, and in the
# build targeting the host, ensuring that the ${PROJECT_NAME}_protoc_plugin
# is installed in <host_install_prefix>/bin/${PROJECT_NAME}_protoc_plugin .
# Then, the same version of gz-msgs
# can be cross-compiled, and in the cross-compilation build the location of the
# host gz_msgs_gen is specified via the GZ_MSGS_GEN_EXECUTABLE CMake option.
# Similarly to gz-msgs, also a copy of protoc and of the Python interpreter need
# to be installed in the host at <host_install_prefix>/bin, and they can be passed
# to the build appropriate CMake options:
# > cmake -Dgz-msgs<MajVer>_PROTO_GENERATOR_PLUGIN=gz-msgs<MajVer>_protoc_plugin
# >       -Dgz-msgs<MajVer>_PROTOC_EXECUTABLE=<host_install_prefix>/bin/protoc
# >       -Dgz-msgs<MajVer>_PYTHON_INTERPRETER=<host_install_prefix>/bin/python
# >        ..
# In case the gz-msgs CMake functions are used also in downstream projects,
# the same variables can be passed when configuring the downsream projects.

set(
  ${PROJECT_NAME}_PROTO_GENERATOR_PLUGIN
  "${PROJECT_NAME}_protoc_plugin"
  CACHE STRING
  "gz_msgs_gen executable used in the gz-msgs CMake functions.")
mark_as_advanced(${PROJECT_NAME}_PROTO_GENERATOR_PLUGIN)

set(
  ${PROJECT_NAME}_PROTOC_EXECUTABLE
  protobuf::protoc
  CACHE STRING
  "protoc target or executable used in the gz-msgs CMake functions.")
mark_as_advanced(${PROJECT_NAME}_PROTOC_EXECUTABLE)

set(
  ${PROJECT_NAME}_PYTHON_INTERPRETER
  Python3::Interpreter
  CACHE STRING
  "python target or executable used in the gz-msgs CMake functions.")
mark_as_advanced(${PROJECT_NAME}_PYTHON_INTERPRETER)

# Python interfaces vars
option(USE_SYSTEM_PATHS_FOR_PYTHON_INSTALLATION
      "Install python modules in standard system paths in the system"
      OFF)

option(USE_DIST_PACKAGES_FOR_PYTHON
      "Use dist-packages instead of site-package to install python modules"
      OFF)

#============================================================================
# Search for project-specific dependencies
#============================================================================
message(STATUS "\n\n-- ====== Finding Dependencies ======")

#--------------------------------------
# Find Protobuf
gz_find_package(GzProtobuf
                 REQUIRED
                 COMPONENTS all
                 PRETTY Protobuf)

#--------------------------------------
# Find gz-utils
gz_find_package(gz-utils3 REQUIRED COMPONENTS cli)
set(GZ_UTILS_VER ${gz-utils3_VERSION_MAJOR})

#--------------------------------------
# Find gz-math
gz_find_package(gz-math8 REQUIRED)
set(GZ_MATH_VER ${gz-math8_VERSION_MAJOR})

#--------------------------------------
# Find if command is available. This is used to enable tests.
# Note that CLI files are installed regardless of whether the dependency is
# available during build time
find_program(HAVE_GZ_TOOLS gz)
set(GZ_TOOLS_VER 2)

#--------------------------------------
# Find Tinyxml2
gz_find_package(TINYXML2 REQUIRED PRETTY tinyxml2)

#--------------------------------------
# Find DL if doing relocatable installation
if (GZ_ENABLE_RELOCATABLE_INSTALL)
  gz_find_package(DL REQUIRED)
endif()

#--------------------------------------
# Find Python
find_package(Python3 REQUIRED COMPONENTS Interpreter)

if(NOT GZ_PYTHON_INSTALL_PATH)
  if(USE_SYSTEM_PATHS_FOR_PYTHON_INSTALLATION)
    if(USE_DIST_PACKAGES_FOR_PYTHON)
      string(REPLACE "site-packages" "dist-packages" GZ_PYTHON_INSTALL_PATH ${Python3_SITELIB})
    else()
      # Python3_SITELIB might use dist-packages in some platforms
      string(REPLACE "dist-packages" "site-packages" GZ_PYTHON_INSTALL_PATH ${Python3_SITELIB})
    endif()
  else()
    # If not a system installation, respect local paths
    set(GZ_PYTHON_INSTALL_PATH ${GZ_LIB_INSTALL_DIR}/python)
  endif()
endif()
#============================================================================
# Configure the build
#============================================================================
gz_configure_build(QUIT_IF_BUILD_ERRORS)

#============================================================================
# gz command line support
#============================================================================
add_subdirectory(conf)

#============================================================================
# Test the command line commands
#============================================================================
add_subdirectory(tools)

#============================================================================
# Install proto files
#============================================================================
add_subdirectory(proto)

# Generate python
add_subdirectory(python)

#============================================================================
# Create package information
#============================================================================
gz_create_packages()

#============================================================================
# Install cmake extras for downstream users
#============================================================================
# Necessary to populate generator expressions
file(
  GENERATE
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/gz-cmake/gz-msgs-extras.cmake"
  INPUT "${CMAKE_CURRENT_BINARY_DIR}/gz-cmake/gz-msgs-extras.cmake"
)

# Install cmake support files
install(
  DIRECTORY cmake/
  DESTINATION "${PROJECT_CMAKE_EXTRAS_RELATIVE_INSTALL_DIR}"
)

#============================================================================
# Create documentation
#============================================================================
configure_file(${CMAKE_SOURCE_DIR}/api.md.in ${CMAKE_BINARY_DIR}/api.md)
configure_file(${CMAKE_SOURCE_DIR}/tutorials.md.in ${CMAKE_BINARY_DIR}/tutorials.md)

gz_create_docs(
  API_MAINPAGE_MD "${CMAKE_BINARY_DIR}/api.md"
  TUTORIALS_MAINPAGE_MD "${CMAKE_BINARY_DIR}/tutorials.md"
  IMAGE_PATH_DIRS "${CMAKE_SOURCE_DIR}/tutorials/files"
  TAGFILES
    "${GZ-MATH_DOXYGEN_TAGFILE} = ${GZ-MATH_API_URL}"
)

# Wait to build the doc for the autogenerated code
# The TARGET will exist if doxygen is installed
if(TARGET doc)
  add_dependencies(doc gz-msgs${PROJECT_VERSION_MAJOR})
endif()
