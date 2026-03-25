# This CMakeLists.txt only include the lines needed to set up this test
# in your own custom ROS 2 package. Other necessary CMake commands
# (such as the boilerplate ROS 2 items added when using ros2 pkg create)
# are omitted.

# THIS LINE SHOULD BE UNCOMMENTED - it is only commented here
# so this file can be included in the top level CMakeLists.txt
# find_package(catch_ros2 REQUIRED) # UNCOMMENT
find_package(rclcpp REQUIRED)
find_package(std_srvs REQUIRED)


##########################
## INTEGRATION AUX NODE ##
##########################
# This could be any node in your ROS 2 package
# Likely you are looking to test some functionality of this node
add_executable(integration_aux_node
  examples/integration_test/integration_aux_node.cpp
)
target_link_libraries(integration_aux_node PUBLIC
  ${std_srvs_TARGETS}
  rclcpp::rclcpp
)

install(TARGETS
  integration_aux_node
  DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  include(CTest)

  ###########################
  ## INTEGRATION TEST NODE ##
  ###########################
  # This is the node in which integration tests occur
  add_executable(integration_test_node
    examples/integration_test/integration_test_node.cpp
  )
  # The link libraries call links this node with catch_ros2::catch_ros2_with_node_main
  # to get the default integration test node main function
  target_link_libraries(integration_test_node
    catch_ros2::catch_ros2_with_node_main
    ${std_srvs_TARGETS}
    rclcpp::rclcpp
)
  install(TARGETS
    integration_test_node
    DESTINATION lib/${PROJECT_NAME}
  )

  #################
  ## LAUNCH FILE ##
  #################
  # This is the launch file that will be used to run the integration test
  install(FILES
    examples/integration_test/example_integration_test.launch.py  # example in Python
    examples/integration_test/example_integration_test.launch.xml  # example in XML
    examples/integration_test/example_integration_test.launch.yaml  # example in YAML
    DESTINATION share/${PROJECT_NAME}
  )

  ######################
  ## INTEGRATION TEST ##
  ######################
  # This CMake function allows the integration test to be run
  # when running "colcon test".
  catch_ros2_add_integration_test(ExampleIntegration_TestPython
    LAUNCH_FILE example_integration_test.launch.py  # example in Python
  )
  catch_ros2_add_integration_test(ExampleIntegration_TestXML
    LAUNCH_FILE example_integration_test.launch.xml  # example in XML
  )
  catch_ros2_add_integration_test(ExampleIntegration_TestYAML
    LAUNCH_FILE example_integration_test.launch.xml  # example in YAML
  )

endif()
