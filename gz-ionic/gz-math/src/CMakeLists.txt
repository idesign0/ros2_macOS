
# Collect source files into the "sources" variable and unit test files into the
# "gtest_sources" variable
gz_get_libsources_and_unittests(sources gtest_sources)

# Create the library target
gz_create_core_library(SOURCES ${sources} CXX_STANDARD ${CMAKE_CXX_STANDARD})

target_link_libraries(${PROJECT_LIBRARY_TARGET_NAME}
  PUBLIC
    gz-utils${GZ_UTILS_VER}::gz-utils${GZ_UTILS_VER}
)

# Build the unit tests
gz_build_tests(TYPE UNIT SOURCES ${gtest_sources})

# graph namespace
add_subdirectory(graph)

# Bindings subdirectories
if (NOT SKIP_PYBIND11)
  if (Python3_Development_FOUND)
    add_subdirectory(python_pybind11)
  else()
    message(WARNING "Python development libraries are missing: Python interfaces are disabled.")
  endif()
endif()

if (SWIG_FOUND AND NOT SKIP_SWIG)
  add_subdirectory(ruby)
endif()
