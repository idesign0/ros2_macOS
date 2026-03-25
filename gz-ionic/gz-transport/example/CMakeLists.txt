cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)

project(gz-transport-examples)

# Find the Gazebo Transport library
find_package(gz-transport14 QUIET REQUIRED OPTIONAL_COMPONENTS log)
set(GZ_TRANSPORT_VER ${gz-transport14_VERSION_MAJOR})

if (EXISTS "${CMAKE_SOURCE_DIR}/msgs/")
  # Message generation. Only required when using custom Protobuf messages.
  find_package(Protobuf REQUIRED)
  add_subdirectory(msgs)
  set_source_files_properties(${PROTO_SRC} ${PROTO_HEADER} PROPERTIES GENERATED TRUE)
endif()

include_directories(${CMAKE_BINARY_DIR})

# Generate examples
if (EXISTS "${CMAKE_SOURCE_DIR}/bench.cc")
  ########################################
  # GFlags
  find_library(gflags_LIBRARIES NAMES gflags)
  find_path(gflags_INCLUDE_DIRS gflags/gflags.h ENV CPATH)
  if (NOT gflags_LIBRARIES OR NOT gflags_INCLUDE_DIRS)
    message (STATUS "Unable to compile 'bench.cc', missing GFLags library")
  else()
    include_directories(SYSTEM ${gflags_INCLUDE_DIRS})
    add_executable(bench bench.cc)
    target_link_libraries(bench
      gz-transport${GZ_TRANSPORT_VER}::core
      ${gflags_LIBRARIES}
      pthread)
  endif()
endif()

if (NOT MSVC AND EXISTS "${CMAKE_SOURCE_DIR}/publisher_c.cc")
  add_executable(publisher_c publisher_c.cc)
  target_link_libraries(publisher_c gz-transport${GZ_TRANSPORT_VER}::core)
endif()
if (NOT MSVC AND EXISTS "${CMAKE_SOURCE_DIR}/publisher_c_fast.cc")
  add_executable(publisher_c_fast publisher_c_fast.cc)
  target_link_libraries(publisher_c_fast gz-transport${GZ_TRANSPORT_VER}::core)
endif()

if (EXISTS "${CMAKE_SOURCE_DIR}/subscriber_c.cc")
  add_executable(subscriber_c subscriber_c.cc)
  target_link_libraries(subscriber_c gz-transport${GZ_TRANSPORT_VER}::core)
endif()

if (NOT MSVC AND EXISTS "${CMAKE_SOURCE_DIR}/subscriber_c_slow.cc")
  add_executable(subscriber_c_slow subscriber_c_slow.cc)
  target_link_libraries(subscriber_c_slow gz-transport${GZ_TRANSPORT_VER}::core)
endif()

if (EXISTS "${CMAKE_SOURCE_DIR}/subscriber_stats.cc")
  add_executable(subscriber_stats subscriber_stats.cc)
  target_link_libraries(subscriber_stats gz-transport${GZ_TRANSPORT_VER}::core)
endif()

if (EXISTS "${CMAKE_SOURCE_DIR}/publisher.cc")
  add_executable(publisher publisher.cc)
  target_link_libraries(publisher gz-transport${GZ_TRANSPORT_VER}::core)
endif()

if (EXISTS "${CMAKE_SOURCE_DIR}/subscriber.cc")
  add_executable(subscriber subscriber.cc)
  target_link_libraries(subscriber gz-transport${GZ_TRANSPORT_VER}::core)
endif()

if (EXISTS "${CMAKE_SOURCE_DIR}/subscriber_generic.cc")
  add_executable(subscriber_generic subscriber_generic.cc)
  target_link_libraries(subscriber_generic
    gz-transport${GZ_TRANSPORT_VER}::core)
endif()

if (EXISTS "${CMAKE_SOURCE_DIR}/responser.cc")
  add_executable(responser responser.cc)
  target_link_libraries(responser gz-transport${GZ_TRANSPORT_VER}::core)
endif()

if (EXISTS "${CMAKE_SOURCE_DIR}/requester.cc")
  add_executable(requester requester.cc)
  target_link_libraries(requester gz-transport${GZ_TRANSPORT_VER}::core)
endif()

if (EXISTS "${CMAKE_SOURCE_DIR}/requester_async.cc")
  add_executable(requester_async requester_async.cc)
  target_link_libraries(requester_async
    gz-transport${GZ_TRANSPORT_VER}::core)
endif()

if (EXISTS "${CMAKE_SOURCE_DIR}/responser_oneway.cc")
  add_executable(responser_oneway responser_oneway.cc)
  target_link_libraries(responser_oneway
    gz-transport${GZ_TRANSPORT_VER}::core)
endif()

if (EXISTS "${CMAKE_SOURCE_DIR}/requester_oneway.cc")
  add_executable(requester_oneway requester_oneway.cc)
  target_link_libraries(requester_oneway
    gz-transport${GZ_TRANSPORT_VER}::core)
endif()

if (EXISTS "${CMAKE_SOURCE_DIR}/requester_raw.cc")
  add_executable(requester_raw requester_raw.cc)
  target_link_libraries(requester_raw
    gz-transport${GZ_TRANSPORT_VER}::core)
endif()

if (EXISTS "${CMAKE_SOURCE_DIR}/responser_no_input.cc")
  add_executable(responser_no_input responser_no_input.cc)
  target_link_libraries(responser_no_input
    gz-transport${GZ_TRANSPORT_VER}::core)
endif()

if (EXISTS "${CMAKE_SOURCE_DIR}/requester_no_input.cc")
  add_executable(requester_no_input requester_no_input.cc)
  target_link_libraries(requester_no_input
    gz-transport${GZ_TRANSPORT_VER}::core)
endif()

if (EXISTS "${CMAKE_SOURCE_DIR}/requester_async_no_input.cc")
  add_executable(requester_async_no_input requester_async_no_input.cc)
  target_link_libraries(requester_async_no_input
    gz-transport${GZ_TRANSPORT_VER}::core)
endif()

# FIXME(chapulina) Linking failure on Windows:
# msgs\stringmsg.pb.cc : fatal error LNK1107: invalid or corrupt file: cannot read at 0x3028
if (NOT MSVC AND TARGET protobuf_compilation AND
    EXISTS "${CMAKE_SOURCE_DIR}/publisher_custom_msg.cc")
  add_executable(publisher_custom_msg publisher_custom_msg.cc)
  target_link_libraries(publisher_custom_msg
    gz-transport${GZ_TRANSPORT_VER}::core
    ${PROTO_SRC}
  )
  add_dependencies(publisher_custom_msg protobuf_compilation)
endif()

# FIXME(chapulina) Linking failure on Windows:
# msgs\stringmsg.pb.cc : fatal error LNK1107: invalid or corrupt file: cannot read at 0x3028
if (NOT MSVC AND TARGET protobuf_compilation AND
    EXISTS "${CMAKE_SOURCE_DIR}/subscriber_custom_msg.cc")
  add_executable(subscriber_custom_msg subscriber_custom_msg.cc)
  target_link_libraries(subscriber_custom_msg
    gz-transport${GZ_TRANSPORT_VER}::core
    ${PROTO_SRC}
  )
  add_dependencies(subscriber_custom_msg protobuf_compilation)
endif()

if (EXISTS "${CMAKE_SOURCE_DIR}/publisher_raw.cc")
  add_executable(publisher_raw publisher_raw.cc)
  target_link_libraries(publisher_raw
    gz-transport${GZ_TRANSPORT_VER}::core
  )
endif()

if (EXISTS "${CMAKE_SOURCE_DIR}/subscriber_raw.cc")
  add_executable(subscriber_raw subscriber_raw.cc)
  target_link_libraries(subscriber_raw
    gz-transport${GZ_TRANSPORT_VER}::core
  )
endif()

# Logging examples.
if (TARGET gz-transport${GZ_TRANSPORT_VER}::log)

  if (EXISTS "${CMAKE_SOURCE_DIR}/record.cc")
    add_executable(log_record record.cc)
    target_link_libraries(log_record
      gz-transport${GZ_TRANSPORT_VER}::log
    )
  endif()

  if (EXISTS "${CMAKE_SOURCE_DIR}/playback.cc")
    add_executable(log_playback playback.cc)
    target_link_libraries(log_playback
      gz-transport${GZ_TRANSPORT_VER}::log
    )
  endif()

  if (EXISTS "${CMAKE_SOURCE_DIR}/custom_query.cc")
    add_executable(log_custom_query custom_query.cc)
    target_link_libraries(log_custom_query
      gz-transport${GZ_TRANSPORT_VER}::log
    )
  endif()

endif()


if (MSVC)
  # Suppress Protobuf message generation warnings.
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4018 /wd4100 /wd4127 /wd4244 /wd4267 /wd4512")

  # Suppress the "decorated name length exceed" warning (inside the STL).
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4503")

  add_definitions(-DBUILDING_STATIC_LIBS -DWIN32_LEAN_AND_MEAN)

  # Don't pull in the Windows min/max macros
  add_definitions(-DNOMINMAX)

endif()
