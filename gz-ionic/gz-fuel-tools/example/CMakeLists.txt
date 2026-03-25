cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)

project(fuel-tools-examples)

# Find the Gazebo Fuel Tools library
find_package(gz-fuel_tools10 QUIET REQUIRED)
set(GZ_FUEL_TOOLS_VER ${gz-fuel_tools10_VERSION_MAJOR})

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GZ-FUEL-TOOLS_CXX_FLAGS}")

# GFlags
find_library(gflags_LIBRARIES NAMES gflags)
find_path(gflags_INCLUDE_DIRS gflags/gflags.h ENV CPATH)
if (NOT gflags_LIBRARIES OR NOT gflags_INCLUDE_DIRS)
  message (STATUS "Unable to compile 'bench.cc', missing GFLags library")
else()
  include_directories(SYSTEM ${gflags_INCLUDE_DIRS})
endif()

# Generate examples
if (EXISTS "${CMAKE_SOURCE_DIR}/list.cc")
  add_executable(list list.cc)
  target_link_libraries(list
                        gz-fuel_tools${GZ_FUEL_TOOLS_VER}::gz-fuel_tools${GZ_FUEL_TOOLS_VER}
                        ${gflags_LIBRARIES})
endif()

if (EXISTS "${CMAKE_SOURCE_DIR}/download.cc")
  add_executable(download download.cc)
  target_link_libraries(download
                        gz-fuel_tools${GZ_FUEL_TOOLS_VER}::gz-fuel_tools${GZ_FUEL_TOOLS_VER}
                        ${gflags_LIBRARIES})
endif()

if (EXISTS "${CMAKE_SOURCE_DIR}/details.cc")
  add_executable(details details.cc)
  target_link_libraries(details
                        gz-fuel_tools${GZ_FUEL_TOOLS_VER}::gz-fuel_tools${GZ_FUEL_TOOLS_VER}
                        ${gflags_LIBRARIES})
endif()

if (MSVC)

  # Don't pull in the Windows min/max macros
  add_definitions(-DNOMINMAX)

endif()
