cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)

# Each source file in this directory is meant to be compiled into a plugin

file(GLOB plugin_files "*.cc")
list(SORT plugin_files)

foreach(plugin_file ${plugin_files})

  get_filename_component(plugin ${plugin_file} NAME_WE)

  add_library(${plugin} SHARED ${plugin_file})

  target_link_libraries(${plugin}
    PRIVATE
      gz-plugin${GZ_PLUGIN_VER}::register
      gz-math${GZ_MATH_VER}::core)

  # All these libraries will go into the same directory, so it's sufficient for
  # us to grab the last one.
  set(PLUGIN_LIBDIR "$<TARGET_FILE_DIR:${plugin}>" PARENT_SCOPE)

endforeach()
