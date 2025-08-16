// Copyright (c) 2019, Open Source Robotics Foundation, Inc.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
//    * Neither the name of the copyright holder nor the names of its
//      contributors may be used to endorse or promote products derived from
//      this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#ifndef INTERACTIVE_MARKERS__MOCK_INTERACTIVE_MARKER_CLIENT_HPP_
#define INTERACTIVE_MARKERS__MOCK_INTERACTIVE_MARKER_CLIENT_HPP_
#include <memory>
#include <string>

#include "geometry_msgs/msg/pose.hpp"
#include "rclcpp/rclcpp.hpp"
#include "visualization_msgs/msg/interactive_marker.hpp"
#include "visualization_msgs/msg/interactive_marker_feedback.hpp"
#include "visualization_msgs/msg/interactive_marker_pose.hpp"
#include "visualization_msgs/msg/interactive_marker_update.hpp"
#include "visualization_msgs/srv/get_interactive_markers.hpp"

class MockInteractiveMarkerClient : public rclcpp::Node
{
public:
  using SharedFuture =
    rclcpp::Client<visualization_msgs::srv::GetInteractiveMarkers>::SharedFuture;
  using SharedResponse =
    rclcpp::Client<visualization_msgs::srv::GetInteractiveMarkers>::SharedResponse;

  MockInteractiveMarkerClient(
    const std::string topic_namespace = "mock_interactive_markers",
    const rclcpp::NodeOptions & options = rclcpp::NodeOptions());

  void publishFeedback(const visualization_msgs::msg::InteractiveMarkerFeedback & feedback);
  auto requestInteractiveMarkers();

  uint32_t updates_received;
  visualization_msgs::msg::InteractiveMarkerUpdate::ConstSharedPtr last_update_message;
  visualization_msgs::srv::GetInteractiveMarkers::Response::SharedPtr last_response_message;

  std::string topic_namespace_;
  rclcpp::Client<visualization_msgs::srv::GetInteractiveMarkers>::SharedPtr client_;
  rclcpp::Publisher<visualization_msgs::msg::InteractiveMarkerFeedback>::SharedPtr publisher_;
  rclcpp::Subscription<visualization_msgs::msg::InteractiveMarkerUpdate>::SharedPtr subscription_;
};  // class MockInteractiveMarkerClient

MockInteractiveMarkerClient::MockInteractiveMarkerClient(
  const std::string topic_namespace,
  const rclcpp::NodeOptions & options)
: rclcpp::Node("mock_interactive_marker_client", options),
  updates_received(0),
  topic_namespace_(topic_namespace)
{
  client_ = create_client<visualization_msgs::srv::GetInteractiveMarkers>(
    topic_namespace_ + "/get_interactive_markers");

  publisher_ = create_publisher<visualization_msgs::msg::InteractiveMarkerFeedback>(
    topic_namespace_ + "/feedback",
    1);

  subscription_ = create_subscription<visualization_msgs::msg::InteractiveMarkerUpdate>(
    topic_namespace_ + "/update",
    1,
    [this](const visualization_msgs::msg::InteractiveMarkerUpdate::ConstSharedPtr update)
    {
      RCLCPP_INFO(this->get_logger(), "Update received");
      ++this->updates_received;
      last_update_message = update;
    });
}

void MockInteractiveMarkerClient::publishFeedback(
  const visualization_msgs::msg::InteractiveMarkerFeedback & feedback)
{
  publisher_->publish(feedback);
}

auto
MockInteractiveMarkerClient::requestInteractiveMarkers()
{
  if (!client_->wait_for_service(std::chrono::seconds(2))) {
    RCLCPP_WARN(get_logger(), "Reqested interactive markers, but service is not available");
  }
  return client_->async_send_request(
    std::make_shared<visualization_msgs::srv::GetInteractiveMarkers::Request>());
}
#endif  // INTERACTIVE_MARKERS__MOCK_INTERACTIVE_MARKER_CLIENT_HPP_
