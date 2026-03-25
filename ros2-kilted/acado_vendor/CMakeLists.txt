# ament wrapper for ACADO toolkit
cmake_minimum_required(VERSION 3.5)

project(acado_vendor)

find_package(ament_cmake REQUIRED)

list(APPEND extra_cmake_args -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})

if(DEFINED CMAKE_TOOLCHAIN_FILE)
  list(APPEND extra_cmake_args "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}")
endif()

message("extra_cmake_args: ${extra_cmake_args} ====")

include(ExternalProject)

set(acado_commit 8366e22cd5d815078e8d24f78dbced28b48d7e32)
externalproject_add(acado-${acado_commit}
  GIT_REPOSITORY https://github.com/cho3/acado.git
  GIT_TAG ${acado_commit}
  GIT_SHALLOW 1
  TIMEOUT 600
  UPDATE_COMMAND ""
  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/acado_external_project_install
    -DCMAKE_C_FLAGS="-w"
    -DCMAKE_CXX_FLAGS="-w"
    -DCMAKE_SKIP_INSTALL_RPATH=YES
    ${extra_cmake_args}
    -Wno-dev
)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/acado_external_project_install/
  DESTINATION ${CMAKE_INSTALL_PREFIX}
  USE_SOURCE_PERMISSIONS)

set(hpmpc_commit abbcf0806e4c8763ab2129a90efd827d194c1b00)
externalproject_add(hpmpc-${hpmpc_commit}
  GIT_REPOSITORY https://github.com/cho3/hpmpc.git
  GIT_TAG ${hpmpc_commit}
  GIT_SHALLOW 1
  TIMEOUT 600
  UPDATE_COMMAND ""
  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/hpmpc_external_project_install
    -DCMAKE_C_FLAGS="-w"
    -DCMAKE_CXX_FLAGS="-w"
    -DCMAKE_SKIP_INSTALL_RPATH=YES
    ${extra_cmake_args}
    -Wno-dev
)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/hpmpc_external_project_install/
  DESTINATION ${CMAKE_INSTALL_PREFIX}
  USE_SOURCE_PERMISSIONS)

# Explicitly export include headers and libraries so ament knows about them
ament_export_include_directories(include)
ament_export_libraries(
  acado_toolkit_s # For code generation
  hpmpc)

# Install cmake helper functions
install(
  DIRECTORY cmake
  DESTINATION share/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package(CONFIG_EXTRAS "acado_vendor-extras.cmake")
