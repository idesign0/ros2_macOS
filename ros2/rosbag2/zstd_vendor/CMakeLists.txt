cmake_minimum_required(VERSION 4.0)

project(zstd_vendor)

find_package(ament_cmake REQUIRED)

list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")

option(FORCE_BUILD_VENDOR_PKG
  "Build zstd from source, even if system-installed package is available"
  OFF)

if(NOT FORCE_BUILD_VENDOR_PKG)
  # We need at least VERSION 1.4.4, version check is done in Findzstd.cmake
  find_package(zstd 1.4.4 QUIET)
endif()

macro(build_zstd)
  set(extra_cmake_args)

  if(DEFINED CMAKE_BUILD_TYPE)
    list(APPEND extra_cmake_args -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
  endif()

  if(DEFINED CMAKE_TOOLCHAIN_FILE)
    list(APPEND extra_cmake_args "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}")
  else()
    list(APPEND extra_cmake_args "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}")
  endif()
  list(APPEND extra_cmake_args "-DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}")

  set(zstd_version 1.4.4)
  include(ExternalProject)
  # The CMakeLists.txt file for zstd is in a subdirectory.
  # We need to configure the CMake command to build from there instead.
  ExternalProject_Add(zstd-${zstd_version}
    GIT_REPOSITORY https://github.com/facebook/zstd.git
    GIT_TAG 10f0e6993f9d2f682da6d04aa2385b7d53cbb4ee  # v${zstd_version}
    GIT_CONFIG advice.detachedHead=false
    # Suppress git update due to https://gitlab.kitware.com/cmake/cmake/-/issues/16419
    # See https://github.com/ament/uncrustify_vendor/pull/22 for details
    UPDATE_COMMAND ""
    TIMEOUT 60
    SOURCE_SUBDIR build/cmake
    CMAKE_ARGS
      -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_install
      -DZSTD_BUILD_STATIC=OFF
      -DZSTD_BUILD_SHARED=ON
      -DZSTD_BUILD_PROGRAMS=OFF
      ${extra_cmake_args}
    # Note: zstd v1.4.6 will include the following fix. When that is released, upgrade and remove this patch.
    PATCH_COMMAND
      ${CMAKE_COMMAND} -E chdir <SOURCE_DIR> git apply -p1 --ignore-space-change --whitespace=nowarn
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake_minimum_required_2.8.12.patch &&
      ${CMAKE_COMMAND} -E chdir <SOURCE_DIR> git apply -p1 --ignore-space-change --whitespace=nowarn
        ${CMAKE_CURRENT_SOURCE_DIR}/no_internal_headers.patch
  )

  install(
    DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_install/
    DESTINATION ${CMAKE_INSTALL_PREFIX}
    USE_SOURCE_PERMISSIONS)
endmacro()

if (NOT zstd_FOUND)
  build_zstd()
else()
  message(STATUS "Found Zstd, skipping build.")
endif()

install(DIRECTORY cmake DESTINATION share/${PROJECT_NAME})

ament_package(CONFIG_EXTRAS zstd_vendor-extras.cmake)
