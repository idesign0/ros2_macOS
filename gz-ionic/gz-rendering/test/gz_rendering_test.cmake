#===============================================================================
# Copyright (C) 2022 Open Source Robotics Foundation
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
#
#################################################

find_package(Python3 REQUIRED COMPONENTS Interpreter)

# Determine Ubuntu version to enable/disable vulkan backend testing
if (UNIX AND NOT APPLE)
  execute_process(COMMAND lsb_release -cs
    OUTPUT_VARIABLE RELEASE_CODENAME
    RESULT_VARIABLE LSB_RESULT
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  if(NOT (${LSB_RESULT} STREQUAL "0"))
    message(WARNING "lsb_release executable not found. Disabling Ubuntu-specific tests")
  elseif (${RELEASE_CODENAME} STREQUAL "focal")
    set(UBUNTU_FOCAL 1)
  elseif (${RELEASE_CODENAME} STREQUAL "jammy")
    set(UBUNTU_JAMMY 1)
  endif()
endif()

#################################################
# gz_configure_rendering_test(<TARGET>
#                 [HEADLESS]
#                 [RENDER_ENGINE <arg>]
#                 [RENDER_ENGINE_BACKEND <arg>]
#
# Set up a rendering test to match Gazebo test conventions with additionally
# specifying engine-specific test parameters
#
# The test will be added with the name <TARGET>_<ENGINE>_<BACKEND>
# For example: UNIT_Camera_TEST_ogre2_gl3plus
#
# <TARGET>: The executable to create a test from. The same executable may be
#           used for multiple tests with differing engines/backends
# 
# [RENDER_ENGINE]: Required. Set the render engine to be used by the test (eg "ogre", "ogre2")
#
# [RENDER_ENGINE_BACKEND]: Set the render engine backend 
#                          to be used by the test (eg "metal", "vulkan")
#
# [HEADLESS]: Optional.  Enable headless rendering if the engine/backend supports it
macro(gz_configure_rendering_test)
  set(options HEADLESS)
  set(oneValueArgs TARGET RENDER_ENGINE RENDER_ENGINE_BACKEND)
  set(multiValueArgs)

  _gz_cmake_parse_arguments(gz_configure_rendering_test
    "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(NOT DEFINED gz_configure_rendering_test_RENDER_ENGINE_BACKEND)
    set(gz_configure_rendering_test_RENDER_ENGINE_BACKEND "gl3plus")
  endif()

  set(test_name ${gz_configure_rendering_test_TARGET}_${gz_configure_rendering_test_RENDER_ENGINE}_${gz_configure_rendering_test_RENDER_ENGINE_BACKEND})

  add_test(NAME ${test_name} 
    COMMAND ${gz_configure_rendering_test_TARGET} --gtest_output=xml:${CMAKE_BINARY_DIR}/test_results/${test_name}.xml)
  set_tests_properties(${test_name} PROPERTIES TIMEOUT 240)

  set_tests_properties(${test_name}
    PROPERTIES ENVIRONMENT "GZ_ENGINE_TO_TEST=${gz_configure_rendering_test_RENDER_ENGINE}")

  set_property(
      TEST ${test_name}
      APPEND PROPERTY
        ENVIRONMENT "GZ_ENGINE_BACKEND=${gz_configure_rendering_test_RENDER_ENGINE_BACKEND}")

  set_property(
      TEST ${test_name}
      APPEND PROPERTY
        ENVIRONMENT "GZ_RENDERING_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")

  if(gz_configure_rendering_test_HEADLESS)
    set_property(
        TEST ${test_name}
        APPEND PROPERTY
        ENVIRONMENT "GZ_ENGINE_HEADLESS=1")
  endif()

  if(Python3_Interpreter_FOUND)
    # Check that the test produced a result and create a failure if it didn't.
    # Guards against crashed and timed out tests.
    add_test(check_${test_name} ${Python3_EXECUTABLE} ${GZ_CMAKE_TOOLS_DIR}/check_test_ran.py
      ${CMAKE_BINARY_DIR}/test_results/${test_name}.xml)
  endif()
endmacro()

#################################################
# gz_rendering_test(<TYPE> <SOURCE>
#                 [LIB_DEPS <arg>]
#
# Set up a rendering test to match Gazebo test conventions.
# This will automatically add corresponding engine/backend tests for the platform
#
# <TYPE>: The type of test ("UNIT", "INTEGRATION") 
# 
# <SOURCE>: The source file of the test to build 
#
# [LIB_DEPS]: Additional optional library dependencies 
macro(gz_rendering_test)
  set(options)
  set(oneValueArgs TYPE SOURCE)
  set(multiValueArgs LIB_DEPS)

  _gz_cmake_parse_arguments(gz_rendering_test "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  set(TEST_NAME ${gz_rendering_test_TYPE}_${gz_rendering_test_SOURCE})

  add_executable(${TEST_NAME} ${gz_rendering_test_SOURCE}.cc)

  target_link_libraries(${TEST_NAME} PUBLIC gtest gtest_main ${gz_rendering_test_LIB_DEPS})
  if (UNIX)
    target_link_libraries(${TEST_NAME} PUBLIC pthread)
  endif()

  if (TARGET ${TEST_NAME})
    target_compile_definitions(${TEST_NAME}
      PRIVATE
      "PROJECT_SOURCE_PATH=\"${PROJECT_SOURCE_DIR}\""
      "PROJECT_BUILD_PATH=\"${PROJECT_BINARY_DIR}\""
      )
    target_include_directories(${TEST_NAME}
      PRIVATE
      ${PROJECT_SOURCE_DIR}/test/common_test
    )
  endif()

  if (GZ_RENDERING_HAVE_OGRE)
    gz_configure_rendering_test(
      TARGET ${TEST_NAME}
      RENDER_ENGINE "ogre"
    )
  endif()

  if (GZ_RENDERING_HAVE_OGRE2)
    if (APPLE)
      gz_configure_rendering_test(
        TARGET ${TEST_NAME}
        RENDER_ENGINE "ogre2"
        RENDER_ENGINE_BACKEND "metal")
    else()
      gz_configure_rendering_test(
        TARGET ${TEST_NAME}
        RENDER_ENGINE "ogre2"
        RENDER_ENGINE_BACKEND "gl3plus")
      # \TODO(mjcarroll) Re-enable when Github Actions
      # supports it
      # if(UBUNTU_JAMMY)
      #   gz_configure_rendering_test(
      #     TARGET ${TEST_NAME}
      #     RENDER_ENGINE "ogre2"
      #     RENDER_ENGINE_BACKEND "vulkan"
      #     HEADLESS)
      # endif()
    endif()
  endif()
  if (GZ_RENDERING_HAVE_OPTIX)
    gz_configure_rendering_test(
      TARGET ${TEST_NAME}
      RENDER_ENGINE "optix"
    )
  endif()
endmacro()
