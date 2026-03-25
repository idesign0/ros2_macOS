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

#include "bag2_to_image/bag2_to_image.hpp"

#include "rosbag2_cpp/converter_interfaces/serialization_format_converter.hpp"
#include "rosbag2_cpp/converter_options.hpp"
#include "rosbag2_cpp/typesupport_helpers.hpp"
#include "rosbag2_storage/storage_options.hpp"

#include <opencv2/opencv.hpp>

#include "sensor_msgs/image_encodings.hpp"
#include "sensor_msgs/msg/compressed_image.hpp"
#include "sensor_msgs/msg/image.hpp"

#include <filesystem>
#include <memory>
#include <string>

namespace
{
int encoding2mat_type(const std::string & encoding)
{
  if (encoding == "mono8") {
    return CV_8UC1;
  } else if (encoding == "bgr8") {
    return CV_8UC3;
  } else if (encoding == "mono16") {
    return CV_16SC1;
  } else if (encoding == "rgba8") {
    return CV_8UC4;
  } else if (encoding == "bgra8") {
    return CV_8UC4;
  } else if (encoding == "32FC1") {
    return CV_32FC1;
  } else if (encoding == "rgb8") {
    return CV_8UC3;
  } else if (encoding == "yuv422") {
    return CV_8UC2;
  } else {
    throw std::runtime_error("Unsupported encoding type");
  }
}
}  // namespace

namespace bag2_to_image
{
namespace enc = sensor_msgs::image_encodings;
namespace fs = std::filesystem;

Bag2ToImageNode::Bag2ToImageNode(const rclcpp::NodeOptions & options)
: rclcpp::Node("bag2_to_image", options)
{
  reader_ = std::make_unique<rosbag2_cpp::readers::SequentialReader>();
  rosbag2_storage::StorageOptions storage_options{};
  storage_options.uri = declare_parameter<std::string>("uri");
  storage_options.storage_id = declare_parameter("storage_id", "sqlite3");
  rosbag2_cpp::ConverterOptions converter_options{};
  const auto serialization_format = declare_parameter("serialization_format", "cdr");
  converter_options.input_serialization_format = serialization_format;
  converter_options.output_serialization_format = serialization_format;
  const auto mode = declare_parameter("imdecode_flag", "unchanged");
  if (mode == "unchanged") {
    imdecode_flag_ = cv::IMREAD_UNCHANGED;
  } else if (mode == "gray") {
    imdecode_flag_ = cv::IMREAD_GRAYSCALE;
  } else if (mode == "color") {
    imdecode_flag_ = cv::IMREAD_COLOR;
  } else {
    RCLCPP_ERROR(get_logger(), "Unknown mode: %s, defaulting to 'unchanged", mode.c_str());
    imdecode_flag_ = cv::IMREAD_UNCHANGED;
  }
  factory_ = std::make_unique<rosbag2_cpp::SerializationFormatConverterFactory>();
  deserializer_ = factory_->load_deserializer(serialization_format);
  const fs::path image_save_directory{declare_parameter<std::string>("image_save_directory")};
  if (!fs::is_directory(image_save_directory)) {
    RCLCPP_ERROR_STREAM(get_logger(), image_save_directory << " is not a directory.");
    rclcpp::shutdown();
    return;
  }

  reader_->open(std::move(storage_options), std::move(converter_options));
  const auto topics = reader_->get_all_topics_and_types();

  const auto image_topic = declare_parameter<std::string>("image_topic");
  const auto result = std::find_if(topics.begin(), topics.end(), [&image_topic](const auto & t) {
    return (
      t.name == image_topic &&
      (t.type == "sensor_msgs/msg/Image" || t.type == "sensor_msgs/msg/CompressedImage"));
  });

  if (result == topics.end()) {
    RCLCPP_ERROR_STREAM(get_logger(), "Cannot find image topic: " << image_topic);
    rclcpp::shutdown();
    return;
  }

  rosbag2_storage::StorageFilter filter{};
  filter.topics.emplace_back(image_topic);
  reader_->set_filter(std::move(filter));

  while (reader_->has_next()) {
    auto serialized_message = reader_->read_next();
    auto ros_message = std::make_shared<rosbag2_cpp::rosbag2_introspection_message_t>();
    ros_message->time_stamp = 0;
    ros_message->message = nullptr;
    ros_message->allocator = rcutils_get_default_allocator();
    if (result->type == "sensor_msgs/msg/Image") {
      auto image = std::make_unique<sensor_msgs::msg::Image>();
      ros_message->message = image.get();
      auto library = rosbag2_cpp::get_typesupport_library(result->type, "rosidl_typesupport_cpp");
      auto type_support =
        rosbag2_cpp::get_typesupport_handle(result->type, "rosidl_typesupport_cpp", library);
      deserializer_->deserialize(serialized_message, type_support, ros_message);
      const cv::Mat mat{
        static_cast<int>(image->height), static_cast<int>(image->width),
        encoding2mat_type(image->encoding), image->data.data(), image->step};
      fs::path save_file =
        image_save_directory / fs::path(
                                 std::to_string(image->header.stamp.sec) + "_" +
                                 std::to_string(image->header.stamp.nanosec) + ".jpg");
      cv::imwrite(save_file.string(), mat);
    } else if (result->type == "sensor_msgs/msg/CompressedImage") {
      auto image = std::make_unique<sensor_msgs::msg::CompressedImage>();
      ros_message->message = image.get();
      auto library = rosbag2_cpp::get_typesupport_library(result->type, "rosidl_typesupport_cpp");
      auto type_support =
        rosbag2_cpp::get_typesupport_handle(result->type, "rosidl_typesupport_cpp", library);
      deserializer_->deserialize(serialized_message, type_support, ros_message);
      try {
        auto mat = cv::imdecode(cv::Mat(image->data), imdecode_flag_);

        const size_t split_pos = image->format.find(';');
        std::string image_encoding = image->format.substr(0, split_pos);

        if (enc::isColor(image_encoding)) {
          std::string compressed_encoding = image->format.substr(split_pos);
          bool compressed_bgr_image =
            (compressed_encoding.find("compressed bgr") != std::string::npos);

          if (!compressed_bgr_image) {
            // if necessary convert colors from rgb to bgr
            if ((image_encoding == enc::BGR8) || (image_encoding == enc::BGR16))
              cv::cvtColor(mat, mat, cv::COLOR_RGB2BGR);

            if ((image_encoding == enc::BGRA8) || (image_encoding == enc::BGRA16))
              cv::cvtColor(mat, mat, cv::COLOR_RGB2BGRA);

            if ((image_encoding == enc::RGBA8) || (image_encoding == enc::RGBA16))
              cv::cvtColor(mat, mat, cv::COLOR_RGB2RGBA);
          }
        }
        if (
          image->format.find("jpeg") != std::string::npos && enc::bitDepth(image_encoding) == 16) {
          mat.convertTo(mat, CV_16U, 256);
        }
        fs::path save_file =
          image_save_directory / fs::path(
                                   std::to_string(image->header.stamp.sec) + "_" +
                                   std::to_string(image->header.stamp.nanosec) + ".jpg");
        cv::imwrite(save_file.string(), mat);
      } catch (cv::Exception & e) {
        RCLCPP_ERROR(get_logger(), "%s", e.what());
      }
    }
  }
  rclcpp::shutdown();
}
}  // namespace bag2_to_image

#include "rclcpp_components/register_node_macro.hpp"

RCLCPP_COMPONENTS_REGISTER_NODE(bag2_to_image::Bag2ToImageNode)
