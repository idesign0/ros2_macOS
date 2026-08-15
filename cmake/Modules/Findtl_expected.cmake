# Bridges find_package(tl_expected) (underscore, the name used in ROS
# package.xml <depend> tags and the corresponding CMakeLists.txt calls) to
# Homebrew's tl-expected formula, which installs its CMake config as
# tl-expected-config.cmake (hyphen) -> CMake's default find_package search
# for "tl_expected" never matches it.
#
# tl-expected is header-only; it exports the interface target tl::expected.
# Consumers in this tree typically do target_link_libraries(... ${tl_expected_LIBRARIES}).

find_package(tl-expected CONFIG QUIET)

if(tl-expected_FOUND)
  set(tl_expected_FOUND TRUE)
  set(tl_expected_LIBRARIES tl::expected)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(tl_expected DEFAULT_MSG tl_expected_FOUND)
