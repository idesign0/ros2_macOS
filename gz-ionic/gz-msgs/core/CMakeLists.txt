##################################################
# Build a custom protoc plugin
set(PROTOC_PLUGIN ${PROJECT_NAME}_protoc_plugin)

gz_add_executable(${PROTOC_PLUGIN}
  generator/Generator.cc
  generator/generator_main.cc)
target_link_libraries(${PROTOC_PLUGIN}
  protobuf::libprotoc
  protobuf::libprotobuf)
target_include_directories(${PROTOC_PLUGIN} PRIVATE ${PROTOBUF_INCLUDE_DIR})
target_compile_features(${PROTOC_PLUGIN} PRIVATE ${GZ_CXX_11_FEATURES})

if (UNIX)
  target_link_libraries(${PROTOC_PLUGIN} pthread)
endif()

install(TARGETS ${PROTOC_PLUGIN} DESTINATION ${GZ_BIN_INSTALL_DIR})

##################################################
# Generate messages
file(GLOB proto_files ${PROJECT_SOURCE_DIR}/proto/gz/msgs/*.proto)

include(${PROJECT_SOURCE_DIR}/cmake/gz_msgs_factory.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/gz_msgs_generate.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/gz_msgs_protoc.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/gz_msgs_string_utils.cmake)

if(TARGET ${${PROJECT_NAME}_PROTO_GENERATOR_PLUGIN})
  set(${PROJECT_NAME}_PROTO_GENERATOR_PLUGIN_FILE $<TARGET_FILE:${${PROJECT_NAME}_PROTO_GENERATOR_PLUGIN}>)
else()
  set(${PROJECT_NAME}_PROTO_GENERATOR_PLUGIN_FILE ${${PROJECT_NAME}_PROTO_GENERATOR_PLUGIN})
endif()

gz_msgs_generate_messages_impl(
  MSGS_GEN_SCRIPT
    ${PROJECT_SOURCE_DIR}/tools/gz_msgs_generate.py
  FACTORY_GEN_SCRIPT
    ${PROJECT_SOURCE_DIR}/tools/gz_msgs_generate_factory.py
  PYTHON_INTERPRETER
    ${${PROJECT_NAME}_PYTHON_INTERPRETER}
  PROTOC_EXEC
    ${${PROJECT_NAME}_PROTOC_EXECUTABLE}
  GZ_PROTOC_PLUGIN
    ${${PROJECT_NAME}_PROTO_GENERATOR_PLUGIN_FILE}
  INPUT_PROTOS
    ${proto_files}
  DLLEXPORT_DECL
    "GZ_MSGS_VISIBLE"
  PROTO_PACKAGE
    "gz.msgs"
  PROTO_PATH
    ${PROJECT_SOURCE_DIR}/proto
  OUTPUT_DIRECTORY
    ${PROJECT_BINARY_DIR}/gz_msgs_gen
  OUTPUT_SOURCES
    msgs_sources
  OUTPUT_HEADERS
    msgs_headers
  OUTPUT_DETAIL_HEADERS
    msgs_detail_headers
  OUTPUT_PYTHON
    msgs_python
)

gz_msgs_generate_desc_impl(
  PROTOC_EXEC ${${PROJECT_NAME}_PROTOC_EXECUTABLE}
  INPUT_PROTOS ${proto_files}
  PROTO_PATH ${PROJECT_SOURCE_DIR}/proto
  DEPENDENCY_DESCRIPTIONS ${depends_msgs_desc}
  OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/gz_msgs_gen
  OUTPUT_FILENAME gz-msgs${PROJECT_VERSION_MAJOR}.gz_desc
)

install(FILES ${msgs_headers} DESTINATION ${GZ_INCLUDE_INSTALL_DIR_FULL}/gz/msgs)
install(FILES ${msgs_detail_headers} DESTINATION ${GZ_INCLUDE_INSTALL_DIR_FULL}/gz/msgs/details)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/gz-msgs${PROJECT_VERSION_MAJOR}.gz_desc
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/gz/protos/)

if (NOT GZ_PYTHON_INSTALL_PATH)
  find_package(Python3 REQUIRED COMPONENTS Interpreter)
  if(USE_SYSTEM_PATHS_FOR_PYTHON_INSTALLATION)
    if(USE_DIST_PACKAGES_FOR_PYTHON)
      string(REPLACE "site-packages" "dist-packages" GZ_PYTHON_INSTALL_PATH ${Python3_SITELIB})
    else()
      # Python3_SITELIB might use dist-packages in some platforms
      string(REPLACE "dist-packages" "site-packages" GZ_PYTHON_INSTALL_PATH ${Python3_SITELIB})
    endif()
  else()
    # If not a system installation, respect local paths
    set(GZ_PYTHON_INSTALL_PATH ${GZ_LIB_INSTALL_DIR}/python)
  endif()
endif()
install(FILES ${msgs_python} DESTINATION ${GZ_PYTHON_INSTALL_PATH}/gz/msgs${PROJECT_VERSION_MAJOR})

##################################################
# Build core library
gz_get_libsources_and_unittests(sources gtest_sources)

gz_create_core_library(SOURCES
  src/Factory.cc
  src/MessageFactory.cc
  src/DynamicFactory.cc
  ${msgs_sources}
  gz-msgs${PROJECT_VERSION_MAJOR}.gz_desc
)

target_include_directories(${PROJECT_LIBRARY_TARGET_NAME} PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/gz_msgs_gen>")

set_target_properties(
  ${PROJECT_LIBRARY_TARGET_NAME}
    PROPERTIES
      SOVERSION ${PROJECT_VERSION_MAJOR}
      VERSION ${PROJECT_VERSION_FULL}
      GZ_MSGS_DESC_FILE "\$\{_IMPORT_PREFIX\}/share/gz/protos/gz-msgs${PROJECT_VERSION_MAJOR}.gz_desc"
      )
set_property(TARGET ${PROJECT_LIBRARY_TARGET_NAME} PROPERTY EXPORT_PROPERTIES "GZ_MSGS_DESC_FILE")

include(GzRelocatableBinaries)
gz_add_get_install_prefix_impl(GET_INSTALL_PREFIX_FUNCTION gz::msgs::getInstallPrefix
                               GET_INSTALL_PREFIX_HEADER gz/msgs/InstallationDirectories.hh
                               OVERRIDE_INSTALL_PREFIX_ENV_VARIABLE GZ_MSGS_INSTALL_PREFIX)

target_link_libraries(${PROJECT_LIBRARY_TARGET_NAME}
  PUBLIC
    gz-math${GZ_MATH_VER}::gz-math${GZ_MATH_VER}
    TINYXML2::TINYXML2
    protobuf::libprotobuf
)
target_include_directories(${PROJECT_LIBRARY_TARGET_NAME}
  SYSTEM PUBLIC $<TARGET_PROPERTY:protobuf::libprotobuf,INTERFACE_INCLUDE_DIRECTORIES>)

add_subdirectory(include/gz/msgs)
add_subdirectory(cmd)
