#.rst
# GzBuildProject
# -------------------
#
# gz_configure_build()
#
# Configures the build rules of a Gazebo library project.
#
#===============================================================================
# Copyright (C) 2017 Open Source Robotics Foundation
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

#################################################
# Configure the build of the Gazebo project
# Pass the argument HIDE_SYMBOLS_BY_DEFAULT to configure symbol visibility so
# that symbols are hidden unless explicitly marked as visible. (DEPRECATED)
# Pass the argument EXPOSE_SYMBOLS_BY_DEFAULT to configure symbol visibility
# so symbols are exposed unless explicitly marked as hidden.
# Pass the argument QUIT_IF_BUILD_ERRORS to have this macro quit cmake when the
# build_errors
macro(gz_configure_build)
  # Parse the arguments that are passed in
  set(options HIDE_SYMBOLS_BY_DEFAULT EXPOSE_SYMBOLS_BY_DEFAULT QUIT_IF_BUILD_ERRORS)
  set(oneValueArgs)
  set(multiValueArgs COMPONENTS)
  cmake_parse_arguments(gz_configure_build "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(gz_configure_build_HIDE_SYMBOLS_BY_DEFAULT)
    message(DEPRECATION "HIDE_SYMBOLS_BY_DEFAULT is now the default. You can safely remove the option.")
  endif()

  #============================================================================
  # Examine the build type. If we do not recognize the type, we will generate
  # an error, so this must come before the error handling.
  _gz_parse_build_type()

  #============================================================================
  # Ask whether we should make a shared or static library.
  option(BUILD_SHARED_LIBS "Set this to true to generate shared libraries (recommended), or false for static libraries" ON)

  #============================================================================
  # Print warnings and errors
  if(build_warnings)
    set(all_warnings " CONFIGURATION WARNINGS:")
    foreach (msg ${build_warnings})
      gz_string_append(all_warnings " -- ${msg}" DELIM "\n")
    endforeach ()
    message(WARNING "${all_warnings}")
  endif (build_warnings)

  if(build_errors)
    message(SEND_ERROR "-- BUILD ERRORS: These must be resolved before compiling.")
    foreach(msg ${build_errors})
      message(SEND_ERROR "-- ${msg}")
    endforeach()
    message(SEND_ERROR "-- END BUILD ERRORS\n")

    set(error_str "Errors encountered in build. Please see BUILD ERRORS above.")

    if(gz_configure_build_QUIT_IF_BUILD_ERRORS)
      message(FATAL_ERROR "${error_str}")
    else()
      message(WARNING "${error_str}")
    endif()

  endif()


  #============================================================================
  # If there are no build errors, try building
  if(NOT build_errors)

    #--------------------------------------
    # Turn on testing
    include(CTest)
    enable_testing()


    #--------------------------------------
    # Set up the compiler flags
    _gz_set_compiler_flags()


    #--------------------------------------
    # Set up the compiler feature flags to help us choose our standard
    _gz_set_cxx_feature_flags()


    #--------------------------------------
    # We want to include both the include directory from the source tree and
    # also the include directory that's generated in the build folder,
    # ${PROJECT_BINARY_DIR}, so that headers which are generated via cmake will
    # be visible to the compiler.
    #
    # TODO: We should consider removing this include_directories(~) command.
    # If these directories are needed by any targets, then we should specify it
    # for those targets directly.
    if(EXISTS "${PROJECT_SOURCE_DIR}/include")
      include_directories("${PROJECT_SOURCE_DIR}/include")
    endif()

    include_directories("${PROJECT_BINARY_DIR}/include")
    file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/include")


    #--------------------------------------
    # Clear the test results directory
    #
    # TODO: This should probably be in a CI script instead of our build
    # configuration script.
    execute_process(COMMAND cmake -E remove_directory ${CMAKE_BINARY_DIR}/test_results)
    execute_process(COMMAND cmake -E make_directory ${CMAKE_BINARY_DIR}/test_results)


    #--------------------------------------
    # Create the "all" meta-target
    _gz_create_all_target()


    #--------------------------------------
    # Add the source code directories of the core library
    if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/core")

      # If the directory structure has a subdirectory called "core", we will
      # use that instead of assuming that src, include, and test subdirectories
      # exist in the root directory.
      #
      # We treat "core" the same way as we treat the component subdirectories.
      # It's inserted into the beginning of the list to make sure that the core
      # subdirectory is handled before any other.
      list(INSERT gz_configure_build_COMPONENTS 0 core)

    else()

      add_subdirectory(src)
      _gz_find_include_script()

    endif()

    if(BUILD_TESTING AND EXISTS ${CMAKE_CURRENT_LIST_DIR}/test)
      add_subdirectory(test)
    endif()

    #--------------------------------------
    # Add the source, include, and test directories to the cppcheck dirs.
    # CPPCHECK_DIRS is used in GzCodeCheck. The variable specifies the
    # directories static code analyzers should check. Additional directories
    # are added for each component.
    set (CPPCHECK_DIRS)
    set (potential_cppcheck_dirs
      ${CMAKE_SOURCE_DIR}/src
      ${CMAKE_SOURCE_DIR}/include
      ${CMAKE_SOURCE_DIR}/test/integration
      ${CMAKE_SOURCE_DIR}/test/regression
      ${CMAKE_SOURCE_DIR}/test/performance)
    foreach (dir ${potential_cppcheck_dirs})
      if (EXISTS ${dir})
        list (APPEND CPPCHECK_DIRS ${dir})
      endif()
    endforeach()

    # Includes for cppcheck. This sets include paths for cppcheck. Additional
    # directories are added for each component.
    set (CPPCHECK_INCLUDE_DIRS)
    set (potential_cppcheck_include_dirs
      ${CMAKE_BINARY_DIR}
      ${CMAKE_SOURCE_DIR}/include/${PROJECT_INCLUDE_DIR}
      ${CMAKE_SOURCE_DIR}/test/integration
      ${CMAKE_SOURCE_DIR}/test/regression
      ${CMAKE_SOURCE_DIR}/test/performance)
    foreach (dir ${potential_cppcheck_include_dirs})
      if (EXISTS ${dir})
        list (APPEND CPPCHECK_INCLUDE_DIRS ${dir})
      endif()
    endforeach()

    #--------------------------------------
    # Initialize the list of header directories that should be parsed by doxygen
    if(EXISTS "${CMAKE_SOURCE_DIR}/include")
      set(gz_doxygen_component_input_dirs "${CMAKE_SOURCE_DIR}/include")
    else()
      set(gz_doxygen_component_input_dirs "")
    endif()

    #--------------------------------------
    # Add the source code directories of each component if they exist
    foreach(component ${gz_configure_build_COMPONENTS})

      if(NOT SKIP_${component} AND NOT INTERNAL_SKIP_${component})

        set(found_${component}_src FALSE)

        # Append the component's include directory to both CPPCHECK_DIRS and
        # CPPCHECK_INCLUDE_DIRS
        list(APPEND CPPCHECK_DIRS ${CMAKE_CURRENT_LIST_DIR}/${component}/include)
        list(APPEND CPPCHECK_INCLUDE_DIRS
          ${CMAKE_CURRENT_LIST_DIR}/${component}/include)

        if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/${component}/include")
          # Note: It seems we need to give the delimiter exactly this many
          # backslashes in order to get a \ plus a newline. This might be
          # dependent on the implementation of gz_string_append, so be careful
          # when changing the implementation of that function.
          gz_string_append(gz_doxygen_component_input_dirs
            "${CMAKE_CURRENT_LIST_DIR}/${component}/include"
            DELIM " \\\\\\\\\n  ")
        endif()

        # Append the component's source directory to CPPCHECK_DIRS.
        if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/${component}/src")
          list(APPEND CPPCHECK_DIRS ${CMAKE_CURRENT_LIST_DIR}/${component}/src)
        endif()

        if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/${component}/CMakeLists.txt")

          # If the component's directory has a top-level CMakeLists.txt, use
          # that.
          add_subdirectory(${component})
          set(found_${component}_src TRUE)

        else()

          # If the component's directory does not have a top-level
          # CMakeLists.txt, try to call the expected set of subdirectories
          # individually. This saves us from needing to create very redundant
          # CMakeLists.txt files that do nothing but redirect us to these
          # subdirectories.

          # Add the source files
          if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/${component}/src/CMakeLists.txt")
            add_subdirectory(${component}/src)
            set(found_${component}_src TRUE)
          endif()

          _gz_find_include_script(COMPONENT ${component})

          # Add the tests
          if(BUILD_TESTING AND
             EXISTS "${CMAKE_CURRENT_LIST_DIR}/${component}/test/CMakeLists.txt")
            add_subdirectory(${component}/test)
          endif()
        endif()

        if(NOT found_${component}_src)
          message(AUTHOR_WARNING
            "Could not find a top-level CMakeLists.txt or src/CMakeLists.txt "
            "for the component [${component}]!")
        endif()

      else()

        set(skip_msg "Skipping the component [${component}]")
        if(SKIP_${component})
          gz_string_append(skip_msg "by user request")
        elseif(${component}_MISSING_DEPS)
          gz_string_append(skip_msg "because the following packages are missing: ${${component}_MISSING_DEPS}")
        endif()

        message(STATUS "${skip_msg}")

      endif()

    endforeach()

    #--------------------------------------
    # Export the "all" meta-target
    _gz_export_target_all()

    #--------------------------------------
    # Create codecheck target
    include(GzCodeCheck)
    _gz_setup_target_for_codecheck()

    #--------------------------------------
    # If we made it this far, the configuration was successful
    message(STATUS "Build configuration successful")

  endif()

endmacro()

macro(_gz_set_cxx_feature_flags)

  set(GZ_KNOWN_CXX_STANDARDS 11 14 17)
  set(GZ_CXX_11_FEATURES cxx_std_11)
  set(GZ_CXX_14_FEATURES cxx_std_14)
  set(GZ_CXX_17_FEATURES cxx_std_17)

endmacro()

function(_gz_find_include_script)

  #------------------------------------
  # Define the expected arguments
  set(options) # Unused
  set(oneValueArgs COMPONENT)
  set(multiValueArgs) # Unused

  #------------------------------------
  # Parse the arguments
  cmake_parse_arguments(_gz_find_include_script "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  #------------------------------------
  # Set the starting point
  set(include_start "${CMAKE_CURRENT_LIST_DIR}")

  if(_gz_find_include_script_COMPONENT)
    gz_string_append(include_start ${_gz_find_include_script_COMPONENT} DELIM "/")
  endif()

  # Check each level of depth to find the first CMakeLists.txt. This allows us
  # to have custom behavior for each include directory structure while also
  # allowing us to just have one leaf CMakeLists.txt file if a project doesn't
  # need any custom configuration in its include directories.
  if(EXISTS "${include_start}/include")
    if(EXISTS "${include_start}/include/CMakeLists.txt")
      add_subdirectory("${include_start}/include")
    elseif(EXISTS "${include_start}/include/gz/CMakeLists.txt")
      add_subdirectory("${include_start}/include/gz")
    elseif(EXISTS "${include_start}/include/${PROJECT_INCLUDE_DIR}/CMakeLists.txt")
      add_subdirectory("${include_start}/include/${PROJECT_INCLUDE_DIR}")
    else()
      message(AUTHOR_WARNING
        "You have an include directory [${include_start}/include] without a "
        "CMakeLists.txt. This means its headers will not get installed!")
    endif()
  endif()

endfunction()

macro(_gz_parse_build_type)

  #============================================================================
  # If a build type is not specified, set it to RelWithDebInfo by default
  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo")
  endif()

  # Handle NONE in MSVC as blank and default to RelWithDebInfo
  if (MSVC AND CMAKE_BUILD_TYPE_UPPERCASE STREQUAL "NONE")
    set(CMAKE_BUILD_TYPE "RelWithDebInfo")
  endif()

  # Convert to uppercase in order to support arbitrary capitalization
  string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_UPPERCASE)

  #============================================================================
  # Set variables based on the build type
  set(BUILD_TYPE_PROFILE FALSE)
  set(BUILD_TYPE_RELEASE FALSE)
  set(BUILD_TYPE_RELWITHDEBINFO FALSE)
  set(BUILD_TYPE_MINSIZEREL FALSE)
  set(BUILD_TYPE_NONE FALSE)
  set(BUILD_TYPE_DEBUG FALSE)

  if("${CMAKE_BUILD_TYPE_UPPERCASE}" STREQUAL "DEBUG")
    set(BUILD_TYPE_DEBUG TRUE)
  elseif("${CMAKE_BUILD_TYPE_UPPERCASE}" STREQUAL "RELEASE")
    set(BUILD_TYPE_RELEASE TRUE)
  elseif("${CMAKE_BUILD_TYPE_UPPERCASE}" STREQUAL "RELWITHDEBINFO")
    set(BUILD_TYPE_RELWITHDEBINFO TRUE)
  elseif("${CMAKE_BUILD_TYPE_UPPERCASE}" STREQUAL "MINSIZEREL")
    set(BUILD_TYPE_MINSIZEREL TRUE)
  elseif("${CMAKE_BUILD_TYPE_UPPERCASE}" STREQUAL "NONE")
    set(BUILD_TYPE_NONE TRUE)
  elseif("${CMAKE_BUILD_TYPE_UPPERCASE}" STREQUAL "COVERAGE")
    include(GzCodeCoverage)
    set(BUILD_TYPE_DEBUG TRUE)
    gz_setup_target_for_coverage(
      OUTPUT_NAME coverage
      TARGET_NAME coverage
      TEST_RUNNER ctest)
    gz_setup_target_for_coverage(
      BRANCH_COVERAGE
      OUTPUT_NAME coverage-branch
      TARGET_NAME coverage-branch
      TEST_RUNNER ctest)
  elseif("${CMAKE_BUILD_TYPE_UPPERCASE}" STREQUAL "PROFILE")
    set(BUILD_TYPE_PROFILE TRUE)
  else()
    gz_build_error("CMAKE_BUILD_TYPE [${CMAKE_BUILD_TYPE}] unknown. Valid options are: Debug Release RelWithDebInfo MinSizeRel Profile Check")
  endif()

endmacro()
