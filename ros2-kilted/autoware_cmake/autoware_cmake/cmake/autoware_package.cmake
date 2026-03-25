# Copyright 2022 The Autoware Contributors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

macro(autoware_package)
  # Set compile options
  if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
  endif()
  if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.24.0")
      option(CMAKE_COMPILE_WARNING_AS_ERROR "Treat compiler warnings as errors." ON)
      mark_as_advanced(CMAKE_COMPILE_WARNING_AS_ERROR)
    else()
      add_compile_options(-Werror)
    endif()
  endif()

  # Ignore PCL errors in Clang
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wno-gnu-anonymous-struct -Wno-nested-anon-types)
  endif()

  # Ignore Boost deprecated messages
  add_compile_definitions(BOOST_ALLOW_DEPRECATED_HEADERS)

  # Ignore unnecessary CMake warnings
  set(__dummy__ ${CMAKE_EXPORT_COMPILE_COMMANDS})

  # Set ROS_DISTRO macros
  set(ROS_DISTRO $ENV{ROS_DISTRO})
  if(${ROS_DISTRO} STREQUAL "rolling")
    add_compile_definitions(ROS_DISTRO_ROLLING)
  elseif(${ROS_DISTRO} STREQUAL "galactic")
    add_compile_definitions(ROS_DISTRO_GALACTIC)
  elseif(${ROS_DISTRO} STREQUAL "humble")
    add_compile_definitions(ROS_DISTRO_HUMBLE)
  elseif(${ROS_DISTRO} STREQUAL "jazzy")
    add_compile_definitions(ROS_DISTRO_JAZZY)
  endif()

  # Find dependencies
  find_package(ament_cmake_auto REQUIRED)

  # TODO(youtalk): Remove this workaround once https://github.com/autowarefoundation/autoware_universe/issues/10410 is fixed
  find_package(TinyXML2 CONFIG QUIET)
  if(NOT TinyXML2_FOUND)
    find_path(TINYXML2_INCLUDE_DIR NAMES tinyxml2.h)
    find_library(TINYXML2_LIBRARY tinyxml2)
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(TinyXML2 DEFAULT_MSG TINYXML2_LIBRARY TINYXML2_INCLUDE_DIR)
    mark_as_advanced(TINYXML2_INCLUDE_DIR TINYXML2_LIBRARY)
    if(NOT TARGET tinyxml2::tinyxml2)
      add_library(tinyxml2::tinyxml2 UNKNOWN IMPORTED)
      set_property(TARGET tinyxml2::tinyxml2 PROPERTY IMPORTED_LOCATION ${TINYXML2_LIBRARY})
      set_property(TARGET tinyxml2::tinyxml2 PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${TINYXML2_INCLUDE_DIR})
      list(APPEND TinyXML2_TARGETS tinyxml2::tinyxml2)
    endif()
  endif()

  ament_auto_find_build_dependencies()

  # Set common system includes
  include_directories(SYSTEM
    ${EIGEN3_INCLUDE_DIR}
  )

  # Workaround for lanelet2-core@1.2.1
  if(TARGET lanelet2_core::lanelet2_core)
    get_target_property(lanelet2_core_INCLUDE_DIRECTORIES lanelet2_core::lanelet2_core INTERFACE_INCLUDE_DIRECTORIES)
    include_directories(SYSTEM
      ${lanelet2_core_INCLUDE_DIRECTORIES}
    )
  endif()

  # Find test dependencies
  if(BUILD_TESTING)
    find_package(ament_lint_auto REQUIRED)
    ament_lint_auto_find_test_dependencies()
  endif()
endmacro()
