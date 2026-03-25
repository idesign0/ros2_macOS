set (plugin gz-launch-joytotwist)
set (sources JoyToTwist.cc)

add_library(${plugin} SHARED ${sources})
target_link_libraries(${plugin}
  PRIVATE
    ${PROJECT_LIBRARY_TARGET_NAME}
    gz-common${GZ_COMMON_VER}::gz-common${GZ_COMMON_VER}
    gz-math${GZ_MATH_VER}::gz-math${GZ_MATH_VER}
    gz-plugin${GZ_PLUGIN_VER}::core
    gz-transport${GZ_TRANSPORT_VER}::core
)

install (TARGETS ${plugin} DESTINATION ${GZ_LAUNCH_PLUGIN_RELATIVE_INSTALL_PATH})
