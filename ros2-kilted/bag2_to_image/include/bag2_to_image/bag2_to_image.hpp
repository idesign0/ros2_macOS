// Copyright 2022 Daisuke Nishimatsu
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

#ifndef BAG2_TO_IMAGE__BAG2_TO_IMAGE_HPP_
#define BAG2_TO_IMAGE__BAG2_TO_IMAGE_HPP_

#include "rclcpp/rclcpp.hpp"
#include "rosbag2_cpp/readers/sequential_reader.hpp"

#include <memory>

namespace bag2_to_image
{
class Bag2ToImageNode : public rclcpp::Node
{
public:
  explicit Bag2ToImageNode(const rclcpp::NodeOptions & options);

private:
  std::unique_ptr<rosbag2_cpp::readers::SequentialReader> reader_;
  std::unique_ptr<rosbag2_cpp::SerializationFormatConverterFactory> factory_;
  std::unique_ptr<rosbag2_cpp::converter_interfaces::SerializationFormatDeserializer> deserializer_;
  int imdecode_flag_;
};
}  // namespace bag2_to_image

#endif  // BAG2_TO_IMAGE__BAG2_TO_IMAGE_HPP_
