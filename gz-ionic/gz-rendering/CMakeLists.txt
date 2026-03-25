cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)

#============================================================================
# Initialize the project
#============================================================================
project(gz-rendering9 VERSION 9.5.0)

#============================================================================
# Find gz-cmake
#============================================================================
# If you get an error at this line, you need to install gz-cmake
find_package(gz-cmake4 REQUIRED)
set(GZ_CMAKE_VER ${gz-cmake4_VERSION_MAJOR})

#============================================================================
# Set up the project
#============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

gz_configure_project(VERSION_SUFFIX)

#============================================================================
# Set project-specific options
#============================================================================
option(USE_UNOFFICIAL_OGRE_VERSIONS "Accept unsupported Ogre versions in the build" OFF)
option(USE_OGRE2_OVERLAY "Enable OGRE2 Overlay support" ON)

#============================================================================
# Search for project-specific dependencies
#============================================================================

#--------------------------------------
# Find gz-math
gz_find_package(gz-math8 REQUIRED COMPONENTS eigen3)
set(GZ_MATH_VER ${gz-math8_VERSION_MAJOR})

#--------------------------------------
# Find gz-common
gz_find_package(gz-common6 REQUIRED
  COMPONENTS graphics events geospatial)
set(GZ_COMMON_VER ${gz-common6_VERSION_MAJOR})

#--------------------------------------
# Find gz-plugin
gz_find_package(gz-plugin3 REQUIRED COMPONENTS all)
set(GZ_PLUGIN_VER ${gz-plugin3_VERSION_MAJOR})

#--------------------------------------
# Find gz-utils
gz_find_package(gz-utils3 REQUIRED)
set(GZ_UTILS_VER ${gz-utils3_VERSION_MAJOR})

#--------------------------------------
# Find OpenGL
if (APPLE)
  gz_find_package(OpenGL
    REQUIRED_BY ogre ogre2
    PKGCONFIG gl)
else()
  gz_find_package(OpenGL REQUIRED
    COMPONENTS OpenGL
    OPTIONAL_COMPONENTS EGL
    REQUIRED_BY ogre ogre2
    PKGCONFIG gl)
endif()

if (OpenGL_FOUND)
  set(HAVE_OPENGL TRUE)
endif()

#--------------------------------------
# Find OGRE
list(APPEND gz_ogre_components "RTShaderSystem" "Terrain" "Overlay" "Paging")

gz_find_package(GzOGRE VERSION 1.9.0
  COMPONENTS ${gz_ogre_components}
  REQUIRED_BY ogre
  PRIVATE_FOR ogre)

# Ogre versions greater than 1.9.x are not officially supported.
# Display a warning for the users on this setup unless they provide
# USE_UNOFFICIAL_OGRE_VERSIONS flag
if (NOT USE_UNOFFICIAL_OGRE_VERSIONS)
  if (OGRE_VERSION VERSION_GREATER_EQUAL 1.10.0)
    GZ_BUILD_WARNING("Ogre 1.x versions greater than 1.9 are not officially supported."
                     "The software might compile and even work but support from upstream"
                     "could be reduced to accepting patches for newer versions")
  endif()
endif()

if (OGRE_FOUND)
  # find boost - mainly needed on macOS and also by the terrain component
  find_package(Boost)
  if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
  endif()

  set(GZ_RENDERING_HAVE_OGRE TRUE)
endif()

#--------------------------------------
# Find OGRE2: first try to find OGRE2 built with PlanarReflections support and
# fallback to look for OGRE2 without it. Both seems to works for gz-rendering.
# See https://github.com/gazebosim/gz-rendering/issues/597

# Build the list of OGRE2 components based on the options
set(OGRE2_COMPONENTS HlmsPbs HlmsUnlit PlanarReflections)
if(USE_OGRE2_OVERLAY)
  list(APPEND OGRE2_COMPONENTS Overlay)
  set(GZ_RENDERING_HAVE_OGRE2_OVERLAY TRUE)
endif()

gz_find_package(GzOGRE2 VERSION 2.3.1
    COMPONENTS ${OGRE2_COMPONENTS}
    PRIVATE_FOR ogre2
    QUIET)

if ("${OGRE2-PlanarReflections}" STREQUAL "OGRE2-PlanarReflections-NOTFOUND")
  message(STATUS "PlanarReflections component was not found. Try looking without it:")
  gz_find_package(GzOGRE2 VERSION 2.3.1
    COMPONENTS ${OGRE2_COMPONENTS}
    REQUIRED_BY ogre2
    PRIVATE_FOR ogre2)
endif()

if (OGRE2_FOUND)
  set(GZ_RENDERING_HAVE_OGRE2 TRUE)

  if(NOT USE_OGRE2_OVERLAY)
    message(STATUS "OGRE2 Overlay support disabled by configuration")
  endif()
endif()

#--------------------------------------
# Find Vulkan
# ogre-next 2.3 is built with vulkan support on Jammy
# Make sure we have vulkan headers, e.g. vulkan/vulkan_core.h
if (UNIX AND NOT APPLE)
  # check if ogre-next vulkan headers are installed
  find_path(ogre_vulkan_INCLUDE "RenderSystems/Vulkan/OgreVulkanDevice.h"
    HINTS ${OGRE2_INCLUDE_DIRS})

  # if not installed, ogre-next is likely built on platform without vulkan
  # support, e.g. Focal
  if (NOT ogre_vulkan_INCLUDE)
    message(STATUS "Skipping vulkan support for component [ogre2]")
  else()
    find_path(vulkan_INCLUDE vulkan/vulkan_core.h)
    if(NOT vulkan_INCLUDE)
      message(STATUS "Looking for vulkan header (vulkan/vulkan_core.h) - not found")
      gz_build_warning("Skipping component [ogre2]: Missing vulkan headers.\n")
      # Create a variable to indicate that we need to skip the component
      set(INTERNAL_SKIP_ogre2 true)
      set(GZ_RENDERING_HAVE_OGRE2 FALSE)
    else()
      message(STATUS "Looking for vulkan header (vulkan/vulkan_core.h) - found")
      set(GZ_RENDERING_HAVE_VULKAN TRUE)
    endif()
  endif()
endif()

# Plugin install dirs
set(GZ_RENDERING_ENGINE_RELATIVE_INSTALL_DIR
  ${GZ_LIB_INSTALL_DIR}/gz-${GZ_DESIGNATION}-${PROJECT_VERSION_MAJOR}/engine-plugins
)
set(GZ_RENDERING_ENGINE_INSTALL_DIR
  ${CMAKE_INSTALL_PREFIX}/${GZ_RENDERING_ENGINE_RELATIVE_INSTALL_DIR})

#--------------------------------------
# Find DL if doing relocatable installation
if (GZ_ENABLE_RELOCATABLE_INSTALL)
  gz_find_package(DL REQUIRED)
endif()

#--------------------------------------
# Find dependencies that we ignore for Visual Studio
if(NOT MSVC)
  #--------------------------------------
  # Find FreeImage
  gz_find_package(FreeImage VERSION 3.9
    REQUIRED_BY optix
    PRIVATE_FOR optix)

  #--------------------------------------
  # Find CUDA
  # Module is being removed in CMake and needs a non trivial
  # migration https://cmake.org/cmake/help/latest/policy/CMP0146.html
  if(POLICY CMP0146)
    cmake_policy(SET CMP0146 OLD)
  endif()
  find_package(CUDA QUIET)

  #--------------------------------------
  # Find OptiX
  gz_find_package(OptiX QUIET VERSION 3.8.0)

  if (OptiX_FOUND AND CUDA_FOUND)
    set(GZ_RENDERING_HAVE_OPTIX TRUE)
  endif()
endif()

#####################################
# Define compile-time default variables
if(MSVC)
  set(GZ_RENDERING_PLUGIN_PATH ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR})
else()
  set(GZ_RENDERING_PLUGIN_PATH ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
endif()

set(GZ_RENDERING_RELATIVE_RESOURCE_PATH ${GZ_DATA_INSTALL_DIR})
set(GZ_RENDERING_RESOURCE_PATH ${CMAKE_INSTALL_PREFIX}/${GZ_RENDERING_RELATIVE_RESOURCE_PATH})

#============================================================================
# Configure the build
#============================================================================
if (GZ_RENDERING_HAVE_OGRE)
  list(APPEND RENDERING_COMPONENTS ogre)
endif()

if (GZ_RENDERING_HAVE_OPTIX)
  list(APPEND RENDERING_COMPONENTS optix)
endif()

if (GZ_RENDERING_HAVE_OGRE2)
  list(APPEND RENDERING_COMPONENTS ogre2)
endif()

configure_file("${PROJECT_SOURCE_DIR}/cppcheck.suppress.in"
               ${PROJECT_BINARY_DIR}/cppcheck.suppress)

gz_configure_build(QUIT_IF_BUILD_ERRORS
    COMPONENTS ${RENDERING_COMPONENTS})

if (GZ_RENDERING_HAVE_OGRE2)
  # Must be done after gz_configure_build or else Terra
  # won't see GZ_ADD_fPIC_TO_LIBRARIES
  add_subdirectory(ogre2/src/terrain/Terra)
endif()

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
  IMAGE_PATH_DIRS "${CMAKE_SOURCE_DIR}/tutorials/img"
  TAGFILES
    "${GZ-COMMON_DOXYGEN_TAGFILE} = ${GZ-COMMON_API_URL}"
    "${GZ-MATH_DOXYGEN_TAGFILE} = ${GZ-MATH_API_URL}"
)

file(COPY ${CMAKE_SOURCE_DIR}/tutorials/img/ DESTINATION ${CMAKE_BINARY_DIR}/doxygen/html/img/)
