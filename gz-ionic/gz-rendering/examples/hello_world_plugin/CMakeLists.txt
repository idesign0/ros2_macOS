cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)

find_package(gz-rendering9 REQUIRED)
set(GZ_RENDERING_VER ${gz-rendering9_VERSION_MAJOR})

find_package(gz-plugin3 REQUIRED COMPONENTS all)
set(GZ_PLUGIN_VER ${gz-plugin3_VERSION_MAJOR})

find_package(gz-common6 REQUIRED)
set(GZ_COMMON_VER ${gz-common6_VERSION_MAJOR})

add_library(HelloWorldPlugin SHARED HelloWorldPlugin.cc)
target_link_libraries(HelloWorldPlugin
  PUBLIC
    gz-rendering${GZ_RENDERING_VER}::gz-rendering${GZ_RENDERING_VER}
    gz-common${GZ_COMMON_VER}::gz-common${GZ_COMMON_VER}
  PRIVATE
    gz-plugin${GZ_PLUGIN_VER}::register
)
