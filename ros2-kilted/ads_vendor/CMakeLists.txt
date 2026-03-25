cmake_minimum_required(VERSION 3.13)

project(ads_vendor)

find_package(ament_cmake REQUIRED)

include(ExternalProject)


set(ADS_EXTERNAL_PROJECT_NAME ads_external_project)
set(ADS_GIT_REPOSITORY "https://github.com/Beckhoff/ADS.git")
set(ADS_GIT_TAG "e85ef696447a77039a4c3710d7573c8df32c0d58")

set(EXTERNAL_PROJECT_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/ads_external_build)
set(EXTERNAL_PROJECT_SOURCE_DIR ${EXTERNAL_PROJECT_PREFIX}/src/Ads)
set(EXTERNAL_PROJECT_BINARY_DIR ${EXTERNAL_PROJECT_PREFIX}/build/Ads)
set(EXTERNAL_PROJECT_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}) # ADS library should install its files (libraries, headers) into ads_vendor package install space.

# Conditionally append BUILD_TYPE and PARALLEL_LEVEL to build command for ExternalProject_Add
set(EXTERNAL_PROJECT_BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR>)

if(CMAKE_BUILD_TYPE)
  message(STATUS "ads_vendor: CMAKE_BUILD_TYPE for ADS library will be: ${CMAKE_BUILD_TYPE}")
  list(APPEND EXTERNAL_PROJECT_BUILD_COMMAND --config ${CMAKE_BUILD_TYPE})
  list(APPEND EXTERNAL_PROJECT_INSTALL_COMMAND --config ${CMAKE_BUILD_TYPE})
else()
  message(STATUS "ads_vendor: No CMAKE_BUILD_TYPE specified, defaulting to 'RelWithDebInfo'.")
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build." FORCE)
endif()

if(DEFINED CMAKE_BUILD_PARALLEL_LEVEL)
    list(APPEND ADS_BUILD_COMMAND -j${CMAKE_BUILD_PARALLEL_LEVEL})
endif()

ExternalProject_Add(${ADS_EXTERNAL_PROJECT_NAME}
    # download step
    GIT_REPOSITORY    ${ADS_GIT_REPOSITORY} 
    GIT_TAG           ${ADS_GIT_TAG}        

    # directiories config
    PREFIX            ${EXTERNAL_PROJECT_PREFIX}
    SOURCE_DIR        ${EXTERNAL_PROJECT_SOURCE_DIR}
    BINARY_DIR        ${EXTERNAL_PROJECT_BINARY_DIR}
    INSTALL_DIR       ${EXTERNAL_PROJECT_INSTALL_DIR}

    # after initial download, don't re-fetch source on subsequent CMake runs
    UPDATE_COMMAND    ""

    # configure step
    CONFIGURE_COMMAND ${CMAKE_COMMAND} -S <SOURCE_DIR> -B <BINARY_DIR>
                      -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
                      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                      -DBUILD_SHARED_LIBS=ON

    # build step
    BUILD_COMMAND     ${EXTERNAL_PROJECT_BUILD_COMMAND}

    # install step
    INSTALL_COMMAND   ""

    # logging
    LOG_CONFIGURE     YES
    LOG_BUILD         YES
    LOG_INSTALL       YES
)

# ros buildfarm fix: separate ExternalProject install step from the clone/uild steps.
# Reference: https://github.com/ros-industrial/ros2_canopen/blob/master/lely_core_libraries/CMakeLists.txt
install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} --install ${EXTERNAL_PROJECT_BINARY_DIR} --config ${CMAKE_BUILD_TYPE})")

add_library(ads_vendor INTERFACE)
add_library(ads_vendor::ads_vendor ALIAS ads_vendor)

target_include_directories(ads_vendor
  INTERFACE
  $<INSTALL_INTERFACE:include>
)
target_link_libraries(ads_vendor
  INTERFACE
  ${CMAKE_INSTALL_PREFIX}/lib/libads.so
)

install(
  TARGETS ads_vendor
  EXPORT ads_vendor_targets
)

ament_export_targets(ads_vendor_targets HAS_LIBRARY_TARGET)

ament_package()