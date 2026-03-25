function(cx_helper_to_snake_case input_string output_variable)
    # Step 1: Handle consecutive uppercase letters properly
    # Insert underscores between capital-lower or capital-capital-lower sequences.
    string(REGEX REPLACE "([A-Z]+)([A-Z][a-z])" "\\1_\\2" temp_string "${input_string}")
    # Step 2: Replace remaining single capital letters with an underscore before them
    string(REGEX REPLACE "([a-z0-9])([A-Z])" "\\1_\\2" temp_string "${temp_string}")
    # Step 3: Convert the whole string to lowercase
    string(TOLOWER "${temp_string}" temp_string)
    # Step 4: Remove leading underscores, if any
    string(REGEX REPLACE "^_" "" temp_string "${temp_string}")
    # Step 5: Return the result
    set(${output_variable} "${temp_string}" PARENT_SCOPE)
endfunction()

macro(_cx_generate_linked_protobuf_plugin_base name lib PROTO_FILES)
  cx_helper_to_snake_case(${name} snake_case_name)

  find_package(Python3 REQUIRED COMPONENTS Interpreter)
  find_package(Protobuf)
  find_package(ProtobufComm)
  find_package(ament_cmake REQUIRED)
  find_package(cx_protobuf_plugin REQUIRED)
  find_package(pluginlib REQUIRED)
  find_package(clips_vendor REQUIRED)
  find_package(clips REQUIRED)
  set(package_prefix_path "")
  ament_index_has_resource(package_prefix_path "packages" "cx_protobuf_plugin")
  if(package_prefix_path)
    set(GENERATOR_SCRIPT "${package_prefix_path}/lib/cx_protobuf_plugin/generator.py")
    if(NOT EXISTS "${GENERATOR_SCRIPT}")
        message(FATAL_ERROR "Python script ${GENERATOR_SCRIPT} does not exist")
    endif()
    set(TEMPLATES_DIR "${package_prefix_path}/share/cx_protobuf_plugin/templates/")
    # Generator script
    add_custom_command(
        OUTPUT  ${snake_case_name}.cpp ${snake_case_name}.hpp ${snake_case_name}.xml
        COMMAND ${Python3_EXECUTABLE} ${GENERATOR_SCRIPT} ${name}
        DEPENDS ${GENERATOR_SCRIPT}
                ${TEMPLATES_DIR}/plugin.jinja.xml
                ${TEMPLATES_DIR}/plugin.jinja.cpp
                ${TEMPLATES_DIR}/plugin.jinja.hpp
        COMMENT "Generate sub plugin for protobuf"
    )
    if(NOT DEFINED lib OR lib STREQUAL "")
      protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

      # Create a shared library for the generated protobuf files
      add_library(${snake_case_name}_proto_messages SHARED ${PROTO_SRCS} ${PROTO_HDRS})
      target_link_libraries(${snake_case_name}_proto_messages PUBLIC protobuf::libprotobuf ProtobufComm::protobuf_comm)
      target_include_directories(${snake_case_name}_proto_messages PUBLIC ${Protobuf_INCLUDE_DIRS})
      install(TARGETS
        ${snake_case_name}_proto_messages
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION lib/${PROJECT_NAME}
      )
    endif()
    # Build plugin from library
    add_library(${snake_case_name} SHARED ${snake_case_name}.cpp)
    set_property(TARGET ${snake_case_name} PROPERTY CXX_STANDARD 20)
    if(NOT DEFINED lib OR lib STREQUAL "")
      target_link_libraries(${snake_case_name} ClipsNS::libclips_ns ${snake_case_name}_proto_messages)
    else()
      target_link_libraries(${snake_case_name} ClipsNS::libclips_ns ${lib})
    endif()
    ament_target_dependencies(${snake_case_name} cx_plugin pluginlib cx_protobuf_plugin)
    install(
      FILES ${CMAKE_CURRENT_BINARY_DIR}/${snake_case_name}.hpp
      DESTINATION include/
    )
    file(MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/include)
    install(
      FILES ${CMAKE_CURRENT_BINARY_DIR}/${snake_case_name}.xml
      DESTINATION share/${PROJECT_NAME}
    )

    # install library and register the plugin plugin
    install(TARGETS
      ${snake_case_name}
      ARCHIVE DESTINATION lib
      LIBRARY DESTINATION lib
      RUNTIME DESTINATION lib/${PROJECT_NAME}
    )
    get_filename_component(relative_dir "${relative_filename}" DIRECTORY)
    # this is just the relevant part from
    # pluginlib_export_plugin_description_file without checking for existence of the file as it checks in src dir only.
    # As the macro generates the xml file, it is located in build dir instead
    set(plugin_category cx_plugin)
    set(relative_filename ${snake_case_name}.xml)
    set(relative_dir "")
    set(__PLUGINLIB_CATEGORY_CONTENT__${plugin_category}
      "${__PLUGINLIB_CATEGORY_CONTENT__${plugin_category}}share/${PROJECT_NAME}/${relative_filename}\n")
    list(APPEND __PLUGINLIB_PLUGIN_CATEGORIES ${plugin_category})  # duplicates are removes on use
  else()
    message(FATAL_ERROR "Failed to query ament index for packages cx_protobuf_plugin")
  endif()
endmacro()

macro(cx_generate_linked_protobuf_plugin_from_proto name PROTO_FILES)
  _cx_generate_linked_protobuf_plugin_base(${name} "" ${PROTO_FILES})
endmacro()

macro(cx_generate_linked_protobuf_plugin_from_lib name lib)
  if(NOT DEFINED lib OR lib STREQUAL "")
    message(FATAL_ERROR "Failed to query ament index for packages cx_protobuf_plugin")
  else()
   _cx_generate_linked_protobuf_plugin_base(${name} ${lib})
  endif()
endmacro()
