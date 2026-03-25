cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)

#============================================================================
# Initialize the project
#============================================================================
project(gz-fuel_tools10 VERSION 10.1.0)

#============================================================================
# Find gz-cmake
#============================================================================
# If you get an error at this line, you need to install gz-cmake
find_package(gz-cmake4 REQUIRED)
set(GZ_CMAKE_VER ${gz-cmake4_VERSION_MAJOR})

#============================================================================
# Configure the project
#============================================================================
gz_configure_project(VERSION_SUFFIX)

#============================================================================
# Set project-specific options
#============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#============================================================================
# Search for project-specific dependencies
#============================================================================

#--------------------------------------
# Find Tinyxml2
gz_find_package(TINYXML2 REQUIRED PRIVATE PRETTY tinyxml2)

#--------------------------------------
# Find libcurl
gz_find_package(GzCURL REQUIRED PRIVATE PRETTY curl)

#--------------------------------------
# Find jsoncpp
gz_find_package(JSONCPP REQUIRED PRIVATE)

#--------------------------------------
# Find libyaml
gz_find_package(YAML REQUIRED PRIVATE)

#--------------------------------------
# Find libzip
gz_find_package(ZIP REQUIRED PRIVATE)

#--------------------------------------
# Find gz-utils
gz_find_package(gz-utils3 REQUIRED)
set(GZ_UTILS_VER ${gz-utils3_VERSION_MAJOR})

#--------------------------------------
# Find gz-common
gz_find_package(gz-common6 REQUIRED PRIVATE COMPONENTS testing)
set(GZ_COMMON_VER ${gz-common6_VERSION_MAJOR})

#--------------------------------------
# Find gz-math
gz_find_package(gz-math8 REQUIRED PRIVATE)
set(GZ_MATH_VER ${gz-math8_VERSION_MAJOR})

#--------------------------------------
# Find gz-msgs
gz_find_package(gz-msgs11 REQUIRED PRIVATE)
set(GZ_MSGS_VER ${gz-msgs11_VERSION_MAJOR})

#--------------------------------------
# Find gz-tools
find_program(HAVE_GZ_TOOLS gz)
set (GZ_TOOLS_VER 2)

#============================================================================
# Configure the build
#============================================================================
gz_configure_build(QUIT_IF_BUILD_ERRORS)

#============================================================================
# gz command line support
#============================================================================
add_subdirectory(conf)

#============================================================================
# Create package information
#============================================================================
gz_create_packages()

#============================================================================
# Configure documentation
#============================================================================
configure_file(${CMAKE_SOURCE_DIR}/api.md.in ${CMAKE_BINARY_DIR}/api.md)
configure_file(${CMAKE_SOURCE_DIR}/tutorials.md.in ${CMAKE_BINARY_DIR}/tutorials.md)

gz_create_docs(
  API_MAINPAGE_MD "${CMAKE_BINARY_DIR}/api.md"
  TUTORIALS_MAINPAGE_MD "${CMAKE_BINARY_DIR}/tutorials.md"
  TAGFILES
   "${GZ-COMMON_DOXYGEN_TAGFILE} = ${GZ-COMMON_API_URL}"
  )
