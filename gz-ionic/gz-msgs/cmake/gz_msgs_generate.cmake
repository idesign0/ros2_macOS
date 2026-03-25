##################################################
# The implementation of gz_msgs_generate_messages
# Options:
# One value arguments:
#   PYTHON_INTERPRETER  - Target or path to the python interpreter used
#   PROTO_PACKAGE       - Protobuf package the file belongs to (e.g. "gz.msgs")
#   MSGS_GEN_SCRIPT     - Location of the message generator script
#   GZ_PROTOC_PLUGIN    - Location of the gazebo generator plugin
#   FACTORY_GEN_SCRIPT  - Location of the factory generator script
#   PROTO_PATH          - Base directory of the proto files
#   DEPENDENCY_DESCRIPTIONS - .gz_desc files for each dependency
#   DLLEXPORT_DECL      - Visibility macro to apply to messages
#   OUTPUT_DIRECTORY    - CMake binary directory to place generated artifacts
#   OUTPUT_SOURCES      - Variable to contain list of generated source files
#   OUTPUT_HEADERS      - Variable to contain list of generated header files
#   OUTPUT_DETAIL_HEADERS - Variable to contain list of generated detail header files
#   OUTPUT_PYTHON       - Variable to contain list of generated python files
# Multi value arguments
#   INPUT_PROTOS        - List of input proto files
function(gz_msgs_generate_messages_impl)
  set(options "")
  set(oneValueArgs
    # Inputs
    PYTHON_INTERPRETER
    PROTOC_EXEC
    PROTO_PACKAGE MSGS_GEN_SCRIPT GZ_PROTOC_PLUGIN FACTORY_GEN_SCRIPT PROTO_PATH
    DEPENDENCY_DESCRIPTIONS
    DLLEXPORT_DECL
    OUTPUT_DIRECTORY
    # Outputs
    OUTPUT_SOURCES
    OUTPUT_HEADERS
    OUTPUT_DETAIL_HEADERS
    OUTPUT_PYTHON
  )
  set(multiValueArgs INPUT_PROTOS)

  cmake_parse_arguments(generate_messages "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Default values for optional parameters
  if (NOT DEFINED generate_messages_PYTHON_INTERPRETER)
    set(generate_messages_PYTHON_INTERPRETER Python3::Interpreter)
  endif()
  if (NOT DEFINED generate_messages_PROTOC_EXEC)
    set(generate_messages_PROTOC_EXEC protobuf::protoc)
  endif()

  _gz_msgs_proto_pkg_to_string(${generate_messages_PROTO_PACKAGE} gen_dir)
  _gz_msgs_proto_pkg_to_path(${generate_messages_PROTO_PACKAGE} proto_package_dir)
  set(output_directory ${generate_messages_OUTPUT_DIRECTORY})

  file(MAKE_DIRECTORY ${output_directory})
  file(MAKE_DIRECTORY ${output_directory}/${proto_package_dir})

  foreach(proto_file ${generate_messages_INPUT_PROTOS})
    gz_msgs_protoc(
      PYTHON_INTERPRETER
        ${generate_messages_PYTHON_INTERPRETER}
      MSGS_GEN_SCRIPT
        ${generate_messages_MSGS_GEN_SCRIPT}
      PROTO_PACKAGE
        ${generate_messages_PROTO_PACKAGE}
      INPUT_PROTO
        ${proto_file}
      PROTOC_EXEC
        ${generate_messages_PROTOC_EXEC}
      GZ_PROTOC_PLUGIN
        ${generate_messages_GZ_PROTOC_PLUGIN}
      PROTO_PATH
        ${generate_messages_PROTO_PATH}
      DEPENDENCY_PROTO_DESCS
        ${generate_messages_DEPENDENCY_DESCRIPTIONS}

      # Cpp Specific arguments
      GENERATE_CPP
      DLLEXPORT_DECL
        ${generate_messages_DLLEXPORT_DECL}
      OUTPUT_CPP_HH_VAR
        ${generate_messages_OUTPUT_HEADERS}
      OUTPUT_DETAIL_CPP_HH_VAR
        ${generate_messages_OUTPUT_DETAIL_HEADERS}
      OUTPUT_CPP_CC_VAR
        ${generate_messages_OUTPUT_SOURCES}
      OUTPUT_CPP_DIR
        ${output_directory}

      # Python Specific arguments
      GENERATE_PYTHON
      OUTPUT_PYTHON_VAR
        ${generate_messages_OUTPUT_PYTHON}
        gen_sources_py
      OUTPUT_PYTHON_DIR
        ${output_directory}/python/
    )
  endforeach()

  gz_msgs_factory(
    PYTHON_INTERPRETER
      ${generate_messages_PYTHON_INTERPRETER}
    FACTORY_GEN_SCRIPT
      ${generate_messages_FACTORY_GEN_SCRIPT}
    PROTO_PACKAGE
      ${generate_messages_PROTO_PACKAGE}
    INPUT_PROTOS
      ${generate_messages_INPUT_PROTOS}
    OUTPUT_CPP_DIR
      ${output_directory}
    OUTPUT_CPP_HH_VAR
      ${generate_messages_OUTPUT_HEADERS}
    OUTPUT_CPP_CC_VAR
      ${generate_messages_OUTPUT_SOURCES}
    PROTO_PATH
      ${generate_messages_PROTO_PATH}
  )

  set_source_files_properties(
    ${${generate_messages_OUTPUT_SOURCES}}
    ${${generate_messages_OUTPUT_HEADERS}}
    ${${generate_messages_OUTPUT_DETAIL_HEADERS}}
    PROPERTIES GENERATED TRUE)

  if(WIN32)
    set_source_files_properties(${${generate_messages_OUTPUT_SOURCES}}
      COMPILE_FLAGS "/wd4100 /wd4512 /wd4127 /wd4068 /wd4244 /wd4267 /wd4251 /wd4146")
  endif()

  if(NOT MSVC)
    # -Wno-switch-default flags is required for suppressing a warning in some of
    # the generated protobuf files.
    set_source_files_properties(${${generate_messages_OUTPUT_SOURCES}}
      COMPILE_FLAGS "-Wno-switch-default -Wno-float-equal")
  endif()

  if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # Disable warning in generated *.pb.cc code
    set_source_files_properties(${${generate_messages_OUTPUT_SOURCES}}
      COMPILE_FLAGS "-Wno-invalid-offsetof")
  endif()

  set(${generate_messages_OUTPUT_SOURCES} ${${generate_messages_OUTPUT_SOURCES}} PARENT_SCOPE)
  set(${generate_messages_OUTPUT_HEADERS} ${${generate_messages_OUTPUT_HEADERS}} PARENT_SCOPE)
  set(${generate_messages_OUTPUT_DETAIL_HEADERS} ${${generate_messages_OUTPUT_DETAIL_HEADERS}} PARENT_SCOPE)
  set(${generate_messages_OUTPUT_PYTHON} ${${generate_messages_OUTPUT_PYTHON}} PARENT_SCOPE)
endfunction()

##################################################
# Options:
# One value arguments:
#   PROTOC_EXEC         - protoc target or executable to use
#   PROTO_PATH          - Base directory of the proto files
#   DEPENDENCY_DESCRIPTIONS - Variable containing all dependency description files
#   OUTPUT_DIRECTORY - Directory of output gz_desc file
#   OUTPUT_FILENAME - Name of output gz_desc file
# Multi value arguments
#   INPUT_PROTOS        - List of input proto files
function(gz_msgs_generate_desc_impl)
  set(options "")
  set(oneValueArgs
    # Inputs
    PROTOC_EXEC
    PROTO_PATH
    DEPENDENCY_DESCRIPTIONS
    OUTPUT_DIRECTORY
    OUTPUT_FILENAME)
  set(multiValueArgs INPUT_PROTOS)

  cmake_parse_arguments(generate_messages "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if (NOT DEFINED generate_messages_PROTOC_EXEC)
    set(generate_messages_PROTOC_EXEC protobuf::protoc)
  endif()

  set(ARGS)
  list(APPEND ARGS -I${generate_messages_PROTO_PATH})
  list(APPEND ARGS --descriptor_set_out=${generate_messages_OUTPUT_FILENAME})
  list(APPEND ARGS --include_imports)

  foreach(dep ${generate_messages_DEPENDENCY_DESCRIPTIONS})
    list(APPEND ARGS --descriptor_set_in="${msgs_desc_file}")
  endforeach()

  list(APPEND ARGS ${generate_messages_INPUT_PROTOS})

  add_custom_command(
    OUTPUT ${generate_messages_OUTPUT_FILENAME}
    COMMAND ${generate_messages_PROTOC_EXEC}
    ARGS ${ARGS}
    DEPENDS ${generate_messages_INPUT_PROTOS}
    COMMENT "Generating descriptor set"
  )
endfunction()

##################################################

# Options:
# One value arguments:
#   TARGET              - Target (static library) to create
#   PROTO_PACKAGE       - Protobuf package the file belongs to (e.g. "gz.msgs")
#   MSGS_GEN_SCRIPT     - Location of the message generator script
#   PYTHON_INTERPRETER  - Target or path to the python interpreter used
#   PROTOC_EXEC         - Protoc target or executable to use
#   GZ_PROTOC_PLUGIN    - Location of the gazebo generator plugin
#   FACTORY_GEN_SCRIPT  - Location of the factory generator script
#   MSGS_LIB            - gz-msgs library to link to
#   PROTO_PATH          - Base directory of the proto files
# Multi value arguments
#   INPUT_PROTOS        - List of input proto files
#   DEPENDENCIES        - List of generated messages targets that these messages depend on
#                         Primarily used when generating new custom messages downstream
#                         that depend on gz-msgs
function(gz_msgs_generate_messages_lib)
  set(options "")
  set(oneValueArgs TARGET PROTO_PACKAGE MSGS_GEN_SCRIPT PYTHON_INTERPRETER PROTOC_EXEC GZ_PROTOC_PLUGIN FACTORY_GEN_SCRIPT MSGS_LIB PROTO_PATH)
  set(multiValueArgs INPUT_PROTOS DEPENDENCIES)

  cmake_parse_arguments(generate_messages "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Set up output destination
  _gz_msgs_proto_pkg_to_string(${generate_messages_PROTO_PACKAGE} gen_dir)
  _gz_msgs_proto_pkg_to_path(${generate_messages_PROTO_PACKAGE} proto_package_dir)
  set(target_name ${PROJECT_NAME}-${generate_messages_TARGET})
  set(output_directory ${PROJECT_BINARY_DIR}/${target_name}_genmsg)

  # Extract dependency information from targets
  set(depends_proto_paths)
  set(depends_includes)
  foreach(dep ${generate_messages_DEPENDENCIES})
    get_target_property(msgs_desc_file ${dep} GZ_MSGS_DESC_FILE)
    list(APPEND depends_msgs_desc ${msgs_desc_file})
  endforeach()

  # Default values for optional parameters
  if (NOT DEFINED generate_messages_PYTHON_INTERPRETER)
    set(generate_messages_PYTHON_INTERPRETER Python3::Interpreter)
  endif()
  if (NOT DEFINED generate_messages_PROTOC_EXEC)
    set(generate_messages_PROTOC_EXEC protobuf::protoc)
  endif()

  gz_msgs_generate_messages_impl(
    PROTO_PACKAGE ${generate_messages_PROTO_PACKAGE}
    MSGS_GEN_SCRIPT ${generate_messages_MSGS_GEN_SCRIPT}
    PYTHON_INTERPRETER ${generate_messages_PYTHON_INTERPRETER}
    PROTOC_EXEC ${generate_messages_PROTOC_EXEC}
    GZ_PROTOC_PLUGIN ${generate_messages_GZ_PROTOC_PLUGIN}
    FACTORY_GEN_SCRIPT ${generate_messages_FACTORY_GEN_SCRIPT}
    PROTO_PATH ${generate_messages_PROTO_PATH}
    DEPENDENCY_DESCRIPTIONS ${depends_msgs_desc}
    INPUT_PROTOS ${generate_messages_INPUT_PROTOS}
    OUTPUT_DIRECTORY ${output_directory}
    OUTPUT_SOURCES generated_sources
    OUTPUT_HEADERS generated_headers
    OUTPUT_DETAIL_HEADERS generated_detail_headers
    OUTPUT_PYTHON generated_python
  )

  gz_msgs_generate_desc_impl(
    PROTOC_EXEC ${generate_messages_PROTOC_EXEC}
    INPUT_PROTOS ${generate_messages_INPUT_PROTOS}
    PROTO_PATH ${generate_messages_PROTO_PATH}
    DEPENDENCY_DESCRIPTIONS ${depends_msgs_desc}
    OUTPUT_FILENAME ${target_name}.gz_desc
  )

  add_library(${target_name} SHARED ${generated_sources} ${target_name}.gz_desc)

  foreach(dep ${generate_messages_DEPENDENCIES})
    add_dependencies(${target_name} ${dep})
  endforeach()

  target_link_libraries(${target_name} PUBLIC protobuf::libprotobuf ${generate_messages_MSGS_LIB} ${generate_messages_DEPENDENCIES})
  target_include_directories(${target_name} PUBLIC
    "$<INSTALL_INTERFACE:${GZ_INCLUDE_INSTALL_DIR_FULL}/>"
    "$<BUILD_INTERFACE:${output_directory}>")

  set_target_properties(
    ${target_name}
      PROPERTIES
        SOVERSION ${PROJECT_VERSION_MAJOR}
        VERSION ${PROJECT_VERSION}
        GZ_MSGS_DESC_FILE "\$\{_IMPORT_PREFIX\}/share/gz/protos/${generate_messages_TARGET}.gz_desc"
        )
  set_property(TARGET ${target_name} PROPERTY EXPORT_PROPERTIES "GZ_MSGS_DESC_FILE")

  # If this isn't being done as part of a gazebo project, set installation paths
  # to reasonable defaults
  if (NOT GZ_LIB_INSTALL_DIR)
    set(GZ_LIB_INSTALL_DIR lib)
    set(GZ_BIN_INSTALL_DIR bin)
    set(GZ_INCLUDE_INSTALL_DIR_FULL include)
    set(CMAKE_INSTALL_DATAROOTDIR share)
    set(NO_GZ_PACKAGE true)
  endif()


  install(
    TARGETS ${target_name}
    EXPORT ${target_name}
    LIBRARY DESTINATION ${GZ_LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${GZ_LIB_INSTALL_DIR}
    RUNTIME DESTINATION ${GZ_BIN_INSTALL_DIR}
    COMPONENT libraries)

  install(FILES ${generated_headers} DESTINATION ${GZ_INCLUDE_INSTALL_DIR_FULL}/${proto_package_dir})
  install(FILES ${generated_detail_headers} DESTINATION ${GZ_INCLUDE_INSTALL_DIR_FULL}/${proto_package_dir}/details)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${target_name}.gz_desc DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/gz/protos/)

  if (NOT GZ_PYTHON_INSTALL_PATH)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    if(USE_SYSTEM_PATHS_FOR_PYTHON_INSTALLATION)
      if(USE_DIST_PACKAGES_FOR_PYTHON)
        string(REPLACE "site-packages" "dist-packages" GZ_PYTHON_INSTALL_PATH ${Python3_SITELIB})
      else()
        # Python3_SITELIB might use dist-packages in some platforms
        string(REPLACE "dist-packages" "site-packages" GZ_PYTHON_INSTALL_PATH ${Python3_SITELIB})
      endif()
    else()
      # If not a system installation, respect local paths
      set(GZ_PYTHON_INSTALL_PATH ${GZ_LIB_INSTALL_DIR}/python)
    endif()
  endif()
  install(FILES ${generated_python} DESTINATION ${GZ_PYTHON_INSTALL_PATH}/gz/msgs${GZ_MSGS_VER})

  if (NOT NO_GZ_PACKAGE)
    set(component_name ${generate_messages_TARGET})
    set(component_pkg_name ${target_name})
    _gz_create_cmake_package(COMPONENT ${component_name})
    _gz_create_pkgconfig(COMPONENT ${component_name})
  endif()
endfunction()
