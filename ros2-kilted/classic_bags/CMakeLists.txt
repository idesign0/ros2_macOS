cmake_minimum_required(VERSION 3.5)
project(classic_bags)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

find_package(ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rosbag2_cpp REQUIRED)
find_package(rosbag2_storage REQUIRED)

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  find_package(std_msgs REQUIRED)

  ament_add_gtest(${PROJECT_NAME}_basic_test test/test_basic.cpp)
  target_include_directories(${PROJECT_NAME}_basic_test PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  )
  ament_target_dependencies(${PROJECT_NAME}_basic_test
    rosbag2_cpp
    rosbag2_storage
    std_msgs
  )

  find_package(ament_cmake_pytest REQUIRED)
  ament_add_pytest_test(utest test/utest.py)
endif()

install(DIRECTORY include/
        DESTINATION include
)

ament_python_install_package(${PROJECT_NAME})
ament_export_include_directories(include)

ament_package()
