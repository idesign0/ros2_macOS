# This CMakeLists.txt only include the lines needed to set up this test
# in your own custom ROS 2 package. Other necessary CMake commands
# (such as the boilerplate ROS 2 items added when using ros2 pkg create)
# are omitted.

# THIS LINE SHOULD BE UNCOMMENTED - it is only commented here
# so this file can be included in the top level CMakeLists.txt
# find_package(catch_ros2 REQUIRED) # UNCOMMENT

if(BUILD_TESTING)
  include(CTest)

  ###############
  ## UNIT TEST ##
  ###############
  add_executable(example_unit_test
    examples/unit_test/example_unit_test.cpp
  )
  # Linking a unit test executable with catch_ros2::catch_ros2_with_main
  # provides the default Catch2 main function to the executable.
  target_link_libraries(example_unit_test
    catch_ros2::catch_ros2_with_main
  )
  add_test(NAME ExampleUnitTest COMMAND example_unit_test)
endif()