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

/// @file
/// @brief default node main, which initializes ROS and passes ROS arguments to the node
/// without passing them to the Catch session. On exit, it shuts down ROS.

#include "catch_ros2/catch_ros2.hpp"
#include "rclcpp/rclcpp.hpp"

int main(int argc, char * argv[])
{
  // Split input ROS args from args that will be input to Catch
  const auto split = catch_ros2::SplitROSArgs{argc, argv};

  // Init ROS
  rclcpp::init(split.argc(), split.argv());

  // Run Catch session
  int result = Catch::Session().run(split.argc_without_ros(), split.argv_without_ros() );

  // Shutdown ROS
  rclcpp::shutdown();

  return result;
}
