#.rst
# GzSetCompilerFlags
# -------------------
#
# _gz_set_compiler_flags()
#
# Sets up compiler flags for a Gazebo library project
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
#
#################################################
# Set up compiler flags
# Internal to gz-cmake.
macro(_gz_set_compiler_flags)

  option(USE_GZ_RECOMMENDED_FLAGS "Build project using the compiler flags recommended by the Gazebo developers" ON)

  if(MSVC)
    _gz_setup_msvc()
  elseif(UNIX)
    _gz_setup_unix()
  endif()

  if(APPLE)
    _gz_setup_apple()
  endif()

  # Check if we are compiling with Clang with GNU-like flags and cache it
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
      set(CLANG false)
    else()
      set(CLANG true)
    endif()
  endif()

  # Check if we are compiling with GCC and cache it
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(GCC true)
  endif()

  # GCC and Clang use many of the same compilation flags, so it might be useful
  # to have a variable that indicates if one of them is being used.
  if(GCC OR CLANG)
    set(GCC_OR_CLANG true)
  endif()

  if(GCC_OR_CLANG)

    if(USE_GZ_RECOMMENDED_FLAGS)
      _gz_setup_gcc_or_clang()
    endif()

    option(USE_HOST_SSE_FLAGS "Explicitly use compiler flags to indicate the SSE version of the host machine" TRUE)
    if(USE_HOST_SSE_FLAGS)
      _gz_set_sse_flags()
    endif()

  endif()

endmacro()

#################################################
# Configure settings for Unix
# Internal to gz-cmake.
macro(_gz_setup_unix)
  # No custom setup for Unix systems
endmacro()

#################################################
# Internal to gz-cmake.
macro(_gz_setup_apple)

  # NOTE MacOSX provides different system versions than CMake is parsing.
  #      The following table lists the most recent OSX versions
  #     9.x.x = Mac OSX Leopard (10.5)
  #    10.x.x = Mac OSX Snow Leopard (10.6)
  #    11.x.x = Mac OSX Lion (10.7)
  #    12.x.x = Mac OSX Mountain Lion (10.8)
  if(${CMAKE_SYSTEM_VERSION} LESS 10)
    add_definitions(-DMAC_OS_X_VERSION=1050)
  elseif(${CMAKE_SYSTEM_VERSION} GREATER 10 AND ${CMAKE_SYSTEM_VERSION} LESS 11)
    add_definitions(-DMAC_OS_X_VERSION=1060)
  elseif(${CMAKE_SYSTEM_VERSION} GREATER 11 AND ${CMAKE_SYSTEM_VERSION} LESS 12)
    add_definitions(-DMAC_OS_X_VERSION=1070)
  elseif(${CMAKE_SYSTEM_VERSION} GREATER 12 OR ${CMAKE_SYSTEM_VERSION} EQUAL 12)
    add_definitions(-DMAC_OS_X_VERSION=1080)
    # Use libc++ on Mountain Lion (10.8)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  else()
    add_definitions(-DMAC_OS_X_VERSION=0)
  endif()

  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-undefined -Wl,dynamic_lookup")

endmacro()

#################################################
# Set up compilation flags for GCC or Clang
# Internal to gz-cmake.
macro(_gz_setup_gcc_or_clang)

  if(gz_configure_build_EXPOSE_SYMBOLS_BY_DEFAULT)
    set(CMAKE_C_VISIBILITY_PRESET "default")
    set(CMAKE_CXX_VISIBILITY_PRESET "default")
  else()
    set(CMAKE_C_VISIBILITY_PRESET "hidden")
    set(CMAKE_CXX_VISIBILITY_PRESET "hidden")
  endif()


  _gz_filter_valid_compiler_options(
    CUSTOM_ALL_FLAGS
        -Wall -Wextra -Wno-long-long -Wno-unused-value -Wfloat-equal
        -Wshadow -Winit-self -Wswitch-default -Wmissing-include-dirs -pedantic
        )

  # -ggdb3: Produce comprehensive debug information that can be utilized by gdb
  set(CUSTOM_DEBUG_FLAGS "-ggdb3")

  # We use the default flags for Release
  set(CUSTOM_RELEASE_FLAGS "")

  # We use the default flags for RelWithDebInfo
  set(CUSTOM_RELWITHDEBINFO_FLAGS "")

  # We use the default flags for MinSizeRel
  set(CUSTOM_MINSIZEREL_FLAGS "")

  # -fno-omit-frame-pointer: TODO Why do we use this?
  # -g: Produce debug information
  # -pg: Produce information that is helpful for the gprof profiling  tool
  set(CUSTOM_PROFILE_FLAGS "-fno-omit-frame-pointer -g -pg")

  # -g: Produce debug information.
  # -O0: Do absolutely no performance optimizations and reduce compilation time.
  # -Wformat=2: Print extra warnings for string formatting functions.
  # --coverage: Tell the compiler that we want our build to be instrumented for
  #             coverage analysis.
  # -fno-inline: Prevent the compiler from inlining functions. Inlined functions
  #              may confuse the coverage analysis.
  set(CUSTOM_C_COVERAGE_FLAGS "-g -O0 -Wformat=2 --coverage -fno-inline")

  set(CUSTOM_CXX_COVERAGE_FLAGS "${CUSTOM_C_COVERAGE_FLAGS}")

  # We add these flags depending on whether the compiler can support them,
  # because they cause errors when compiling with Clang.
  # -fno-elide-constructors: Prevent the compiler from eliding constructors.
  #                          Elision may confuse the coverage analysis.
  # -fno-default-inline: Prevent class members that are defined inside of their
  #                      class definition from automatically being marked as
  #                      inline functions.
  #                      ...TODO: Is this redundant with -fno-inline?
  # -fno-implicit-inline-templates: TODO: Why do we use this?
  _gz_filter_valid_compiler_options(
    CUSTOM_CXX_COVERAGE_FLAGS
        -fno-elide-constructors
        -fno-default-inline
        -fno-implicit-inline-templates)


  # NOTE We do not use the CACHE argument when appending flags to these
  # variables, because appending to the CACHE will make these variables grow
  # with redundant flags each time cmake is run. By not using CACHE, we create
  # "local" copies of each of these variables, so they will not be preserved
  # between runs of cmake. However, since these "local" variables are created in
  # the top-level scope, they will be visible to all subdirectories in our
  # filesystem, making them effectively global.

  # NOTE These flags are being specified in a very particular order. First, we
  # specify the original set of flags, then we specify the set of flags which
  # are being passed to all build types, then we specify the set of flags which
  # are specific to each build type. When contradictory flags are given to a
  # compiler, whichever flag was specified last gets precedence. Therefore, we
  # want the flags that we're passing in now to have precedence over the
  # original flags for each build type, and we want the flags that are specific
  # to each build type to have precedence over the flags that are passed to all
  # build types, to make sure that each build type can customize its flags
  # without any conflicts.


  # cmake automatically provides -g for *_FLAGS_DEBUG
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${CUSTOM_ALL_FLAGS} ${CUSTOM_DEBUG_FLAGS}")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${CUSTOM_ALL_FLAGS} ${CUSTOM_DEBUG_FLAGS}")


  # cmake automatically provides -O3 and -DNDEBUG for *_FLAGS_RELEASE
  set(CMAKE_C_FLAGS_RELEASE   "${CMAKE_C_FLAGS_RELEASE} ${CUSTOM_RELEASE_FLAGS} ${CUSTOM_ALL_FLAGS}")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${CUSTOM_ALL_FLAGS} ${CUSTOM_RELEASE_FLAGS}")


  # cmake automatically provides -g, -O2, and -DNDEBUG for *_FLAGS_RELWITHDEBINFO
  set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} ${CUSTOM_ALL_FLAGS} ${CUSTOM_RELWITHDEBINFO_FLAGS}")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${CUSTOM_ALL_FLAGS} ${CUSTOM_RELWITHDEBINFO_FLAGS}")


  # cmake automatically provides -Os and -DNDEBUG for *_FLAGS_MINSIZEREL
  set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} ${CUSTOM_ALL_FLAGS} ${CUSTOM_MINSIZEREL_FLAGS}")
  set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} ${CUSTOM_ALL_FLAGS} ${CUSTOM_MINSIZEREL_FLAGS}")


  # NONE is a custom type used in Debian, not automatically provided by cmake
  set(CMAKE_C_FLAGS_NONE "${CMAKE_C_FLAGS_NONE} ${CUSTOM_ALL_FLAGS} ${MSVC_MINIMAL_FLAGS}")
  set(CMAKE_CXX_FLAGS_NONE "${CMAKE_CXX_FLAGS_NONE} ${CUSTOM_ALL_FLAGS} ${MSVC_MINIMAL_FLAGS}")


  # PROFILE is a custom build type, not automatically provided by cmake
  set(CMAKE_C_FLAGS_PROFILE "${CMAKE_C_FLAGS_PROFILE} ${CUSTOM_ALL_FLAGS} ${CUSTOM_PROFILE_FLAGS}")
  set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_PROFILE} ${CUSTOM_ALL_FLAGS} ${CUSTOM_PROFILE_FLAGS}")


  # COVERAGE is a custom build type, not automatically provided by cmake
  set(CMAKE_C_FLAGS_COVERAGE "${CMAKE_C_FLAGS_COVERAGE} ${CUSTOM_ALL_FLAGS} ${CUSTOM_C_COVERAGE_FLAGS}")
  set(CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS_COVERAGE} ${CUSTOM_ALL_FLAGS} ${CUSTOM_CXX_COVERAGE_FLAGS}")


  # NOTE: Leave CMAKE_C_FLAGS and CMAKE_CXX_FLAGS blank, because those will
  # be appended to all build configurations.

endmacro()

#################################################
# Identify what type of Streaming SIMD Extension is being used by the system and
# then set the compiler's SSE flags appropriately.
# Internal to gz-cmake.
macro(_gz_set_sse_flags)

  message(STATUS "\n-- Searching for host SSE information")
  include(GzCheckSSE)

  if(SSE2_FOUND)
    add_compile_options(-msse -msse2)
    if (NOT APPLE)
      add_compile_options(-mfpmath=sse)
      message(STATUS "SSE2 found")
    endif()
  endif()

  if(SSE3_FOUND)
    add_compile_options(-msse3)
    message(STATUS "SSE3 found")
  endif()
  if (SSSE3_FOUND)
    add_compile_options(-mssse3)
  endif()

  if (SSE4_1_FOUND OR SSE4_2_FOUND)
    if (SSE4_1_FOUND)
      add_compile_options(-msse4.1)
      message(STATUS "SSE4.1 found")
    endif()
    if (SSE4_2_FOUND)
      add_compile_options(-msse4.2)
      message(STATUS "SSE4.2 found")
    endif()
  else()
    message(STATUS "SSE4 disabled.\n--")
  endif()

endmacro()

#################################################
# Set up compilation flags for Microsoft Visual Studio/C++
# Internal to gz-cmake.
macro(_gz_setup_msvc)

  # Reduce overhead by ignoring unnecessary Windows headers
  add_definitions(-DWIN32_LEAN_AND_MEAN)

  # Don't pull in the Windows min/max macros
  add_definitions(-DNOMINMAX)

  if(USE_GZ_RECOMMENDED_FLAGS)

    # Gy: Prevent errors caused by multiply-defined symbols
    # W2: Warning level 2: significant warnings.
    #     TODO: Recommend Wall in the future.
    #     Note: MSVC /Wall generates tons of warnings on gtest code.
    set(MSVC_MINIMAL_FLAGS "/Gy /W2")

    # Zi: Produce complete debug information
    # Note: We provide Zi to ordinary release mode because it does not impact
    # performance and can be helpful for debugging.
    set(MSVC_DEBUG_FLAGS "${MSVC_MINIMAL_FLAGS} /Zi")

    # GL: Enable Whole Program Optimization
    set(MSVC_RELEASE_FLAGS "${MSVC_DEBUG_FLAGS} /GL")

    # Use Release flags for RelWithDebInfo
    set(MSVC_RELWITHDEBINFO_FLAGS "${MSVC_RELEASE_FLAGS}")

    # INCREMENTAL:NO fix LNK4075 warning
    # LTCG: need when using /GL above
    #  see https://docs.microsoft.com/en-us/cpp/build/reference/gl-whole-program-optimization
    set(MSVC_RELWITHDEBINFO_LINKER_FLAGS "/INCREMENTAL:NO /LTCG")

    # cmake automatically provides /Zi /Ob0 /Od /RTC1
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${MSVC_DEBUG_FLAGS}")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${MSVC_DEBUG_FLAGS}")

    # cmake automatically provides /O2 /Ob2 /DNDEBUG
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${MSVC_RELEASE_FLAGS}")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${MSVC_RELEASE_FLAGS}")

    # cmake automatically provides /Zi /O2 /Ob1 /DNDEBUG
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} ${MSVC_RELWITHDEBINFO_FLAGS}")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${MSVC_RELWITHDEBINFO_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO} ${MSVC_RELWITHDEBINFO_LINKER_FLAGS}")

    # cmake automatically provides /O1 /Ob1 /DNDEBUG
    set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} ${MSVC_MINIMAL_FLAGS}")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} ${MSVC_MINIMAL_FLAGS}")

    # NOTE: Leave CMAKE_C_FLAGS and CMAKE_CXX_FLAGS blank, because
    # those will be appended to all build configurations.

    # TODO: What flags should be set for PROFILE and COVERAGE build types?
    #       Is it even possible to generate those build types on Windows?

  endif()

  # Use the dynamically loaded run-time library in Windows by default. The
  # dynamically loaded run-time allows dynamically allocated objects to be
  # passed between different DLLs, which is important for our plugin-based
  # framework.
  #
  # In some cases, a user might want to compile with the static runtime. This
  # should ONLY be done if they do not intend to use the Gazebo library as
  # part of a plugin-based framework.
  option(GZ_USE_STATIC_RUNTIME "Use the static runtime (strongly discouraged)" OFF)

  if(BUILD_SHARED_LIBS)
    # Users should not choose the static runtime unless they are compiling a
    # static library, so we completely disable this option if BUILD_SHARED_LIBS
    # is turned on.
    set(GZ_USE_STATIC_RUNTIME OFF CACHE BOOL "Use the static runtime (strongly discouraged)" FORCE)
  endif()

  if(GZ_USE_STATIC_RUNTIME)
    foreach(build_type DEBUG RELEASE RELWITHDEBINFO MINSIZEREL NONE)
      foreach(lang C CXX)
        set(flags_var CMAKE_${lang}_FLAGS_${build_type})
        string(REGEX REPLACE "/MD" "/MT" ${flags_var} "${${flags_var}}")
      endforeach()
    endforeach()
  endif()

  # We always want this flag to be specified so we get standard-compliant
  # exception handling.
  # EHsc: Use standard-compliant exception handling
  add_compile_options("/EHsc")

endmacro()
