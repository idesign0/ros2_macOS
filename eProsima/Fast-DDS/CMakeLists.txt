# Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

###############################################################################
# CMake build rules for Fast DDS
###############################################################################
cmake_minimum_required(VERSION 3.16.3)
cmake_policy(VERSION 3.16.3...3.20.3)

# Set CMAKE_BUILD_TYPE to Release by default.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

###############################################################################
# Project                                                                     #
###############################################################################
project(fastrtps VERSION "2.6.10" LANGUAGES C CXX)

set(PROJECT_NAME_LARGE "Fast RTPS")
string(TOUPPER "${PROJECT_NAME}" PROJECT_NAME_UPPER)

message(STATUS "Version: ${PROJECT_VERSION}")

###############################################################################
# eProsima build options
###############################################################################
option(EPROSIMA_BUILD "Activate internal building" OFF)

###############################################################################
# Warning level
###############################################################################
if(MSVC OR MSVC_IDE)
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif()

    if(EPROSIMA_EXTRA_CMAKE_CXX_FLAGS)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EPROSIMA_EXTRA_CMAKE_CXX_FLAGS}")
    endif()

    # make visual studio more gcc and clang like.
    # C4101 'identifier' : unreferenced local variable
    # C4189 'identifier' : local variable is initialized but not referenced
    # C4555  expression has no effect; expected expression with side-effect
    # C4715: 'Test': not all control paths return a value
    # C5038 data member 'member1' will be initialized after data member 'member2'
    # C4100 'identifier' : unreferenced formal parameter (matches clang -Wunused-lambda-capture)
    add_compile_options(/w34101 /w34189 /w34555 /w34715 /w35038 /w44100)

    if(EPROSIMA_BUILD)
        string(REPLACE "/DNDEBUG" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
    endif()
else()
    set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra -Wno-unknown-pragmas -Wno-error=deprecated-declarations")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-psabi")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 15,0)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-builtins")
        endif()
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl")
    endif()

    if(EPROSIMA_BUILD)
        string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
    endif()
endif()

###############################################################################
# GCC colors if using CCache
###############################################################################
if("${CMAKE_CXX_COMPILER_LAUNCHER}" STREQUAL "ccache" AND
        CMAKE_COMPILER_IS_GNUCXX AND
        CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4,8)
    add_compile_options(-fdiagnostics-color=always)
endif()

###############################################################################
# Test system configuration
###############################################################################
include(${PROJECT_SOURCE_DIR}/cmake/common/check_configuration.cmake)

set(FORCE_CXX "11" CACHE STRING "C++ standard fulfillment selection")
check_stdcxx()

check_compile_feature()
check_endianness()

###############################################################################
# Installation paths
###############################################################################
option(APPEND_PROJECT_NAME_TO_INCLUDEDIR
  "When ON headers are installed to a path ending with a folder called \
  ${PROJECT_NAME}. This avoids include directory search order issues when \
  overriding this package from a merged catkin, ament, or colcon workspace."
  OFF)

set(BIN_INSTALL_DIR bin/ CACHE PATH "Installation directory for binaries")
set(_include_dir "include/")
if(APPEND_PROJECT_NAME_TO_INCLUDEDIR)
  string(APPEND _include_dir "${PROJECT_NAME}/")
endif()
set(INCLUDE_INSTALL_DIR "${_include_dir}" CACHE PATH "Installation directory for C++ headers")
unset(_include_dir)
set(LIB_INSTALL_DIR lib/ CACHE PATH "Installation directory for libraries")
set(DATA_INSTALL_DIR share/ CACHE PATH "Installation directory for data")
if(WIN32)
    set(DOC_DIR "doc")
else()
    set(DOC_DIR "${DATA_INSTALL_DIR}/doc")
endif()
set(DOC_INSTALL_DIR ${DOC_DIR} CACHE PATH "Installation directory for documentation")
set(LICENSE_INSTALL_DIR ${DATA_INSTALL_DIR}/${PROJECT_NAME} CACHE PATH "Installation directory for licenses")

###############################################################################
# Internal debug messages
###############################################################################

if(EPROSIMA_BUILD)
    set(INTERNAL_DEBUG ON)
endif()

###############################################################################
# Load CMake modules
###############################################################################
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake/modules)

###############################################################################
# Default shared libraries
###############################################################################
# Global flag to cause add_library() to create shared libraries if on.
# If set to true, this will cause all libraries to be built shared
# unless the library was explicitly added as a static library.
option(BUILD_SHARED_LIBS "Create shared libraries by default" ON)

###############################################################################
# Load external projects.
###############################################################################
include(${PROJECT_SOURCE_DIR}/cmake/common/eprosima_libraries.cmake)

# if we are building Fast-DDS as a static library we must load Fast-CDR as one
if(NOT BUILD_SHARED_LIBS)
    set(FASTDDS_STATIC ON)
endif()

eprosima_find_package(fastcdr REQUIRED)
eprosima_find_thirdparty(ASIO asio VERSION 1.10.8)
eprosima_find_thirdparty(TinyXML2 tinyxml2)

find_package(foonathan_memory REQUIRED)
message(STATUS "Found foonathan_memory: ${foonathan_memory_DIR}")
find_package(ThirdpartyBoost REQUIRED)

if(ANDROID)
    if((ANDROID_PLATFORM LESS_EQUAL 23) OR (ANDROID_NATIVE_API_LEVEL LESS_EQUAL 23))
        eprosima_find_thirdparty(android-ifaddrs android-ifaddrs)
    endif()
endif()

include_directories(thirdparty/nlohmann-json)
include_directories(thirdparty/filewatch)

###############################################################################
# Options
###############################################################################
option(SECURITY "Activate security" OFF)

if(SECURITY)
    find_package(OpenSSL REQUIRED)
else()
    find_package(OpenSSL)
endif()

if(OPENSSL_FOUND)
    message(STATUS "OpenSSL library ${OPENSSL_VERSION} found...")
endif()

option(NO_TLS "Disables TLS Support" OFF)
if(OPENSSL_FOUND AND NOT NO_TLS)
    set(TLS_FOUND 1)
else()
    set(TLS_FOUND 0)
endif()

if(SECURITY OR TLS_FOUND)
    set(LINK_SSL 1)
else()
    set(LINK_SSL 0)
endif()

option(SQLITE3_SUPPORT "Activate SQLITE3 support" ON)

###############################################################################
# SHM as Default transport
###############################################################################
option(SHM_TRANSPORT_DEFAULT "Add SHM transport to the default transports" ON)

###############################################################################
# LogConsumer default setup
###############################################################################
set(LOG_CONSUMER_DEFAULT AUTO CACHE STRING "Selects default LogConsumer")
set_property(CACHE LOG_CONSUMER_DEFAULT PROPERTY STRINGS AUTO STDOUT STDOUTERR)

if(LOG_CONSUMER_DEFAULT STREQUAL "STDOUTERR")
    set(STDOUTERR_LOG_CONSUMER ON)
endif()

###############################################################################
# Log Info default setup
###############################################################################

include(CMakeDependentOption)

set(LOG_NO_INFO_HELP "\
Do not compile Info Log level. For the sake of efficiency non-debug \
configs do not log info messages unless the option FASTDDS_ENFORCE_LOG_INFO \
is set to on.")
if(CMAKE_BUILD_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$"  # single config generator
   OR ("Debug" IN_LIST CMAKE_CONFIGURATION_TYPES))      # multi config generator
    option(LOG_NO_INFO ${LOG_NO_INFO_HELP} OFF)
else()
    option(LOG_NO_INFO ${LOG_NO_INFO_HELP} ON)
endif()
unset(LOG_NO_INFO_HELP)

option(LOG_NO_WARNING "Do not compile Warning Log level" OFF)

option(LOG_NO_ERROR "Do not compile Error Log level" OFF)

cmake_dependent_option(
    FASTDDS_ENFORCE_LOG_INFO
    "The LOG_NO_INFO option must be enforced regardless of selected configuration"
    OFF
    "NOT LOG_NO_INFO"
    OFF)

if(LOG_NO_INFO)
    set(HAVE_LOG_NO_INFO 1)
else()
    set(HAVE_LOG_NO_INFO 0)
endif()

if(LOG_NO_WARNING)
    set(HAVE_LOG_NO_WARNING 1)
else()
    set(HAVE_LOG_NO_WARNING 0)
endif()

if(LOG_NO_ERROR)
    set(HAVE_LOG_NO_ERROR 1)
else()
    set(HAVE_LOG_NO_ERROR 0)
endif()

###############################################################################
# Tools default setup
###############################################################################
option(COMPILE_TOOLS "Build tools" ON)

if(EPROSIMA_BUILD)
    set(COMPILE_TOOLS ON)
endif()

###############################################################################
# Fast DDS statistics tool default setup
###############################################################################
option(FASTDDS_STATISTICS "Enable Fast DDS Statistics Module" OFF)

###############################################################################
# Compile library.
###############################################################################
add_subdirectory(src/cpp)

###############################################################################
# Add http://optionparser.sourceforge.net/ as unified cli parser
###############################################################################
add_subdirectory(thirdparty/optionparser)

###############################################################################
# Testing options
###############################################################################
enable_testing()
include(CTest)
add_subdirectory(test)

###############################################################################
# Examples
###############################################################################
option(COMPILE_EXAMPLES "Build example" OFF)

if(EPROSIMA_BUILD)
    set(COMPILE_EXAMPLES ON)
endif()

if(COMPILE_EXAMPLES)
    add_subdirectory(examples)
endif()

###############################################################################
# Fuzzers
###############################################################################
if(DEFINED ENV{LIB_FUZZING_ENGINE})
    add_subdirectory(fuzz)
endif()

###############################################################################
# Tools
###############################################################################

if(COMPILE_TOOLS)
    add_subdirectory(tools)
endif()

###############################################################################
# Documentation
###############################################################################
# Add an option to toggle the generation of the API documentation.
option(BUILD_DOCUMENTATION "Use doxygen to create product documentation" OFF)
option(CHECK_DOCUMENTATION "Use doxygen to check code documentation" OFF)

if(CHECK_DOCUMENTATION)
    set(BUILD_DOCUMENTATION ON)
endif()

if(BUILD_DOCUMENTATION)
    find_package(Doxygen)
    if(NOT DOXYGEN_FOUND)
        message(FATAL_ERROR "doxygen is needed to build the documentation. Please install it correctly")
    endif()
    if(UNIX)
        find_program(DOXYFILE_MAKE make)
        if(DOXYFILE_MAKE)
            message(STATUS "Found Make: ${DOXYFILE_MAKE}")
        else()
            message(FATAL_ERROR "make is needed to build the documentation. Please install it correctly")
        endif()
    elseif(WIN32)
        set(DOXYFILE_MAKE make.bat)
    endif()

    if(NOT CHECK_DOCUMENTATION)
        find_program(UNZIP_EXE unzip)
        if(UNZIP_EXE)
            message(STATUS "Found Unzip: ${UNZIP_EXE}")
        else()
            message(FATAL_ERROR "unzip is needed to build the documentation. Please install it correctly")
        endif()
    endif()

    # Target to create documentation directories
    add_custom_target(fastdds_docdirs
        COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/doc
        COMMENT "Creating documentation directory" VERBATIM)

    ### Doxygen ########################3
    if(CHECK_DOCUMENTATION)
        set(USE_DOT NO)
    else()
        set(USE_DOT YES)
    endif()
    # Configure the template doxyfile for or specific project
    configure_file(doxyfile.in ${PROJECT_BINARY_DIR}/doxyfile @ONLY IMMEDIATE)
    # Add custom target to run doxygen when ever the project is build
    add_custom_target(fastdds_doxygen
        COMMAND "${DOXYGEN_EXECUTABLE}" "${PROJECT_BINARY_DIR}/doxyfile"
        SOURCES "${PROJECT_BINARY_DIR}/doxyfile"
        COMMENT "Generating API documentation with doxygen" VERBATIM)

    add_dependencies(fastdds_doxygen fastdds_docdirs)

    ### README html ########################

    if(WIN32)
        set(README_LOCATION "${PROJECT_BINARY_DIR}/")
        set(README_LOCATION_PREFFIX "doc/")
        set(README_INSTALL_LOCATION ".")
    else()
        set(README_LOCATION "${PROJECT_BINARY_DIR}/doc/")
        set(README_INSTALL_LOCATION "${DOC_INSTALL_DIR}")
    endif()

    configure_file(doc/README.html.in ${README_LOCATION}/README.html @ONLY IMMEDIATE)

    ### ReadTheDocs ########################
    if(NOT CHECK_DOCUMENTATION)

        file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/readthedocs_custom_template.cmake [=[

            file(DOWNLOAD "https://fast-dds.docs.eprosima.com/_/downloads/en/v${PROJECT_VERSION}/htmlzip/" "./eprosima-fast-rtps.zip"
                 STATUS DOWNLOAD_STATUS)

            # As stated in CMake's documentation: https://cmake.org/cmake/help/latest/command/file.html#transfer
            # STATUS will return a list with two elements: the numeric status and the message returned.
            # A 0 numeric error means no error in the operation
            list(GET DOWNLOAD_STATUS 0 DOWNLOAD_RC)

            if (DOWNLOAD_RC)
                message(STATUS "Unable to download documentation for version ${PROJECT_VERSION}. Falling back to latest.")
                file(DOWNLOAD "https://fast-dds.docs.eprosima.com/_/downloads/en/latest/htmlzip/" "./eprosima-fast-rtps.zip")
            endif()
            # TODO: when windows ci CMake version surpasses 17 favor file() instead of UNZIP as in the next line
            # file(ARCHIVE_EXTRACT INPUT "./eprosima-fast-rtps.zip" DESTINATION  "${PROJECT_BINARY_DIR}/doc/")
            execute_process(COMMAND "${UNZIP_EXE}" "./eprosima-fast-rtps.zip" -d "${PROJECT_BINARY_DIR}/doc/")
            file(REMOVE_RECURSE "${PROJECT_BINARY_DIR}/doc/manual")
            if (DOWNLOAD_RC)
                file(RENAME "${PROJECT_BINARY_DIR}/doc/eprosima-fast-rtps-latest" "${PROJECT_BINARY_DIR}/doc/manual")
            else()
                file(RENAME "${PROJECT_BINARY_DIR}/doc/eprosima-fast-rtps-v${PROJECT_VERSION}" "${PROJECT_BINARY_DIR}/doc/manual")
            endif()
            file(REMOVE  "./eprosima-fast-rtps.zip")

            ]=])

        configure_file(${CMAKE_CURRENT_BINARY_DIR}/readthedocs_custom_template.cmake  ${CMAKE_CURRENT_BINARY_DIR}/readthedocs_custom.cmake)

        add_custom_target(readthedocs
            COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/readthedocs_custom.cmake
            )

        add_dependencies(readthedocs fastdds_docdirs)
    endif()

    add_custom_target(fastdds_docs ALL
        COMMENT "Generated project documentation" VERBATIM)

    add_dependencies(fastdds_docs fastdds_doxygen)
    if(NOT CHECK_DOCUMENTATION)
        add_dependencies(fastdds_docs readthedocs)
    endif()
endif()

###############################################################################
# Packaging
###############################################################################
# Install licenses
install(FILES ${PROJECT_SOURCE_DIR}/LICENSE
    DESTINATION ${LICENSE_INSTALL_DIR}
    COMPONENT licenses
    )

option(INSTALL_EXAMPLES "Install example" OFF)

if(INSTALL_EXAMPLES)
  # Install examples
  install(DIRECTORY ${PROJECT_SOURCE_DIR}/examples/C++
      DESTINATION examples/
      COMPONENT examples
      PATTERN "examples/CMakeLists.txt" EXCLUDE
      )
endif()

option(INSTALL_TOOLS "Install tools" OFF)

if(INSTALL_TOOLS)
  # Install tools
  install(DIRECTORY ${PROJECT_SOURCE_DIR}/tools
      DESTINATION tools/
      COMPONENT tools
      PATTERN "tools/CMakeLists.txt" EXCLUDE
      )
endif()

if(BUILD_DOCUMENTATION)

    # Instalation of doxygen files
    install(DIRECTORY ${PROJECT_BINARY_DIR}/doc/api_reference
        DESTINATION ${DOC_INSTALL_DIR}
        COMPONENT documentation
        )

    install(FILES "${README_LOCATION}/README.html"
        DESTINATION ${README_INSTALL_LOCATION}
        COMPONENT documentation
        )

    if(NOT CHECK_DOCUMENTATION)
        install(DIRECTORY ${PROJECT_BINARY_DIR}/doc/manual
            DESTINATION ${DOC_INSTALL_DIR}
            COMPONENT documentation
            )
    endif()
endif()
