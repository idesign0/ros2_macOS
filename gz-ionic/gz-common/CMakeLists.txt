cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)

#============================================================================
# Initialize the project
#============================================================================
project(gz-common6 VERSION 6.3.0)
set(GZ_COMMON_VER ${PROJECT_VERSION_MAJOR})

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
#--------------------------------------
# Option: Should Windows test symlinking?
if(WIN32)
  option(
    GZ_BUILD_SYMLINK_TESTS_ON_WINDOWS
    "Creating symlinks requires special permissions on Windows, so those tests are disabled by default"
    false)
endif()

#============================================================================
# Search for project-specific dependencies
#============================================================================
message(STATUS "\n\n-- ====== Finding Dependencies ======")

#--------------------------------------
# Find gz-math
gz_find_package(gz-math8 REQUIRED_BY geospatial graphics events)
set(GZ_MATH_VER ${gz-math8_VERSION_MAJOR})

#--------------------------------------
# Find gz-utils
gz_find_package(gz-utils3 REQUIRED COMPONENTS log)
set(GZ_UTILS_VER ${gz-utils3_VERSION_MAJOR})

#--------------------------------------
# Find Tinyxml2
gz_find_package(TINYXML2 PRETTY tinyxml2
  VERSION 6.0
  REQUIRED_BY graphics
  PRIVATE_FOR graphics)

#--------------------------------------
# Find dependencies that we ignore for Visual Studio
if(NOT WIN32)
  #------------------------------------
  # Find uuid
  gz_find_package(UUID REQUIRED PRETTY uuid)
endif()

#------------------------------------
# Find Freeimage
gz_find_package(FreeImage VERSION 3.9
  REQUIRED_BY graphics
  PRIVATE_FOR graphics)

#------------------------------------
# Find GDAL
gz_find_package(GDAL VERSION 3.0
  PKGCONFIG gdal
  PKGCONFIG_VER_COMPARISON >=
  PRIVATE_FOR geospatial
  REQUIRED_BY geospatial)

#------------------------------------
# Find libswscale
gz_find_package(SWSCALE REQUIRED_BY av PRETTY libswscale)

#------------------------------------
# Find avdevice
gz_find_package(AVDEVICE VERSION 56.4.100 REQUIRED_BY av PRETTY libavdevice)
set(HAVE_AVDEVICE ${AVDEVICE_FOUND})

#------------------------------------
# Find avformat
gz_find_package(AVFORMAT REQUIRED_BY av PRETTY libavformat)

#------------------------------------
# Find avcodec
gz_find_package(AVCODEC REQUIRED_BY av PRETTY libavcodec)

#------------------------------------
# Find avutil
gz_find_package(AVUTIL REQUIRED_BY av PRETTY libavutil)

#------------------------------------
# Find assimp
gz_find_package(GzAssimp REQUIRED_BY graphics PRETTY assimp)

message(STATUS "-------------------------------------------\n")


#============================================================================
# Configure the build
#============================================================================
configure_file("${PROJECT_SOURCE_DIR}/cppcheck.suppress.in"
               ${PROJECT_BINARY_DIR}/cppcheck.suppress)

gz_configure_build(QUIT_IF_BUILD_ERRORS
  COMPONENTS av events graphics geospatial io profiler testing)

#============================================================================
# Create package information
#============================================================================
gz_create_packages()

#============================================================================
# Create documentation
#============================================================================
configure_file(${CMAKE_SOURCE_DIR}/api.md.in ${CMAKE_BINARY_DIR}/api.md)
configure_file(${CMAKE_SOURCE_DIR}/tutorials.md.in ${CMAKE_BINARY_DIR}/tutorials.md)

gz_create_docs(
  API_MAINPAGE_MD "${CMAKE_BINARY_DIR}/api.md"
  TUTORIALS_MAINPAGE_MD "${CMAKE_BINARY_DIR}/tutorials.md"
  TAGFILES
    "${GZ-MATH_DOXYGEN_TAGFILE} = ${GZ-MATH_API_URL}"
  )

#============================================================================
# Build examples
#============================================================================
if (BUILD_TESTING)
  gz_build_examples(
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/examples
    BINARY_DIR ${PROJECT_BINARY_DIR}/examples
  )
endif()
