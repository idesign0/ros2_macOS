if (websockets_FOUND)
  set (plugin gz-launch-websocket-server)
  set (sources WebsocketServer.cc)

  add_library(${plugin} SHARED ${sources})

  file (READ combined.proto WEBSOCKETSERVER_MESSAGE_DEFINITIONS)
  configure_file("MessageDefinitions.hh.in"
    "${CMAKE_CURRENT_BINARY_DIR}/MessageDefinitions.hh" @ONLY)
  # Add a dependency on binary source dir so that you can change the
  # combinded.proto file and `make` will rebuild the plugin.
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS combined.proto)
  target_include_directories(${plugin} PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/>
  )

  target_link_libraries(${plugin}
    PRIVATE
      ${websockets_LIBRARIES}
      ${PROJECT_LIBRARY_TARGET_NAME}
      gz-common${GZ_COMMON_VER}::gz-common${GZ_COMMON_VER}
      gz-common${GZ_COMMON_VER}::graphics
      gz-msgs${GZ_MSGS_VER}::core
      gz-plugin${GZ_PLUGIN_VER}::core
      gz-transport${GZ_TRANSPORT_VER}::core
  )

  install (TARGETS ${plugin} DESTINATION ${GZ_LAUNCH_PLUGIN_RELATIVE_INSTALL_PATH})
endif()
