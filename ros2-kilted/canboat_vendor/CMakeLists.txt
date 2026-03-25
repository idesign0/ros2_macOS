# Copyright 2025 Robotic ESP
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

cmake_minimum_required(VERSION 3.8)

project(canboat_vendor)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)

find_program(ADDLICENSE_EXECUTABLE NAMES addlicense)
if(ADDLICENSE_EXECUTABLE)
  add_custom_target(apply_license_headers
    COMMAND ${ADDLICENSE_EXECUTABLE}
            -c "Robotic ESP"
            -l apache
            -y 2025
            -ignore **/*.xml
            ${CMAKE_CURRENT_SOURCE_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    VERBATIM
  )
else()
  message(WARNING "addlicense not found; license headers won't be applied")
endif()

include(ExternalProject)
ExternalProject_Add(
  canboat
  PREFIX ${CMAKE_BINARY_DIR}/external
  GIT_REPOSITORY https://github.com/canboat/canboat.git
  GIT_TAG v5.1.3
  UPDATE_COMMAND ""
  CONFIGURE_COMMAND ""
  BUILD_IN_SOURCE 1
  PATCH_COMMAND
    ${CMAKE_COMMAND} -E echo "override CFLAGS += -fpie" >> <SOURCE_DIR>/Makefile &&
    ${CMAKE_COMMAND} -E echo "override LDFLAGS += -pie"  >> <SOURCE_DIR>/Makefile
  BUILD_COMMAND ${CMAKE_MAKE_PROGRAM} -C <SOURCE_DIR> BUILDDIR=rel
  INSTALL_COMMAND ""
  BUILD_BYPRODUCTS
    <SOURCE_DIR>/rel/actisense-serial
    <SOURCE_DIR>/rel/analyzer
    <SOURCE_DIR>/rel/analyzer-explain
    <SOURCE_DIR>/rel/analyzer-j1939
    <SOURCE_DIR>/rel/analyzer-explain-j1939
    <SOURCE_DIR>/rel/candump2analyzer
    <SOURCE_DIR>/rel/command-group-function
    <SOURCE_DIR>/rel/ikonvert-serial
    <SOURCE_DIR>/rel/iptee
    <SOURCE_DIR>/rel/n2kd
    <SOURCE_DIR>/rel/n2kd_monitor
    <SOURCE_DIR>/rel/nmea0183-serial
    <SOURCE_DIR>/rel/request-group-function
    <SOURCE_DIR>/rel/replay
    <SOURCE_DIR>/rel/socketcan-writer
)

ExternalProject_Get_Property(canboat SOURCE_DIR)
set(CANBOAT_BIN_DIR ${SOURCE_DIR}/rel)
set(CANBOAT_TOOLS
  actisense-serial
  analyzer
  analyzer-explain
  analyzer-j1939
  analyzer-explain-j1939
  candump2analyzer
  command-group-function
  ikonvert-serial
  iptee
  n2kd
  n2kd_monitor
  nmea0183-serial
  request-group-function
  replay
  socketcan-writer
)
set(CANBOAT_BINARIES "")
foreach(_t ${CANBOAT_TOOLS})
  list(APPEND CANBOAT_BINARIES ${CANBOAT_BIN_DIR}/${_t})
endforeach()

install(
  PROGRAMS ${CANBOAT_BINARIES}
  DESTINATION lib/${PROJECT_NAME}
)

if(TARGET install)
  add_dependencies(install canboat)
endif()

ament_package()
