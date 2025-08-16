cmake_minimum_required(VERSION 3.5)
project(google_benchmark_vendor CXX)

option(FORCE_BUILD_VENDOR_PKG
  "Build google benchmark from source, even if system-installed package is available"
  OFF)

if(NOT FORCE_BUILD_VENDOR_PKG)
  find_package(benchmark QUIET)
else()
  message(FATAL_ERROR "Building benchmark from source is not implemented yet")
endif()

macro(build_benchmark)
  set(extra_cmake_args)

  set(GOOGLE_BENCHMARK_TARGET_VERSION "1.5.3")

  if(DEFINED CMAKE_BUILD_TYPE)
    list(APPEND extra_cmake_args -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
  endif()

  list(APPEND extra_cmake_args "-DBENCHMARK_ENABLE_TESTING=OFF")
  list(APPEND extra_cmake_args "-DBUILD_SHARED_LIBS=ON")
  list(APPEND extra_cmake_args "-DGIT_VERSION=\"${GOOGLE_BENCHMARK_TARGET_VERSION}\"")

  if(DEFINED CMAKE_TOOLCHAIN_FILE)
    list(APPEND extra_cmake_args "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}")
    if(ANDROID)
      if(DEFINED ANDROID_ABI)
        list(APPEND extra_cmake_args "-DANDROID_ABI=${ANDROID_ABI}")
      endif()
      if(DEFINED ANDROID_CPP_FEATURES)
        list(APPEND extra_cmake_args "-DANDROID_CPP_FEATURES=${ANDROID_CPP_FEATURES}")
      endif()
      if(DEFINED ANDROID_FUNCTION_LEVEL_LINKING)
        list(APPEND extra_cmake_args "-DANDROID_FUNCTION_LEVEL_LINKING=${ANDROID_FUNCTION_LEVEL_LINKING}")
      endif()
      if(DEFINED ANDROID_NATIVE_API_LEVEL)
        list(APPEND extra_cmake_args "-DANDROID_NATIVE_API_LEVEL=${ANDROID_NATIVE_API_LEVEL}")
      endif()
      if(DEFINED ANDROID_NDK)
        list(APPEND extra_cmake_args "-DANDROID_NDK=${ANDROID_NDK}")
      endif()
      if(DEFINED ANDROID_STL)
        list(APPEND extra_cmake_args "-DANDROID_STL=${ANDROID_STL}")
      endif()
      if(DEFINED ANDROID_TOOLCHAIN_NAME)
        list(APPEND extra_cmake_args "-DANDROID_TOOLCHAIN_NAME=${ANDROID_TOOLCHAIN_NAME}")
      endif()
    endif()
  else()
    list(APPEND extra_cmake_args "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}")
  endif()
  list(APPEND extra_cmake_args "-DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}")

  include(ExternalProject)

  # externalproject_add(benchmark-${GOOGLE_BENCHMARK_TARGET_VERSION}
  #   GIT_REPOSITORY https://github.com/google/benchmark.git
  #   GIT_TAG c05843a9f622db08ad59804c190f98879b76beba  # v${GOOGLE_BENCHMARK_TARGET_VERSION}
  #   GIT_CONFIG advice.detachedHead=false
  #   # Suppress git update due to https://gitlab.kitware.com/cmake/cmake/-/issues/16419
  #   # See https://github.com/ament/uncrustify_vendor/pull/22 for details
  #   UPDATE_COMMAND ""
  #   TIMEOUT 600
  #   CMAKE_ARGS
  #     -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_install
  #     ${extra_cmake_args}
  #   PATCH_COMMAND
  #     COMMAND ${CMAKE_COMMAND} -E chdir <SOURCE_DIR> git apply -p1 --ignore-space-change --whitespace=nowarn ${CMAKE_CURRENT_SOURCE_DIR}/thread_safety_attributes.patch
  #     COMMAND ${CMAKE_COMMAND} -E chdir <SOURCE_DIR> git apply -p1 --ignore-space-change --whitespace=nowarn ${CMAKE_CURRENT_SOURCE_DIR}/0001-NFCI-Drop-warning-to-satisfy-clang-s-Wunused-but-set.patch
  # )

  # Try to find system-installed Google Benchmark
  find_package(benchmark REQUIRED)
  
endmacro()

if(NOT benchmark_FOUND OR "${benchmark_VERSION}" VERSION_LESS 1.5.3)
  build_benchmark()
elseif(benchmark_FOUND)
  # Ubuntu Focal and Jammy have a packaging bug where libbenchmark_main has no symbols,
  # causing linker failures. I'm pretty sure it shouldn't be a static library.
  # Details: https://bugs.launchpad.net/ubuntu/+source/benchmark/+bug/1887872
  get_target_property(_benchmark_main_type benchmark::benchmark_main TYPE)
  if(NOT "${_benchmark_main_type}" STREQUAL "SHARED_LIBRARY")
    build_benchmark()
  endif()
endif()

install(
  FILES package.xml
  DESTINATION share/${PROJECT_NAME}
)
