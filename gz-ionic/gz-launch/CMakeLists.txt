cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)

#============================================================================
# Initialize the project
#============================================================================
project(gz-launch8 VERSION 8.0.2)

#============================================================================
# Find gz-cmake
#============================================================================
find_package(gz-cmake4 REQUIRED)
set(GZ_CMAKE_VER ${gz-cmake4_VERSION_MAJOR})

#============================================================================
# Configure the project
#============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

gz_configure_project(VERSION_SUFFIX)

#============================================================================
# Set project-specific options
#============================================================================

set(GZ_LAUNCH_PLUGIN_RELATIVE_INSTALL_PATH "${GZ_LIB_INSTALL_DIR}/gz-${GZ_DESIGNATION}-${PROJECT_VERSION_MAJOR}/plugins/")
set(GZ_LAUNCH_PLUGIN_INSTALL_PATH "${CMAKE_INSTALL_PREFIX}/${GZ_LAUNCH_PLUGIN_RELATIVE_INSTALL_PATH}")

#============================================================================
# Search for project-specific dependencies
#============================================================================
message(STATUS "\n\n-- ====== Finding Dependencies ======")

#--------------------------------------
# Find Tinyxml2
gz_find_package(TINYXML2 REQUIRED PRIVATE PRETTY tinyxml2)

#--------------------------------------
# Find gz-common
gz_find_package(gz-common6 REQUIRED PRIVATE)
set(GZ_COMMON_VER ${gz-common6_VERSION_MAJOR})

#--------------------------------------
# Find gz-utils
gz_find_package(gz-utils3 REQUIRED COMPONENTS cli)
set(GZ_UTILS_VER ${gz-utils3_VERSION_MAJOR})

#--------------------------------------
# Find gz-plugin
gz_find_package(gz-plugin3 REQUIRED_BY launch COMPONENTS loader register)
set(GZ_PLUGIN_VER ${gz-plugin3_VERSION_MAJOR})

#--------------------------------------
# Find if gz command is available
find_program(GZ_TOOLS_PROGRAM gz)
if (GZ_TOOLS_PROGRAM)
  message (STATUS "Searching for gz program - found. CLI tests can be built.")
else()
  message (STATUS "Searching for gz program - not found. CLI tests are skipped.")
endif()
set(GZ_TOOLS_VER 2)

#--------------------------------------
# Find gz-transport
gz_find_package(gz-transport14 REQUIRED)
set(GZ_TRANSPORT_VER ${gz-transport14_VERSION_MAJOR})

#--------------------------------------
# Find gz-msgs
gz_find_package(gz-msgs11 REQUIRED)
set(GZ_MSGS_VER ${gz-msgs11_VERSION_MAJOR})

#--------------------------------------
# Find gz-math
gz_find_package(gz-math8 REQUIRED)
set(GZ_MATH_VER ${gz-math8_VERSION_MAJOR})

#--------------------------------------
# Find gz-gui
gz_find_package(gz-gui9 REQUIRED)
set(GZ_GUI_VER ${gz-gui9_VERSION_MAJOR})
gz_find_package (Qt5
  COMPONENTS
    Core
    Quick
    QuickControls2
  REQUIRED
  PKGCONFIG "Qt5Core Qt5Quick Qt5QuickControls2")

#--------------------------------------
# Find gz-sim
gz_find_package(gz-sim9 REQUIRED PRIVATE COMPONENTS gui)
set(GZ_SIM_VER ${gz-sim9_VERSION_MAJOR})

gz_pkg_check_modules(websockets libwebsockets)
if (NOT websockets_FOUND)
  gz_build_warning("Unable to find libwebsockets. The websocket_server plugin will not be built.")
endif()

#--------------------------------------
# Find prerequistes for backward
include(src/vendor/BackwardConfig.cmake)

#============================================================================
# Add plugins
#============================================================================
add_subdirectory(plugins)

#============================================================================
# Configure the build
#============================================================================
gz_configure_build(QUIT_IF_BUILD_ERRORS)

#============================================================================
# install example .gzlaunch files
#============================================================================
add_subdirectory(examples)

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
   "${GZ-MATH_DOXYGEN_TAGFILE} = ${GZ-MATH_API_URL}"
   "${GZ-MSGS_DOXYGEN_TAGFILE} = ${GZ-MSGS_API_URL}"
   "${GZ-PLUGIN_DOXYGEN_TAGFILE} = ${GZ-PLUGIN_API_URL}"
   "${GZ-TRANSPORT_DOXYGEN_TAGFILE} = ${GZ-TRANSPORT_API_URL}"
   "${GZ-COMMON_DOXYGEN_TAGFILE} = ${GZ-COMMON_API_URL}"
   "${GZ-SIM_DOXYGEN_TAGFILE} = ${GZ-SIM_API_URL}"
   "${GZ-GUI_DOXYGEN_TAGFILE} = ${GZ-GUI_API_URL}"
)
