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

macro(cx_generate_bindings package msg_name type)
  cx_helper_to_snake_case(${package} snake_case_package)
  cx_helper_to_snake_case(${msg_name} snake_case_msg_name)
  set(plugin_name cx_${snake_case_package}_${snake_case_msg_name}_plugin)
  message(STATUS "Generate bindings for ${package}/${type}/${msg_name}")

  find_package(Python3 REQUIRED COMPONENTS Interpreter)
  find_package(cx_plugin REQUIRED)
  find_package(cx_utils REQUIRED)
  find_package(${package} REQUIRED)
  find_package(pluginlib REQUIRED)
  find_package(clips_vendor REQUIRED)
  find_package(clips REQUIRED)
  if("${type}" MATCHES "action")
    find_package(rclcpp_action)
    set(extra_deps "rclcpp_action")
  endif()

  # Generator script
  set(GENERATOR_SCRIPT "${cx_ros_comm_gen_DIR}/../../../lib/cx_ros_comm_gen/generator.py")
  set(TEMPLATES_DIR "${cx_ros_comm_gen_DIR}/../templates/")
  if(NOT EXISTS "${GENERATOR_SCRIPT}")
      message(FATAL_ERROR "Python script ${GENERATOR_SCRIPT} does not exist")
  endif()
  ament_index_get_resource(idl_paths "rosidl_interfaces" ${package})
  string(REPLACE "\n" ";" idl_path_list "${idl_paths}")
  set(pattern "${msg_name}.${type}$")
  set(idl_suffix ${idl_path_list})
  list(FILTER idl_suffix INCLUDE REGEX "${pattern}")
  if(NOT idl_suffix)
    message(FATAL_ERROR "No matching interface definition found for ${msg_name}.${type} in package ${package}.")
  endif()

  ament_index_has_resource(package_prefix "packages" "cx_rl_interfaces")
  set(interface_file
    ${package_prefix}/share/${package}/${idl_suffix}
  )
  add_custom_command(
      OUTPUT  ${plugin_name}.cpp ${plugin_name}.hpp ${plugin_name}.xml
      COMMAND ${Python3_EXECUTABLE} ${GENERATOR_SCRIPT} ${type} ${package} ${msg_name}
      DEPENDS ${GENERATOR_SCRIPT}
              ${interace_file}
              ${TEMPLATES_DIR}/${type}.jinja.cpp
              ${TEMPLATES_DIR}/${type}.jinja.hpp
              ${TEMPLATES_DIR}/plugin.jinja.xml
              ${TEMPLATES_DIR}/set_field.jinja.cpp
              ${TEMPLATES_DIR}/get_field.jinja.cpp
              ${TEMPLATES_DIR}/create.jinja.cpp
              ${TEMPLATES_DIR}/destroy.jinja.cpp
              ${TEMPLATES_DIR}/ret_fun.jinja.cpp
              ${TEMPLATES_DIR}/void_fun.jinja.cpp
      COMMENT "Generate cx plugin for ${package} ${msg_name}"
  )

  # Build plugin from library
  add_library(${plugin_name} SHARED ${plugin_name}.cpp)
  set_property(TARGET ${plugin_name} PROPERTY CXX_STANDARD 20)
  target_link_libraries(${plugin_name} ClipsNS::libclips_ns)
  ament_target_dependencies(${plugin_name} cx_plugin cx_utils pluginlib ${package} ${extra_deps})
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/${plugin_name}.hpp
    DESTINATION include/
  )
  file(MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/include)
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/${plugin_name}.xml
    DESTINATION share/${PROJECT_NAME}
  )
  # install library and register the plugin plugin
  install(TARGETS
    ${plugin_name}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION lib/${PROJECT_NAME}
  )
  get_filename_component(relative_dir "${relative_filename}" DIRECTORY)
  # this is just the relevant part from
  # pluginlib_export_plugin_description_file without checking for existence of the file as it checks in src dir only.
  # As the macro generates the xml file, it is located in build dir instead
  set(plugin_category cx_plugin)
  set(relative_filename ${plugin_name}.xml)
  set(relative_dir "")
  set(__PLUGINLIB_CATEGORY_CONTENT__${plugin_category}
    "${__PLUGINLIB_CATEGORY_CONTENT__${plugin_category}}share/${PROJECT_NAME}/${relative_filename}\n")
  list(APPEND __PLUGINLIB_PLUGIN_CATEGORIES ${plugin_category})  # duplicates are removes on use
endmacro()

macro(cx_generate_msg_bindings package msg_name)
  cx_generate_bindings(${package} ${msg_name} "msg")
endmacro()

macro(cx_generate_srv_bindings package srv_name)
  cx_generate_bindings(${package} ${srv_name} "srv")
endmacro()

macro(cx_generate_action_bindings package action_name)
  cx_generate_bindings(${package} ${action_name} "action")
endmacro()
