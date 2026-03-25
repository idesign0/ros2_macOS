# add_subdirectory(plugins)

set (sources
  InstallationDirectories.cc
  Manager.cc
)

set (gtest_sources
  Manager_TEST.cc
)

# Create the library target.
gz_create_core_library(SOURCES ${sources})
gz_add_get_install_prefix_impl(GET_INSTALL_PREFIX_FUNCTION gz::launch::getInstallPrefix
                               GET_INSTALL_PREFIX_HEADER gz/launch/InstallationDirectories.hh
                               OVERRIDE_INSTALL_PREFIX_ENV_VARIABLE GZ_LAUNCH_INSTALL_PREFIX)

set_property(
  SOURCE InstallationDirectories.cc
  PROPERTY COMPILE_DEFINITIONS
  GZ_LAUNCH_PLUGIN_RELATIVE_INSTALL_PATH="${GZ_LAUNCH_PLUGIN_RELATIVE_INSTALL_PATH}"
  GZ_LAUNCH_INITIAL_CONFIG_RELATIVE_PATH="${GZ_DATA_INSTALL_DIR}/configs"
)

# Link the libraries that we always need.
target_link_libraries(${PROJECT_LIBRARY_TARGET_NAME}
  PUBLIC
    gz-plugin${GZ_PLUGIN_VER}::loader
    gz-plugin${GZ_PLUGIN_VER}::register
    TINYXML2::TINYXML2
  PRIVATE
    gz-common${GZ_COMMON_VER}::gz-common${GZ_COMMON_VER}
    gz-math${GZ_MATH_VER}::gz-math${GZ_MATH_VER}
    ${BACKWARD_LIBRARIES}
)

target_compile_definitions(${PROJECT_LIBRARY_TARGET_NAME} PRIVATE ${BACKWARD_DEFINITIONS})

# Build the unit tests.
gz_build_tests(TYPE UNIT SOURCES ${gtest_sources}
                EXCLUDE_PROJECT_LIB
                LIB_DEPS
                  ${PROJECT_LIBRARY_TARGET_NAME}
                  gz-common${GZ_COMMON_VER}::gz-common${GZ_COMMON_VER}
                ENVIRONMENT
                  GZ_LAUNCH_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
)

add_subdirectory(cmd)
