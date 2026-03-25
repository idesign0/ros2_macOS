set (plugin gz-launch-simgui)
set (sources SimGui.cc)

string (TOLOWER ${plugin} plugin_lower)

add_library(${plugin_lower} SHARED ${sources})
target_link_libraries(${plugin_lower}
  PRIVATE
  ${PROJECT_LIBRARY_TARGET_NAME}
  gz-sim${GZ_SIM_VER}
  gz-sim${GZ_SIM_VER}::gui
  gz-sim${GZ_SIM_VER}::gz-sim${GZ_SIM_VER}
  gz-gui${GZ_GUI_VER}::gz-gui${GZ_GUI_VER}
  gz-common${GZ_COMMON_VER}::gz-common${GZ_COMMON_VER}
  gz-transport${GZ_TRANSPORT_VER}::core
  gz-plugin${GZ_PLUGIN_VER}::core
)

install (TARGETS ${plugin} DESTINATION ${GZ_LAUNCH_PLUGIN_RELATIVE_INSTALL_PATH})
