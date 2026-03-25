##################################################
# A function that calls protoc on a protobuf file
# Options:
#   GENERATE_CPP        - generates c++ code for the message if specified
#   GENERATE_PYTHON     - generates python code for the message if specified
# One value arguments:
#   PYTHON_INTERPRETER  - Target or path to the Python interpreter to use to run generation scripts
#   MSGS_GEN_SCRIPT     - Path to the message generation python script
#   PROTO_PACKAGE       - Protobuf package the file belongs to (e.g. "gz.msgs")
#   PROTOC_EXEC         - Path to protoc
#   GZ_PROTOC_PLUGIN    - Path to the gazebo-specific protobuf compiler executable
#   DLLEXPORT_DECL      - DLL visibility macro (eg GZ_MSGS_VISIBLE)
#   INPUT_PROTO         - Path to the input .proto file
#   OUTPUT_CPP_DIR      - Path where C++ files are saved
#   OUTPUT_PYTON_DIR    - Path where Python files are saved
#   OUTPUT_CPP_HH_VAR   - A CMake variable name containing a list generated headers should be appended to
#   OUTPUT_DETAIL_CPP_HH_VAR - A CMake variable name containing a list that the C++ detail headers should be appended to
#   OUTPUT_CPP_CC_VAR   - A Cmake variable name containing a list that the C++ source files should be appended to
#   OUTPUT_PYTHON_VAR - A Cmake variable name containing a list that the Python source files should be appended to
# Multi value arguments
#   PROTO_PATH          - Passed to protoc --proto_path
#   DEPENDENCY_PROTO_DESC - Passed to protoc --proto_path
function(gz_msgs_protoc)
  set(options GENERATE_CPP GENERATE_PYTHON)
  set(oneValueArgs
    PYTHON_INTERPRETER
    MSGS_GEN_SCRIPT
    PROTO_PACKAGE
    PROTOC_EXEC
    GZ_PROTOC_PLUGIN
    DLLEXPORT_DECL
    INPUT_PROTO
    OUTPUT_CPP_DIR
    OUTPUT_CPP_HH_VAR
    OUTPUT_DETAIL_CPP_HH_VAR
    OUTPUT_CPP_CC_VAR
    OUTPUT_PYTHON_DIR
    OUTPUT_PYTHON_VAR)
  set(multiValueArgs PROTO_PATH DEPENDENCY_PROTO_DESCS)

  cmake_parse_arguments(gz_msgs_protoc "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if (NOT DEFINED gz_msgs_protoc_PYTHON_INTERPRETER)
    set(gz_msgs_protoc_PYTHON_INTERPRETER Python3::Interpreter)
  endif()
  if (NOT DEFINED gz_msgs_protoc_PROTOC_EXEC)
    set(gz_msgs_protoc_PROTOC_EXEC protobuf::protoc)
  endif()

  get_filename_component(ABS_FIL ${gz_msgs_protoc_INPUT_PROTO} ABSOLUTE)
  get_filename_component(FIL_WE ${gz_msgs_protoc_INPUT_PROTO} NAME_WE)

  set(protoc_args)
  set(output_files)

  _gz_msgs_proto_pkg_to_path(${gz_msgs_protoc_PROTO_PACKAGE} proto_package_dir)

  if(gz_msgs_protoc_GENERATE_CPP)
    # Full path to gazeob-specific header (${PROJECT_BINARY_DIR}/include/gz/msgs/foo.pb.h)
    set(output_header "${gz_msgs_protoc_OUTPUT_CPP_DIR}/${proto_package_dir}/${FIL_WE}.pb.h")
    # Full path to generated protobuf header (${PROJECT_BINARY_DIR}/include/gz/msgs/details/foo.pb.h)
    set(output_detail_header "${gz_msgs_protoc_OUTPUT_CPP_DIR}/${proto_package_dir}/details/${FIL_WE}.pb.h")
    # Full path to generated protobuf source (${PROJECT_BINARY_DIR}/include/foo.pb.cc)
    set(output_source "${gz_msgs_protoc_OUTPUT_CPP_DIR}/${proto_package_dir}/${FIL_WE}.pb.cc")

    _gz_msgs_proto_to_unique(${gz_msgs_protoc_INPUT_PROTO} ${gz_msgs_protoc_PROTO_PACKAGE} UNIQUE_NAME)

    # Full path to an index file, which contains all defined message types for that proto file
    set(output_index "${gz_msgs_protoc_OUTPUT_CPP_DIR}/${UNIQUE_NAME}.pb_index")

    # Generate a clean relative path (gz/msgs/foo.pb.h)
    string(REPLACE "${PROJECT_BINARY_DIR}/include/" "" output_include ${output_header})

    list(APPEND ${gz_msgs_protoc_OUTPUT_CPP_HH_VAR} ${output_header})
    list(APPEND ${gz_msgs_protoc_OUTPUT_CPP_CC_VAR} ${output_source})
    list(APPEND ${gz_msgs_protoc_OUTPUT_DETAIL_CPP_HH_VAR} ${output_detail_header})

    list(APPEND output_files ${output_header})
    list(APPEND output_files ${output_detail_header})
    list(APPEND output_files ${output_source})
    list(APPEND output_files ${output_index})

    set(${gz_msgs_protoc_OUTPUT_DETAIL_CPP_HH_VAR} ${${gz_msgs_protoc_OUTPUT_DETAIL_CPP_HH_VAR}} PARENT_SCOPE)
    set(${gz_msgs_protoc_OUTPUT_CPP_HH_VAR} ${${gz_msgs_protoc_OUTPUT_CPP_HH_VAR}} PARENT_SCOPE)
    set(${gz_msgs_protoc_OUTPUT_CPP_CC_VAR} ${${gz_msgs_protoc_OUTPUT_CPP_CC_VAR}} PARENT_SCOPE)
  endif()

  if(gz_msgs_protoc_GENERATE_PYTHON)
    file(MAKE_DIRECTORY ${gz_msgs_protoc_OUTPUT_PYTHON_DIR})
    # Note: Both proto2 and proto3 use the _pb2.py suffix (https://protobuf.dev/reference/python/python-generated/#invocation)
    set(output_python "${gz_msgs_protoc_OUTPUT_PYTHON_DIR}${proto_package_dir}/${FIL_WE}_pb2.py")
    list(APPEND ${gz_msgs_protoc_OUTPUT_PYTHON_VAR} ${output_python})
    list(APPEND output_files ${output_python})
    set(${gz_msgs_protoc_OUTPUT_PYTHON_VAR} ${${gz_msgs_protoc_OUTPUT_PYTHON_VAR}} PARENT_SCOPE)
  endif()

  if(TARGET ${gz_msgs_protoc_PROTOC_EXEC})
    set(gz_msgs_protoc_PROTOC_EXEC_FILE_ABS_PATH "$<TARGET_FILE:${gz_msgs_protoc_PROTOC_EXEC}>")
  else()
    set(gz_msgs_protoc_PROTOC_EXEC_FILE_ABS_PATH ${gz_msgs_protoc_PROTOC_EXEC})
  endif()

  set(GENERATE_ARGS
      --protoc-exec "${gz_msgs_protoc_PROTOC_EXEC_FILE_ABS_PATH}"
      --gz-generator-bin "${gz_msgs_protoc_GZ_PROTOC_PLUGIN}"
      --proto-path "${gz_msgs_protoc_PROTO_PATH}"
      --input-path "${ABS_FIL}"
  )

  if(gz_msgs_protoc_DEPENDENCY_PROTO_DESCS)
    list(APPEND GENERATE_ARGS
      --dependency-proto-descs "${gz_msgs_protoc_DEPENDENCY_PROTO_DESCS}"
    )
  endif()

  if(gz_msgs_protoc_DLLEXPORT_DECL)
    list(APPEND GENERATE_ARGS
      --dllexport-decl "${gz_msgs_protoc_DLLEXPORT_DECL}"
    )
  endif()

  if(${gz_msgs_protoc_GENERATE_CPP})
    list(APPEND GENERATE_ARGS
      --generate-cpp
      --output-cpp-path "${gz_msgs_protoc_OUTPUT_CPP_DIR}")
  endif()

  if(${gz_msgs_protoc_GENERATE_PYTHON})
    list(APPEND GENERATE_ARGS
      --generate-python
      --output-python-path "${gz_msgs_protoc_OUTPUT_PYTHON_DIR}")
  endif()

  add_custom_command(
    OUTPUT ${output_files}
    COMMAND ${gz_msgs_protoc_PYTHON_INTERPRETER}
    ARGS ${gz_msgs_protoc_MSGS_GEN_SCRIPT} ${GENERATE_ARGS}
    DEPENDS
      ${ABS_FIL}
    # While the script is executed in the source directory, it does not write
    # to the source tree.  All outputs are stored in the build directory.
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMENT "Running protoc on ${gz_msgs_protoc_INPUT_PROTO}"
    VERBATIM
  )

endfunction()
