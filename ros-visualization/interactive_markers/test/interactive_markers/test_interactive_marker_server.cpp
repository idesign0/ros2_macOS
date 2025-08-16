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

#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "interactive_marker_fixtures.hpp"
#include "mock_interactive_marker_client.hpp"
#include "timed_expect.hpp"

#include "interactive_markers/interactive_marker_server.hpp"

TEST(TestInteractiveMarkerServer, construction_and_destruction)
{
  rclcpp::init(0, nullptr);
  auto node = std::make_shared<rclcpp::Node>("test_construction_and_destruction_node", "");
  {
    interactive_markers::InteractiveMarkerServer server("", node);
  }
  {
    interactive_markers::InteractiveMarkerServer server("test_node_ptr_ctor_server", node);
  }
  {
    interactive_markers::InteractiveMarkerServer server(
      "test_node_ptr_ctor_server", node, rclcpp::QoS(42));
  }
  {
    interactive_markers::InteractiveMarkerServer server(
      "test_node_ptr_ctor_server", node, rclcpp::QoS(1), rclcpp::QoS(99));
  }
  {
    interactive_markers::InteractiveMarkerServer server(
      "test_node_interfaces_ctor_server",
      node->get_node_base_interface(),
      node->get_node_clock_interface(),
      node->get_node_logging_interface(),
      node->get_node_topics_interface(),
      node->get_node_services_interface());
  }
  {
    interactive_markers::InteractiveMarkerServer server(
      "test_node_interfaces_ctor_server",
      node->get_node_base_interface(),
      node->get_node_clock_interface(),
      node->get_node_logging_interface(),
      node->get_node_topics_interface(),
      node->get_node_services_interface(),
      rclcpp::QoS(1),
      rclcpp::QoS(100));
  }

  rclcpp::shutdown();
}

TEST(TestInteractiveMarkerServer, insert)
{
  rclcpp::init(0, nullptr);
  auto node = std::make_shared<rclcpp::Node>("test_insert_node", "");
  interactive_markers::InteractiveMarkerServer server("test_insert_server", node);

  // Insert some markers
  std::vector<visualization_msgs::msg::InteractiveMarker> markers = get_interactive_markers();
  for (const auto & marker : markers) {
    server.insert(marker);
  }

  // Expect empty until 'applyChanges' is called
  EXPECT_TRUE(server.empty());
  EXPECT_EQ(server.size(), 0u);
  server.applyChanges();
  EXPECT_FALSE(server.empty());
  EXPECT_EQ(server.size(), markers.size());

  rclcpp::shutdown();
}

class TestInteractiveMarkerServerWithMarkers : public ::testing::Test
{
protected:
  static void SetUpTestCase()
  {
    rclcpp::init(0, nullptr);
  }

  static void TearDownTestCase()
  {
    rclcpp::shutdown();
  }

  void SetUp()
  {
    const std::string topic_namespace = "test_namespace";
    node_ = std::make_shared<rclcpp::Node>("test_interactive_marker_server_node", "");

    server_ = std::make_unique<interactive_markers::InteractiveMarkerServer>(
      topic_namespace, node_);

    // Insert some markers
    markers_ = get_interactive_markers();
    for (const auto & marker : markers_) {
      server_->insert(marker);
    }
    server_->applyChanges();

    mock_client_ = std::make_shared<MockInteractiveMarkerClient>(topic_namespace);

    executor_.add_node(node_);
    executor_.add_node(mock_client_);

    // Wait for discovery (or timeout)
    ASSERT_TRUE(mock_client_->client_->wait_for_service(std::chrono::seconds(3)));
    const auto start_time = std::chrono::system_clock::now();
    while (
      mock_client_->publisher_->get_subscription_count() == 0u &&
      (std::chrono::system_clock::now() - start_time) < std::chrono::seconds(3))
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    ASSERT_EQ(mock_client_->publisher_->get_subscription_count(), 1u);
    // TODO(jacobperron): We should probably also wait for the server to discover the client
    //                    to avoid flakes. This requires additional interactive marker server API.
  }

  void TearDown()
  {
    mock_client_.reset();
    server_.reset();
    node_.reset();
  }

  rclcpp::executors::SingleThreadedExecutor executor_;
  rclcpp::Node::SharedPtr node_;
  std::unique_ptr<interactive_markers::InteractiveMarkerServer> server_;
  std::shared_ptr<MockInteractiveMarkerClient> mock_client_;
  std::vector<visualization_msgs::msg::InteractiveMarker> markers_;
};  // class TestInteractiveMarkerServer

TEST_F(TestInteractiveMarkerServerWithMarkers, erase)
{
  // Erase a marker
  EXPECT_TRUE(server_->erase(markers_[0].name));
  EXPECT_EQ(server_->size(), markers_.size());
  server_->applyChanges();
  EXPECT_EQ(server_->size(), markers_.size() - 1u);

  // Erase a marker that has just been inserted
  server_->insert(markers_[0]);
  EXPECT_TRUE(server_->erase(markers_[0].name));
  EXPECT_EQ(server_->size(), markers_.size() - 1u);
  server_->applyChanges();
  EXPECT_EQ(server_->size(), markers_.size() - 1u);

  // Erase an invalid marker
  EXPECT_FALSE(server_->erase("this_Is_the_name_0f_a_marker_that_doesn't ex1st"));
  server_->applyChanges();
  EXPECT_EQ(server_->size(), markers_.size() - 1u);
}

TEST_F(TestInteractiveMarkerServerWithMarkers, clear)
{
  // Clear all markers
  server_->clear();
  EXPECT_EQ(server_->size(), markers_.size());
  server_->applyChanges();
  ASSERT_EQ(server_->size(), 0u);

  // Clear an empty server
  server_->clear();
  EXPECT_EQ(server_->size(), 0u);
}

TEST_F(TestInteractiveMarkerServerWithMarkers, get_marker_by_name)
{
  // Get markers
  for (const auto & input_marker : markers_) {
    visualization_msgs::msg::InteractiveMarker output_marker;
    EXPECT_TRUE(server_->get(input_marker.name, output_marker));
    EXPECT_EQ(output_marker.header.frame_id, input_marker.header.frame_id);
    EXPECT_EQ(output_marker.pose.position.x, input_marker.pose.position.x);
    EXPECT_EQ(output_marker.pose.orientation.w, input_marker.pose.orientation.w);
    EXPECT_EQ(output_marker.name, input_marker.name);
    EXPECT_EQ(output_marker.description, input_marker.description);
    ASSERT_EQ(output_marker.menu_entries.size(), input_marker.menu_entries.size());
    for (std::size_t i = 0u; i < output_marker.menu_entries.size(); ++i) {
      EXPECT_EQ(output_marker.menu_entries[i].id, input_marker.menu_entries[i].id);
      EXPECT_EQ(output_marker.menu_entries[i].title, input_marker.menu_entries[i].title);
      EXPECT_EQ(output_marker.menu_entries[i].command, input_marker.menu_entries[i].command);
    }
    ASSERT_EQ(output_marker.controls.size(), input_marker.controls.size());
    for (std::size_t i = 0u; i < output_marker.controls.size(); ++i) {
      EXPECT_EQ(output_marker.controls[i].name, input_marker.controls[i].name);
      EXPECT_EQ(output_marker.controls[i].always_visible, input_marker.controls[i].always_visible);
    }
  }

  // Get an invalid marker
  {
    visualization_msgs::msg::InteractiveMarker output_marker;
    EXPECT_FALSE(server_->get("n0t_the_name_of_@ marker", output_marker));
  }

  // Get a pending erased marker
  {
    EXPECT_TRUE(server_->erase(markers_[0].name));
    visualization_msgs::msg::InteractiveMarker output_marker;
    EXPECT_FALSE(server_->get(markers_[0].name, output_marker));
  }
}

TEST_F(TestInteractiveMarkerServerWithMarkers, set_pose)
{
  // Set a pose
  {
    geometry_msgs::msg::Pose pose;
    pose.position.x = 1.0;
    pose.position.y = -2.0;
    pose.position.z = 3.14;
    pose.orientation.w = 0.5;
    EXPECT_TRUE(server_->setPose(markers_[0].name, pose));
    server_->applyChanges();
    visualization_msgs::msg::InteractiveMarker output_marker;
    EXPECT_TRUE(server_->get(markers_[0].name, output_marker));
    EXPECT_EQ(output_marker.pose.position.x, pose.position.x);
    EXPECT_EQ(output_marker.pose.position.y, pose.position.y);
    EXPECT_EQ(output_marker.pose.position.z, pose.position.z);
    EXPECT_EQ(output_marker.pose.orientation.w, pose.orientation.w);
    EXPECT_EQ(output_marker.header.frame_id, markers_[0].header.frame_id);
  }

  // Set a pose with header update
  {
    geometry_msgs::msg::Pose pose;
    pose.position.x = 1.0;
    pose.position.y = -2.0;
    pose.position.z = 3.14;
    pose.orientation.w = 0.5;
    std_msgs::msg::Header header;
    header.frame_id = "test_updating_to_a_new_header";
    EXPECT_TRUE(server_->setPose(markers_[0].name, pose, header));
    server_->applyChanges();
    visualization_msgs::msg::InteractiveMarker output_marker;
    EXPECT_TRUE(server_->get(markers_[0].name, output_marker));
    EXPECT_EQ(output_marker.pose.position.x, pose.position.x);
    EXPECT_EQ(output_marker.pose.position.y, pose.position.y);
    EXPECT_EQ(output_marker.pose.position.z, pose.position.z);
    EXPECT_EQ(output_marker.pose.orientation.w, pose.orientation.w);
    EXPECT_NE(output_marker.header.frame_id, markers_[0].header.frame_id);
    EXPECT_EQ(output_marker.header.frame_id, header.frame_id);
  }

  // Set pose of invalid marker
  {
    geometry_msgs::msg::Pose pose;
    pose.orientation.w = 1.0;
    EXPECT_FALSE(server_->setPose("test_n0t_a_valid_marker_n@me", pose));
  }
}

TEST_F(TestInteractiveMarkerServerWithMarkers, set_callback)
{
  EXPECT_TRUE(server_->setCallback(markers_[0].name, nullptr));
  EXPECT_FALSE(server_->setCallback("test_n0t_a_valid_marker_n@me", nullptr));
}

TEST_F(TestInteractiveMarkerServerWithMarkers, feedback_communication)
{
  using namespace std::chrono_literals;

  // Register a callback function to capture output
  visualization_msgs::msg::InteractiveMarkerFeedback output_feedback;
  auto callback =
    [&output_feedback]
      (interactive_markers::InteractiveMarkerServer::FeedbackConstSharedPtr feedback)
    {
      output_feedback = *feedback;
    };
  EXPECT_TRUE(server_->setCallback(markers_[0].name, callback));
  server_->applyChanges();

  // Populate and publish mock feedback
  visualization_msgs::msg::InteractiveMarkerFeedback feedback;
  feedback.client_id = "test_client_id";
  feedback.marker_name = markers_[0].name;
  feedback.event_type = feedback.POSE_UPDATE;
  feedback.pose.position.x = -3.14;
  feedback.pose.orientation.w = 1.0;
  mock_client_->publishFeedback(feedback);
  TIMED_EXPECT_EQ(output_feedback.client_id, feedback.client_id, 3s, 10ms, executor_);
  EXPECT_EQ(output_feedback.marker_name, markers_[0].name);
  EXPECT_EQ(output_feedback.pose.position.x, feedback.pose.position.x);
  EXPECT_EQ(output_feedback.pose.orientation.w, feedback.pose.orientation.w);
}

TEST_F(TestInteractiveMarkerServerWithMarkers, update_communication)
{
  using namespace std::chrono_literals;

  ASSERT_EQ(mock_client_->updates_received, 0u);
  // This should not trigger an update publication
  server_->applyChanges();
  // Adding a marker should trigger an update
  visualization_msgs::msg::InteractiveMarker marker;
  marker.name = "test_update_from_added_marker";
  server_->insert(marker);
  server_->applyChanges();
  TIMED_EXPECT_EQ(mock_client_->updates_received, 1u, 3s, 10ms, executor_);
  ASSERT_NE(mock_client_->last_update_message, nullptr);
  EXPECT_EQ(mock_client_->last_update_message->markers.size(), 1u);
  EXPECT_EQ(mock_client_->last_update_message->poses.size(), 0u);
  EXPECT_EQ(mock_client_->last_update_message->erases.size(), 0u);
  // Modifying a marker should trigger an update
  geometry_msgs::msg::Pose pose;
  pose.orientation.w = 1.0;
  server_->setPose(
    markers_[0].name,
    pose);
  server_->applyChanges();
  TIMED_EXPECT_EQ(mock_client_->updates_received, 2u, 3s, 10ms, executor_);
  EXPECT_EQ(mock_client_->last_update_message->markers.size(), 0u);
  EXPECT_EQ(mock_client_->last_update_message->poses.size(), 1u);
  EXPECT_EQ(mock_client_->last_update_message->erases.size(), 0u);
  // Erasing a marker should trigger an update
  ASSERT_TRUE(server_->erase(markers_[0].name));
  server_->applyChanges();
  TIMED_EXPECT_EQ(mock_client_->updates_received, 3u, 3s, 10ms, executor_);
  EXPECT_EQ(mock_client_->last_update_message->markers.size(), 0u);
  EXPECT_EQ(mock_client_->last_update_message->poses.size(), 0u);
  ASSERT_EQ(mock_client_->last_update_message->erases.size(), 1u);
  EXPECT_EQ(mock_client_->last_update_message->erases[0], markers_[0].name);
}

TEST_F(TestInteractiveMarkerServerWithMarkers, get_interactive_markers_communication)
{
  using namespace std::chrono_literals;

  auto future = mock_client_->requestInteractiveMarkers();
  auto ret = executor_.spin_until_future_complete(future, 3000ms);
  ASSERT_EQ(ret, rclcpp::FutureReturnCode::SUCCESS);
  visualization_msgs::srv::GetInteractiveMarkers::Response::SharedPtr response = future.get();
  ASSERT_EQ(response->markers.size(), markers_.size());
  for (std::size_t i = 0u; i < markers_.size(); ++i) {
    EXPECT_EQ(response->markers[i].header.frame_id, markers_[i].header.frame_id);
    EXPECT_EQ(response->markers[i].pose.position.x, markers_[i].pose.position.x);
    EXPECT_EQ(response->markers[i].pose.orientation.w, markers_[i].pose.orientation.w);
    EXPECT_EQ(response->markers[i].name, markers_[i].name);
    EXPECT_EQ(response->markers[i].description, markers_[i].description);
    ASSERT_EQ(response->markers[i].menu_entries.size(), markers_[i].menu_entries.size());
    for (std::size_t j = 0u; j < markers_[i].menu_entries.size(); ++j) {
      EXPECT_EQ(response->markers[i].menu_entries[j].id, markers_[i].menu_entries[j].id);
      EXPECT_EQ(response->markers[i].menu_entries[j].title, markers_[i].menu_entries[j].title);
      EXPECT_EQ(response->markers[i].menu_entries[j].command, markers_[i].menu_entries[j].command);
    }
    ASSERT_EQ(response->markers[i].controls.size(), markers_[i].controls.size());
    for (std::size_t j = 0u; j < markers_[i].controls.size(); ++j) {
      EXPECT_EQ(response->markers[i].controls[j].name, markers_[i].controls[j].name);
      EXPECT_EQ(
        response->markers[i].controls[j].always_visible, markers_[i].controls[j].always_visible);
    }
  }
}
