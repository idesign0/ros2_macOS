# Collect source files into the "sources" variable and unit test files into the
# "tests" variable
gz_get_libsources_and_unittests(sources tests)

# Create the library target
gz_create_core_library(
  SOURCES ${sources}
)

target_link_libraries(${PROJECT_LIBRARY_TARGET_NAME}
  PUBLIC gz-utils${GZ_UTILS_VER}::gz-utils${GZ_UTILS_VER})

# Build the unit tests
gz_build_tests(
  TYPE UNIT
  SOURCES ${tests})

add_subdirectory(include/gz/plugin)
