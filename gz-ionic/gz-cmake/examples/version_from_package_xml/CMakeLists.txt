cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)

find_package(gz-cmake4 REQUIRED)
gz_get_package_xml_version(${CMAKE_SOURCE_DIR}/package.xml PACKAGE_XML)

message(STATUS "PACKAGE_XML_VERSION ${PACKAGE_XML_VERSION}")
message(STATUS "PACKAGE_XML_VERSION_MAJOR ${PACKAGE_XML_VERSION_MAJOR}")
message(STATUS "PACKAGE_XML_VERSION_MINOR ${PACKAGE_XML_VERSION_MINOR}")
message(STATUS "PACKAGE_XML_VERSION_PATCH ${PACKAGE_XML_VERSION_PATCH}")

project(gz-version_from_package_xml VERSION ${PACKAGE_XML_VERSION})
gz_configure_project()
gz_configure_build(QUIT_IF_BUILD_ERRORS)
