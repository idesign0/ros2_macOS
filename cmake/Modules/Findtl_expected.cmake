# Bridges find_package(tl_expected) (underscore, the name used in ROS
# package.xml <depend> tags and the corresponding CMakeLists.txt calls) to
# Homebrew's tl-expected formula, which installs its CMake config as
# tl-expected-config.cmake (hyphen) -> CMake's default find_package search
# for "tl_expected" never matches it.
#
# tl-expected is header-only; it exports the interface target tl::expected.
# Two consumer patterns exist in this tree, so we satisfy BOTH:
#   1. target_link_libraries(... ${tl_expected_LIBRARIES})       — the variable
#   2. target_link_libraries(... tl_expected::tl_expected)        — the target
#      (generate_parameter_library's generated cmake links this namespaced target
#      directly; without it, its ~13 ros2_control consumers fail at
#      generate_parameter_library.cmake:92 "target ... not found").

find_package(tl-expected CONFIG QUIET)

if(tl-expected_FOUND)
  set(tl_expected_FOUND TRUE)
  # Provide the namespaced target brew doesn't (it only exports tl::expected)...
  if(NOT TARGET tl_expected::tl_expected)
    add_library(tl_expected::tl_expected INTERFACE IMPORTED)
    set_target_properties(tl_expected::tl_expected PROPERTIES
      INTERFACE_LINK_LIBRARIES tl::expected)
  endif()
  # ...and route the variable through that same target (single source of truth).
  set(tl_expected_LIBRARIES tl_expected::tl_expected)
  # Several ROS sources `#include <tl_expected/expected.hpp>` (underscore
  # dir, matching the PickNickRobotics/cpp_polyfills vendored package layout
  # some distros use instead of this brew bridge) but brew's tl-expected
  # installs the header at `tl/expected.hpp` -> add a tiny redirect shim on
  # the target's include path so both spellings resolve. Harmless no-op for
  # any consumer that only uses `tl/expected.hpp` directly.
  set_property(TARGET tl_expected::tl_expected APPEND PROPERTY
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/tl_expected_shim")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(tl_expected DEFAULT_MSG tl_expected_FOUND)
