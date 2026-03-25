cmake_minimum_required(VERSION 3.10.2)
project(async_web_server_cpp VERSION 2.0.0 LANGUAGES CXX)

find_package(ament_cmake_ros REQUIRED)

## System dependencies are found with CMake's conventions
find_package(Boost REQUIRED COMPONENTS filesystem regex thread)
find_package(OpenSSL REQUIRED)

###########
## Build ##
###########

add_library(${PROJECT_NAME}
    src/http_connection.cpp
    src/http_reply.cpp
    src/http_request.cpp
    src/http_request_handler.cpp
    src/http_request_parser.cpp
    src/http_server.cpp
    src/websocket_connection.cpp
    src/websocket_message.cpp
    src/websocket_request_handler.cpp
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Boost::boost Boost::filesystem Boost::thread
    PRIVATE
        Boost::regex
        OpenSSL::Crypto
)

target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        $<$<CXX_COMPILER_ID:MSVC>:-D_WIN32_WINNT=0x0A00>
)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_11)

# Increase SOVERSION with each breaking ABI change
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

###########
# Testing #
###########

if(BUILD_TESTING)
    add_subdirectory(test)
endif()

###################################################
## Declare things to be passed to other projects ##
###################################################

ament_export_targets(${PROJECT_NAME}Targets HAS_LIBRARY_TARGET)
ament_package(CONFIG_EXTRAS find_dependencies.cmake.in)

#############
## Install ##
#############

## Mark executables and/or libraries for installation
install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Targets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

## Mark cpp header files for installation
install(DIRECTORY include/${PROJECT_NAME}/
  DESTINATION include/${PROJECT_NAME}
  FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h"
)
