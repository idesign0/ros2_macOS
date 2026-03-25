cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)

project(examples)

find_package(gz-plugin3 QUIET REQUIRED COMPONENTS all)
set(GZ_PLUGIN_VER ${gz-plugin3_VERSION_MAJOR})

find_package(gz-common6 QUIET)
set(GZ_COMMON_VER ${gz-common6_VERSION_MAJOR})

find_package(gz-math8 QUIET)
set(GZ_MATH_VER ${gz-math8_VERSION_MAJOR})

add_subdirectory(plugins)

file(GLOB example_files "*.cc")
list(SORT example_files)

find_package(Boost COMPONENTS program_options)
if(NOT Boost_PROGRAM_OPTIONS_FOUND)
  message(STATUS "Could not find Boost::program_options. Some examples might "
                 "have a reduced set of features available.")
endif()

foreach(example_file ${example_files})

  get_filename_component(example ${example_file} NAME_WE)

  add_executable(${example} ${example_file})

  target_link_libraries(${example}
    PRIVATE
      gz-plugin${GZ_PLUGIN_VER}::loader
      gz-common${GZ_COMMON_VER}::core
      gz-math${GZ_MATH_VER}::core
  )

  # This is only needed for the examples that use plugins, but it doesn't hurt
  # to set it for all the examples.
  target_compile_definitions(${example}
    PRIVATE
      "GZ_PLUGIN_EXAMPLES_LIBDIR=\"${PLUGIN_LIBDIR}\"")

  if(Boost_PROGRAM_OPTIONS_FOUND)
    target_link_libraries(${example} PRIVATE ${Boost_PROGRAM_OPTIONS_LIBRARIES})
    target_include_directories(${example} PRIVATE Boost_INCLUDE_DIRS)
    target_compile_definitions(${example} PRIVATE "HAVE_BOOST_PROGRAM_OPTIONS=1")
  endif()

endforeach()
