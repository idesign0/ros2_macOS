// -*-c++-*---------------------------------------------------------------------------------------
// Copyright 2024 Bernd Pfrommer <bernd.pfrommer@gmail.com>
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

#ifndef APRILTAG_DRAW__APRILTAG_DRAW_HPP_
#define APRILTAG_DRAW__APRILTAG_DRAW_HPP_

#include <apriltag_msgs/msg/april_tag_detection_array.hpp>
#include <deque>
#include <image_transport/image_transport.hpp>
#include <memory>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <string>

namespace apriltag_draw
{
using ApriltagArray = apriltag_msgs::msg::AprilTagDetectionArray;
using Image = sensor_msgs::msg::Image;
class ApriltagDraw : public rclcpp::Node
{
public:
  explicit ApriltagDraw(const rclcpp::NodeOptions & options);
  ~ApriltagDraw();

  bool isSubscribed() const { return (is_subscribed_); }

private:
  void processFrame(
    const ApriltagArray::ConstSharedPtr & tags,
    const Image::ConstSharedPtr & img);
  void imageCallback(const Image::ConstSharedPtr & img);
  void tagCallback(const ApriltagArray::ConstSharedPtr & tags);
  void processBuffers();
  void subscriptionCheckTimerExpired();
  void subscribe();
  void unsubscribe();
  // ------------------------  variables ------------------------------
  rclcpp::TimerBase::SharedPtr subscription_check_timer_;
  rclcpp::Subscription<ApriltagArray>::SharedPtr tag_sub_;
  std::shared_ptr<image_transport::Subscriber> image_sub_;
  std::deque<ApriltagArray::ConstSharedPtr> tag_buffer_;
  std::deque<Image::ConstSharedPtr> img_buffer_;
  image_transport::Publisher image_pub_;
  rmw_qos_profile_t qos_profile_ = rmw_qos_profile_default;
  std::string transport_{"raw"};
  bool is_subscribed_{false};
  int max_queue_size_{200};
};
}  // namespace apriltag_draw
#endif  // APRILTAG_DRAW__APRILTAG_DRAW_HPP_
