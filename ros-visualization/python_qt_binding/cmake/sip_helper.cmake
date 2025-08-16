if(__PYTHON_QT_BINDING_SIP_HELPER_INCLUDED)
  return()
endif()
set(__PYTHON_QT_BINDING_SIP_HELPER_INCLUDED TRUE)

set(__PYTHON_QT_BINDING_SIP_HELPER_DIR ${CMAKE_CURRENT_LIST_DIR})

find_package(Python3 ${Python3_VERSION} REQUIRED COMPONENTS Interpreter Development)

execute_process(
  COMMAND ${Python3_EXECUTABLE} -c "import sipconfig; print(sipconfig.Configuration().sip_bin)"
  OUTPUT_VARIABLE PYTHON_SIP_EXECUTABLE
  ERROR_QUIET)

if(PYTHON_SIP_EXECUTABLE)
  string(STRIP ${PYTHON_SIP_EXECUTABLE} SIP_EXECUTABLE)
else()
  find_program(SIP_EXECUTABLE sip)
endif()

if(SIP_EXECUTABLE)
  message(STATUS "SIP binding generator available at: ${SIP_EXECUTABLE}")
  set(sip_helper_FOUND TRUE)
else()
  message(WARNING "SIP binding generator NOT available.")
  set(sip_helper_NOTFOUND TRUE)
endif()

#
# Run the SIP generator and compile the generated code into a library.
#
# .. note:: The target lib${PROJECT_NAME} is created.
#
# :param PROJECT_NAME: The name of the sip project
# :type PROJECT_NAME: string
# :param SIP_FILE: the SIP file to be processed
# :type SIP_FILE: string
#
# The following options can be used to override the default behavior:
#   SIP_CONFIGURE: the used configure script for SIP
#     (default: sip_configure.py in the same folder as this file)
#   SOURCE_DIR: the source dir (default: ${PROJECT_SOURCE_DIR}/src)
#   LIBRARY_DIR: the library dir (default: ${PROJECT_SOURCE_DIR}/src)
#   BINARY_DIR: the binary dir (default: ${PROJECT_BINARY_DIR})
#
# The following keywords arguments can be used to specify:
#   DEPENDS: depends for the custom command
#     (should list all sip and header files)
#   DEPENDENCIES: target dependencies
#     (should list the library for which SIP generates the bindings)
#
function(build_sip_binding PROJECT_NAME SIP_FILE)
    cmake_parse_arguments(sip "" "SIP_CONFIGURE;SOURCE_DIR;LIBRARY_DIR;BINARY_DIR" "DEPENDS;DEPENDENCIES" ${ARGN})
    if(sip_UNPARSED_ARGUMENTS)
        message(WARNING "build_sip_binding(${PROJECT_NAME}) called with unused arguments: ${sip_UNPARSED_ARGUMENTS}")
    endif()

    # set default values for optional arguments
    if(NOT sip_SIP_CONFIGURE)
        # default to sip_configure.py in this directory
        set(sip_SIP_CONFIGURE ${__PYTHON_QT_BINDING_SIP_HELPER_DIR}/sip_configure.py)
    endif()
    if(NOT sip_SOURCE_DIR)
        set(sip_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
    endif()
    if(NOT sip_LIBRARY_DIR)
        set(sip_LIBRARY_DIR ${PROJECT_SOURCE_DIR}/lib)
    endif()
    if(NOT sip_BINARY_DIR)
        set(sip_BINARY_DIR ${PROJECT_BINARY_DIR})
    endif()

    set(SIP_BUILD_DIR ${sip_BINARY_DIR}/sip/${PROJECT_NAME})

    set(INCLUDE_DIRS ${${PROJECT_NAME}_INCLUDE_DIRS} ${Python3_INCLUDE_DIRS})
    set(LIBRARIES ${${PROJECT_NAME}_LIBRARIES})
    set(LIBRARY_DIRS ${${PROJECT_NAME}_LIBRARY_DIRS})
    set(LDFLAGS_OTHER ${${PROJECT_NAME}_LDFLAGS_OTHER})

    add_custom_command(
        OUTPUT ${SIP_BUILD_DIR}/Makefile
        COMMAND ${Python3_EXECUTABLE} ${sip_SIP_CONFIGURE} ${SIP_BUILD_DIR} ${SIP_FILE} ${sip_LIBRARY_DIR}
          \"${INCLUDE_DIRS}\" \"${LIBRARIES}\" \"${LIBRARY_DIRS}\" \"${LDFLAGS_OTHER}\"
        DEPENDS ${sip_SIP_CONFIGURE} ${SIP_FILE} ${sip_DEPENDS}
        WORKING_DIRECTORY ${sip_SOURCE_DIR}
        COMMENT "Running SIP generator for ${PROJECT_NAME} Python bindings..."
    )

    if(NOT EXISTS "${sip_LIBRARY_DIR}")
        file(MAKE_DIRECTORY ${sip_LIBRARY_DIR})
    endif()

    if(WIN32)
      set(MAKE_EXECUTABLE NMake.exe)
    else()
      find_program(MAKE_PROGRAM NAMES make)
      message(STATUS "Found required make: ${MAKE_PROGRAM}")
      set(MAKE_EXECUTABLE ${MAKE_PROGRAM})
    endif()

    add_custom_command(
        OUTPUT ${sip_LIBRARY_DIR}/lib${PROJECT_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}
        COMMAND ${MAKE_EXECUTABLE}
        DEPENDS ${SIP_BUILD_DIR}/Makefile
        WORKING_DIRECTORY ${SIP_BUILD_DIR}
        COMMENT "Compiling generated code for ${PROJECT_NAME} Python bindings..."
    )

    add_custom_target(lib${PROJECT_NAME} ALL
        DEPENDS ${sip_LIBRARY_DIR}/lib${PROJECT_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}
        COMMENT "Meta target for ${PROJECT_NAME} Python bindings..."
    )
    add_dependencies(lib${PROJECT_NAME} ${sip_DEPENDENCIES})
endfunction()
