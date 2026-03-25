// Copyright 2023 Nick Morales.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/// @file - Example unit tests using catch_ros2 utilities.

#include "catch_ros2/catch_ros2.hpp"
#include "rclcpp/rclcpp.hpp"

using catch_ros2::SimulateArgs;

TEST_CASE("parameters", "[parameters]") {
  // The SimulateArgs class can be used to synthesize input arguments for rclcpp::init()
  const auto args = SimulateArgs{"--ros-args -p param1:=-1.4 -p param3:=14"};

  // Initialize ROS with simulated arguments
  rclcpp::init(args.argc(), args.argv());

  // Init test node
  auto node = rclcpp::Node::make_shared("test_node");

  // Test that parameters are received as expected by the node
  node->declare_parameter<double>("param1");
  node->declare_parameter<double>("param2", 3.45);
  node->declare_parameter<int>("param3", 2);


  // Assertions
  // Should be set to simulated parameter and not throw
  CHECK(node->get_parameter("param1").get_parameter_value().get<double>() == -1.4);

  // Should be set to default value
  CHECK(node->get_parameter("param2").get_parameter_value().get<double>() == 3.45);

  // Should be set to simulated parameter, not default value
  CHECK(node->get_parameter("param3").get_parameter_value().get<int>() == 14);

  // Should throw since it was never initialized
  CHECK_THROWS(node->declare_parameter<bool>("param4"));

  // Shutdown ROS
  rclcpp::shutdown();
}

TEST_CASE("complex parameters", "[parameters]") {
  // The SimulateArgs class can also be used with more complex parameters included quotation marks
  const auto args = SimulateArgs{
    "--ros-args -p param1:=1 "
    "-p param2:=\"[2.0, 3.0, 4.0]\" -p param3:=\"/path/with spaces/\""
  };

  // Initialize ROS with simulated arguments
  rclcpp::init(args.argc(), args.argv());

  // Init test node
  auto node = rclcpp::Node::make_shared("test_node");

  // Test that parameters are received as expected by the node
  node->declare_parameter<int>("param1");
  node->declare_parameter<std::vector<double>>("param2");
  node->declare_parameter<std::string>("param3", "default value");


  // Assertions
  // Should be set to simulated parameter and not throw
  CHECK(node->get_parameter("param1").get_parameter_value().get<int>() == 1);
  // Should be properly read as a list
  const auto param2 =
    node->get_parameter("param2").get_parameter_value().get<std::vector<double>>();
  CHECK(param2.at(0) == 2.0);
  CHECK(param2.at(1) == 3.0);
  CHECK(param2.at(2) == 4.0);

  // Should be set to simulated parameter, not default value
  CHECK(
    node->get_parameter(
      "param3").get_parameter_value().get<std::string>() == "/path/with spaces/");

  // Shutdown ROS
  rclcpp::shutdown();
}
