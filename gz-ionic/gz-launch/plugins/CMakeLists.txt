set (plugins
  joy_to_twist
  sim_gui
  sim_factory
  sim_server
  websocket_server
)

if (UNIX AND NOT APPLE)
  set(plugins ${plugins}
        joystick)
endif()

foreach (plugin ${plugins})
  add_subdirectory(${plugin})
endforeach ()
