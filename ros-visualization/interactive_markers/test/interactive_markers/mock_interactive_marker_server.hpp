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

#ifndef INTERACTIVE_MARKERS__MOCK_INTERACTIVE_MARKER_SERVER_HPP_
#define INTERACTIVE_MARKERS__MOCK_INTERACTIVE_MARKER_SERVER_HPP_
#include <memory>
#include <string>
#include <vector>

#include "geometry_msgs/msg/pose.hpp"
#include "rclcpp/rclcpp.hpp"
#include "visualization_msgs/msg/interactive_marker.hpp"
#include "visualization_msgs/msg/interactive_marker_feedback.hpp"
#include "visualization_msgs/msg/interactive_marker_pose.hpp"
#include "visualization_msgs/msg/interactive_marker_update.hpp"
#include "visualization_msgs/srv/get_interactive_markers.hpp"

#include "interactive_marker_fixtures.hpp"

class MockInteractiveMarkerServer : public rclcpp::Node
{
public:
  MockInteractiveMarkerServer(
    const std::string topic_namespace = "mock_interactive_markers",
    const rclcpp::NodeOptions & options = rclcpp::NodeOptions());

  ~MockInteractiveMarkerServer();

  void publishUpdate(const geometry_msgs::msg::Pose & pose = geometry_msgs::msg::Pose());

  uint64_t requests_received_;
  uint64_t feedback_received_;

  std::string topic_namespace_;
  uint64_t sequence_number_;
  std::vector<visualization_msgs::msg::InteractiveMarker> markers_;
  rclcpp::Service<visualization_msgs::srv::GetInteractiveMarkers>::SharedPtr service_;
  rclcpp::Subscription<visualization_msgs::msg::InteractiveMarkerFeedback>::SharedPtr
    subscription_;
  rclcpp::Publisher<visualization_msgs::msg::InteractiveMarkerUpdate>::SharedPtr publisher_;
};  // class MockInteractiveMarkerServer

MockInteractiveMarkerServer::MockInteractiveMarkerServer(
  const std::string topic_namespace,
  const rclcpp::NodeOptions & options)
: rclcpp::Node("mock_interactive_marker_server", options),
  requests_received_(0),
  feedback_received_(0),
  topic_namespace_(topic_namespace),
  sequence_number_(0)
{
  markers_ = get_interactive_markers();

  service_ = create_service<visualization_msgs::srv::GetInteractiveMarkers>(
    topic_namespace_ + "/get_interactive_markers",
    [this](const std::shared_ptr<rmw_request_id_t> request_header,
    const std::shared_ptr<visualization_msgs::srv::GetInteractiveMarkers::Request> request,
    std::shared_ptr<visualization_msgs::srv::GetInteractiveMarkers::Response> response)
    {
      (void)request_header;
      (void)request;
      RCLCPP_INFO(this->get_logger(), "Interactive markers request received");
      response->sequence_number = this->sequence_number_;
      response->markers = this->markers_;
      ++this->requests_received_;
    });

  subscription_ = create_subscription<visualization_msgs::msg::InteractiveMarkerFeedback>(
    topic_namespace_ + "/feedback",
    10,
    [this](const visualization_msgs::msg::InteractiveMarkerFeedback::ConstSharedPtr feedback)
    {
      (void)feedback;
      RCLCPP_INFO(this->get_logger(), "Feedback received");
      ++this->feedback_received_;
    });

  publisher_ = create_publisher<visualization_msgs::msg::InteractiveMarkerUpdate>(
    topic_namespace_ + "/update",
    10);
  RCLCPP_INFO(get_logger(), "Mock server ready");
}

MockInteractiveMarkerServer::~MockInteractiveMarkerServer()
{
  RCLCPP_INFO(get_logger(), "Mock server destroyed");
}

void MockInteractiveMarkerServer::publishUpdate(const geometry_msgs::msg::Pose & pose)
{
  visualization_msgs::msg::InteractiveMarkerUpdate update;
  update.seq_num = sequence_number_++;
  update.type = update.UPDATE;
  markers_[0].pose = pose;
  visualization_msgs::msg::InteractiveMarkerPose marker_pose;
  marker_pose.pose = markers_[0].pose;
  marker_pose.header = markers_[0].header;
  marker_pose.name = markers_[0].name;
  update.poses.push_back(marker_pose);

  publisher_->publish(update);
}
#endif  // INTERACTIVE_MARKERS__MOCK_INTERACTIVE_MARKER_SERVER_HPP_
