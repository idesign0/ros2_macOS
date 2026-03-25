# Collect source files into the "sources" variable and unit test files into the
# "gtest_sources" variable.
gz_get_libsources_and_unittests(sources gtest_sources)

add_subdirectory(base)

if (MSVC)
  # Warning #4251 is the "dll-interface" warning that tells you when types used
  # by a class are not being exported. These generated source files have private
  # members that don't get exported, so they trigger this warning. However, the
  # warning is not important since those members do not need to be interfaced
  # with.
  set_source_files_properties(${sources} ${gtest_sources} COMPILE_FLAGS "/wd4251")
endif()

# Create the library target.
gz_create_core_library(SOURCES ${sources})
gz_add_get_install_prefix_impl(GET_INSTALL_PREFIX_FUNCTION gz::rendering::getInstallPrefix
                               GET_INSTALL_PREFIX_HEADER gz/rendering/InstallationDirectories.hh
                               OVERRIDE_INSTALL_PREFIX_ENV_VARIABLE GZ_RENDERING_INSTALL_PREFIX)

# Include the interface directories that we always need.
#gz_target_interface_include_directories(${PROJECT_LIBRARY_TARGET_NAME}
#  gz-math${GZ_MATH_VER})

set_property(
  SOURCE RenderEngineManager.cc
  PROPERTY COMPILE_DEFINITIONS
  GZ_RENDERING_PLUGIN_PATH="${GZ_RENDERING_PLUGIN_PATH}"
)

set_property(
  SOURCE InstallationDirectories.cc
  PROPERTY COMPILE_DEFINITIONS
  GZ_RENDERING_RELATIVE_RESOURCE_PATH="${GZ_RENDERING_RELATIVE_RESOURCE_PATH}"
  GZ_RENDERING_ENGINE_RELATIVE_INSTALL_DIR="${GZ_RENDERING_ENGINE_RELATIVE_INSTALL_DIR}"
)

target_link_libraries(${PROJECT_LIBRARY_TARGET_NAME}
  PUBLIC
  gz-common${GZ_COMMON_VER}::requested
  gz-utils${GZ_UTILS_VER}::gz-utils${GZ_UTILS_VER}
  PRIVATE
  gz-plugin${GZ_PLUGIN_VER}::loader
)

# Build the unit tests.
gz_build_tests(TYPE UNIT
               SOURCES ${gtest_sources}
               ENVIRONMENT GZ_RENDERING_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX})
