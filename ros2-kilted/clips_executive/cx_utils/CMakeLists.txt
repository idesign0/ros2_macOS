cmake_minimum_required(VERSION 3.16...3.28)
project(cx_utils)

# Default to C++17
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if($ENV{ROS_DISTRO} MATCHES "humble")
  add_compile_definitions(HUMBLE)
elseif($ENV{ROS_DISTRO} MATCHES "jazzy")
  add_compile_definitions(JAZZY)
elseif($ENV{ROS_DISTRO} MATCHES "kilted")
  add_compile_definitions(KILTED)
elseif($ENV{ROS_DISTRO} MATCHES "rolling")
  add_compile_definitions(ROLLING)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(spdlog_vendor REQUIRED)
find_package(spdlog REQUIRED)
find_package(clips_vendor REQUIRED)
find_package(clips REQUIRED)
find_package(ament_index_cpp REQUIRED)

set(dependencies
  ament_cmake
  rclcpp
  spdlog
  ament_index_cpp
)

include_directories(include)

set(CX_UTILS_SOURCES
  src/clips_env_context.cpp
  src/param_utils.cpp
)

add_library(${PROJECT_NAME} SHARED ${CX_UTILS_SOURCES})
ament_target_dependencies(${PROJECT_NAME} PUBLIC ${dependencies})
target_link_libraries(${PROJECT_NAME} PUBLIC spdlog::spdlog ClipsNS::libclips_ns)

include(CheckCXXSourceCompiles)

set(CMAKE_REQUIRED_FLAGS "-std=c++20")

check_cxx_source_compiles("
  #include <format>
  #include <string>
  int main() {
      auto s = std::format(\"Hello {}\", 42);
      return s.size();
  }
" HAVE_STD_FORMAT)

if(HAVE_STD_FORMAT)
    message(STATUS "std::format is supported")
else()
    message(STATUS "std::format is NOT supported, checking for bundled fmt")
    include(CheckIncludeFileCXX)
    check_include_file_cxx("spdlog/fmt/fmt.h" SPDLOG_HAS_BUNDLED_FMT CMAKE_REQUIRED_LIBRARIES spdlog)
    if(SPDLOG_HAS_BUNDLED_FMT)
      message(Status "spdlog has bundled fmt, using that")
    else()
        message(Status "spdlog has not bundled fmt, looking for external fmt lib")
        find_package(fmt REQUIRED)
        target_compile_definitions(${PROJECT_NAME} PUBLIC SPDLOG_FMT_EXTERNAL=1)
        ament_export_definitions(SPDLOG_FMT_EXTERNAL)
        ament_target_dependencies(${PROJECT_NAME} PUBLIC fmt)
    endif()
endif()

install(
  DIRECTORY include/
  DESTINATION include/
)

install(TARGETS
  ${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()
ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})
ament_export_dependencies(${dependencies})

ament_package()
