# This CMakeLists.txt only include the lines needed to set up this test
# in your own custom ROS 2 package. Other necessary CMake commands
# (such as the boilerplate ROS 2 items added when using ros2 pkg create)
# are omitted.

# THIS LINE SHOULD BE UNCOMMENTED - it is only commented here
# so this file can be included in the top level CMakeLists.txt
# find_package(catch_ros2 REQUIRED) # UNCOMMENT

if(BUILD_TESTING)
  include(CTest)

  ###########################
  ## TEST WITH CUSTOM MAIN ##
  ###########################
  add_executable(example_test_with_custom_main
    examples/custom_main/example_test_with_custom_main.cpp
  )
  # Linking a test executable with catch_ros2::catch_ros2
  # provides no main function. The user must supply their own.
  target_link_libraries(example_test_with_custom_main
    catch_ros2::catch_ros2
  )
  add_test(NAME ExampleTestWithCustomMain COMMAND example_test_with_custom_main)
endif()