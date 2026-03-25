
add_library(bad_plugins SHARED bad_plugins.cc)

target_link_libraries(
  bad_plugins
  PRIVATE
  gz-plugin${GZ_PLUGIN_VER}::core
  ${PROJECT_LIBRARY_TARGET_NAME}
)
