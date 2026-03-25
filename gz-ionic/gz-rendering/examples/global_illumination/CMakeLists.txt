cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)
project(gz-rendering-global_illumination)

find_package(gz-rendering9)

include_directories(SYSTEM
  ${PROJECT_BINARY_DIR}
)

find_package(SDL2 REQUIRED)
include_directories(SYSTEM ${SDL2_INCLUDE_DIRS})
link_directories(${SDL2_LIBRARY_DIRS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

configure_file (example_config.hh.in ${PROJECT_BINARY_DIR}/example_config.hh)

get_cmake_property(_variableNames VARIABLES)
list (SORT _variableNames)
foreach (_variableName ${_variableNames})
  message(STATUS "${_variableName}=${${_variableName}}")
endforeach()
add_executable(global_illumination Main.cc Sdl2Window.cc)

target_link_libraries(global_illumination
  ${SDL2_LIBRARIES}
  ${GZ-RENDERING_LIBRARIES}
  )

add_custom_command(TARGET global_illumination POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_SOURCE_DIR}/../ogre2_demo/media
  $<TARGET_FILE_DIR:global_illumination>/media)
