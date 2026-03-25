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

/// @file A dummy ROS 2 node that provides a service for the test node
/// to check.

#include "rclcpp/rclcpp.hpp"
#include "std_srvs/srv/empty.hpp"

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  auto node = rclcpp::Node::make_shared("integration_aux_node");
  auto service = node->create_service<std_srvs::srv::Empty>(
    "test_service",
    [](
      const std_srvs::srv::Empty::Request::SharedPtr,
      std_srvs::srv::Empty::Response::SharedPtr
    ) {}
  );
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
