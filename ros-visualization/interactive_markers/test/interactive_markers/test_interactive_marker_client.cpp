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
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "tf2/buffer_core.h"

#include "interactive_marker_fixtures.hpp"
#include "mock_interactive_marker_server.hpp"
#include "timed_expect.hpp"

#include "interactive_markers/interactive_marker_client.hpp"

using ClientState = interactive_markers::InteractiveMarkerClient::State;

TEST(TestInteractiveMarkerClientInitialize, construction_destruction)
{
  rclcpp::init(0, nullptr);
  auto node = std::make_shared<rclcpp::Node>("test_interactive_marker_client_node", "");
  auto buffer = std::make_shared<tf2::BufferCore>();

  {
    interactive_markers::InteractiveMarkerClient client(node, buffer);
  }
  {
    interactive_markers::InteractiveMarkerClient client(
      node,
      buffer,
      "test_frame_id",
      "test_namespace",
      std::chrono::seconds(3),
      rclcpp::QoS(1),
      rclcpp::QoS(100));
  }
  {
    interactive_markers::InteractiveMarkerClient client(
      node->get_node_base_interface(),
      node->get_node_topics_interface(),
      node->get_node_services_interface(),
      node->get_node_graph_interface(),
      node->get_node_clock_interface(),
      node->get_node_logging_interface(),
      buffer);
  }
  {
    interactive_markers::InteractiveMarkerClient client(
      node->get_node_base_interface(),
      node->get_node_topics_interface(),
      node->get_node_services_interface(),
      node->get_node_graph_interface(),
      node->get_node_clock_interface(),
      node->get_node_logging_interface(),
      buffer,
      "test_frame_id",
      "test_namespace",
      std::chrono::milliseconds(100),
      rclcpp::QoS(42),
      rclcpp::QoS(1));
  }
  {
    // Invalid namespace (shouldn't crash)
    interactive_markers::InteractiveMarkerClient client(
      node, buffer, "test_frame_id", "th!s//is/aninvalid_namespace?");
  }

  rclcpp::shutdown();
}

class TestInteractiveMarkerClient : public ::testing::Test
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
    target_frame_id_ = "test_target_frame_id";
    topic_namespace_ = "test_namespace";
    executor_ = std::make_unique<rclcpp::executors::SingleThreadedExecutor>();
    node_ = std::make_shared<rclcpp::Node>("test_interactive_marker_server_node", "");
    buffer_ = std::make_shared<tf2::BufferCore>();
    client_ = std::make_unique<interactive_markers::InteractiveMarkerClient>(
      node_, buffer_, target_frame_id_, topic_namespace_);
    executor_->add_node(node_);
  }

  void TearDown()
  {
    client_.reset();
    buffer_.reset();
    node_.reset();
    executor_.reset();
  }

  void makeTfDataAvailable(const std::vector<visualization_msgs::msg::InteractiveMarker> & markers)
  {
    geometry_msgs::msg::TransformStamped transform_stamped;
    transform_stamped.header.frame_id = target_frame_id_;
    transform_stamped.header.stamp = builtin_interfaces::msg::Time(node_->now());
    transform_stamped.transform.rotation.w = 1.0;
    for (std::size_t i = 0u; i < markers.size(); ++i) {
      transform_stamped.child_frame_id = markers[0].header.frame_id;
      ASSERT_TRUE(buffer_->setTransform(transform_stamped, "mock_tf_authority"));
    }
  }

  /// Wait for client and server to discover each other or die trying
  void waitForDiscovery(
    std::shared_ptr<MockInteractiveMarkerServer> server,
    std::chrono::seconds timeout = std::chrono::seconds(3))
  {
    const auto start_time = std::chrono::system_clock::now();
    while (
      (server->publisher_->get_subscription_count() == 0u ||
      server->subscription_->get_publisher_count() == 0u) &&
      (std::chrono::system_clock::now() - start_time) < timeout)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    ASSERT_GT(server->publisher_->get_subscription_count(), 0u);
    ASSERT_GT(server->subscription_->get_publisher_count(), 0u);
    // TODO(jacobperron): We should probably also wait for the client to discover the server
    //                    to avoid flakes. This requires additional interactive marker client API.
  }

  std::string target_frame_id_;
  std::string topic_namespace_;
  std::unique_ptr<rclcpp::executors::SingleThreadedExecutor> executor_;
  rclcpp::Node::SharedPtr node_;
  std::shared_ptr<tf2::BufferCore> buffer_;
  std::unique_ptr<interactive_markers::InteractiveMarkerClient> client_;
};  // class TestInteractiveMarkerClient

TEST_F(TestInteractiveMarkerClient, states)
{
  using namespace std::chrono_literals;

  // IDLE -> IDLE
  {
    client_.reset(
      new interactive_markers::InteractiveMarkerClient(
        node_, buffer_, target_frame_id_, topic_namespace_));
    EXPECT_EQ(client_->getState(), ClientState::STATE_IDLE);
    client_->update();
    EXPECT_EQ(client_->getState(), ClientState::STATE_IDLE);
  }

  // IDLE -> INITIALIZE -> IDLE -> INITIALIZE
  {
    client_.reset(
      new interactive_markers::InteractiveMarkerClient(
        node_, buffer_, target_frame_id_, topic_namespace_));
    client_->update();
    EXPECT_EQ(client_->getState(), ClientState::STATE_IDLE);
    // Start server
    auto mock_server = std::make_shared<MockInteractiveMarkerServer>(topic_namespace_);
    executor_->add_node(mock_server);
    // Repeatedly call the client's update method until the server is detected
    auto update_func = std::bind(
      &interactive_markers::InteractiveMarkerClient::update, client_.get());
    TIMED_EXPECT_EQ(
      client_->getState(), ClientState::STATE_INITIALIZE, 3s, 10ms, (*executor_), update_func);
    // Disconnect server
    executor_->remove_node(mock_server);
    mock_server.reset();
    TIMED_EXPECT_EQ(
      client_->getState(), ClientState::STATE_IDLE, 3s, 10ms, (*executor_), update_func);
    // Re-start server
    mock_server.reset(new MockInteractiveMarkerServer(topic_namespace_));
    executor_->add_node(mock_server);
    TIMED_EXPECT_EQ(
      client_->getState(), ClientState::STATE_INITIALIZE, 3s, 10ms, (*executor_), update_func);
    executor_->remove_node(mock_server);
  }

  // IDLE -> INITIALIZE -> RUNNING -> IDLE
  {
    client_.reset(
      new interactive_markers::InteractiveMarkerClient(
        node_, buffer_, target_frame_id_, topic_namespace_));
    EXPECT_EQ(client_->getState(), ClientState::STATE_IDLE);
    // Start server
    auto mock_server = std::make_shared<MockInteractiveMarkerServer>(topic_namespace_);
    executor_->add_node(mock_server);
    // Repeatedly call the client's update method until the server is detected
    auto update_func = std::bind(
      &interactive_markers::InteractiveMarkerClient::update, client_.get());
    TIMED_EXPECT_EQ(
      client_->getState(), ClientState::STATE_INITIALIZE, 3s, 10ms, (*executor_), update_func);
    // Make the required TF data in order to finish initializing
    makeTfDataAvailable(mock_server->markers_);
    TIMED_EXPECT_EQ(
      client_->getState(), ClientState::STATE_RUNNING, 3s, 10ms, (*executor_), update_func);
    // Disconnect server
    executor_->remove_node(mock_server);
    mock_server.reset();
    TIMED_EXPECT_EQ(
      client_->getState(), ClientState::STATE_IDLE, 3s, 10ms, (*executor_), update_func);
  }

  // IDLE -> INITIALIZE -> RUNNING -> INITIALIZE -> IDLE
  {
    client_.reset(
      new interactive_markers::InteractiveMarkerClient(
        node_, buffer_, target_frame_id_, topic_namespace_));
    EXPECT_EQ(client_->getState(), ClientState::STATE_IDLE);
    // Start server
    auto mock_server = std::make_shared<MockInteractiveMarkerServer>(topic_namespace_);
    executor_->add_node(mock_server);
    waitForDiscovery(mock_server);
    // Repeatedly call the client's update method until the server is detected
    auto update_func = std::bind(
      &interactive_markers::InteractiveMarkerClient::update, client_.get());
    TIMED_EXPECT_EQ(
      client_->getState(), ClientState::STATE_INITIALIZE, 3s, 10ms, (*executor_), update_func);
    // Make the required TF data in order to finish initializing
    makeTfDataAvailable(mock_server->markers_);
    TIMED_EXPECT_EQ(
      client_->getState(), ClientState::STATE_RUNNING, 3s, 10ms, (*executor_), update_func);
    // Cause a sync error by skipping a sequence number, which should cause state change
    mock_server->publishUpdate();
    mock_server->sequence_number_ += 1;
    mock_server->publishUpdate();
    TIMED_EXPECT_EQ(
      client_->getState(), ClientState::STATE_INITIALIZE, 3s, 10ms, (*executor_), update_func);
    // Disconnect server
    executor_->remove_node(mock_server);
    mock_server.reset();
    TIMED_EXPECT_EQ(
      client_->getState(), ClientState::STATE_IDLE, 3s, 10ms, (*executor_), update_func);
  }
}

TEST_F(TestInteractiveMarkerClient, init_callback)
{
  using namespace std::chrono_literals;

  bool called = false;
  auto callback = [&called](visualization_msgs::srv::GetInteractiveMarkers::Response::SharedPtr)
    {
      called = true;
    };

  client_->setInitializeCallback(callback);

  // Creating a server and publishing the required TF data should trigger the callback
  auto mock_server = std::make_shared<MockInteractiveMarkerServer>(topic_namespace_);
  executor_->add_node(mock_server);
  makeTfDataAvailable(mock_server->markers_);
  auto update_func = std::bind(
    &interactive_markers::InteractiveMarkerClient::update, client_.get());
  TIMED_EXPECT_EQ(called, true, 3s, 10ms, (*executor_), update_func);
}

TEST_F(TestInteractiveMarkerClient, update_callback)
{
  using namespace std::chrono_literals;

  geometry_msgs::msg::Pose output_pose;
  auto callback = [&output_pose](visualization_msgs::msg::InteractiveMarkerUpdate::SharedPtr msg)
    {
      ASSERT_EQ(msg->poses.size(), 1u);
      output_pose = msg->poses[0].pose;
    };

  client_->setUpdateCallback(callback);

  // First, we need to get into the RUNNING state
  auto mock_server = std::make_shared<MockInteractiveMarkerServer>(topic_namespace_);
  executor_->add_node(mock_server);
  waitForDiscovery(mock_server);
  makeTfDataAvailable(mock_server->markers_);
  auto update_func = std::bind(
    &interactive_markers::InteractiveMarkerClient::update, client_.get());
  TIMED_EXPECT_EQ(
    client_->getState(), ClientState::STATE_RUNNING, 3s, 10ms, (*executor_), update_func);

  // Publish an update message
  geometry_msgs::msg::Pose input_pose;
  input_pose.position.x = 42.0;
  input_pose.position.y = -2.2;
  input_pose.position.z = 0.0;
  input_pose.orientation.w = 0.5;
  mock_server->publishUpdate(input_pose);
  TIMED_EXPECT_EQ(
    output_pose.position.x, input_pose.position.x, 3s, 10ms, (*executor_), update_func);
  EXPECT_EQ(output_pose.position.y, input_pose.position.y);
  EXPECT_EQ(output_pose.position.z, input_pose.position.z);
  EXPECT_EQ(output_pose.orientation.w, input_pose.orientation.w);
}

TEST_F(TestInteractiveMarkerClient, reset_callback)
{
  using namespace std::chrono_literals;

  bool reset_called = false;
  auto callback = [&reset_called]()
    {
      reset_called = true;
    };

  client_->setResetCallback(callback);

  // Get out of the IDLE state by creating a server and publishing the required TF data
  auto mock_server = std::make_shared<MockInteractiveMarkerServer>(topic_namespace_);
  executor_->add_node(mock_server);
  makeTfDataAvailable(mock_server->markers_);
  auto update_func = std::bind(
    &interactive_markers::InteractiveMarkerClient::update, client_.get());
  TIMED_EXPECT_EQ(
    client_->getState(), ClientState::STATE_INITIALIZE, 3s, 10ms, (*executor_), update_func);

  // Disconnect server to cause reset
  executor_->remove_node(mock_server);
  mock_server.reset();
  TIMED_EXPECT_EQ(
    client_->getState(), ClientState::STATE_IDLE, 3s, 10ms, (*executor_), update_func);
  EXPECT_TRUE(reset_called);
}

TEST_F(TestInteractiveMarkerClient, feedback)
{
  using namespace std::chrono_literals;

  auto mock_server = std::make_shared<MockInteractiveMarkerServer>(topic_namespace_);
  executor_->add_node(mock_server);
  waitForDiscovery(mock_server);

  EXPECT_EQ(mock_server->feedback_received_, 0u);

  // Publish a feedback message
  visualization_msgs::msg::InteractiveMarkerFeedback feedback_msg;
  client_->publishFeedback(feedback_msg);
  TIMED_EXPECT_EQ(mock_server->feedback_received_, 1u, 3s, 10ms, (*executor_));

  // Publish another
  client_->publishFeedback(feedback_msg);
  TIMED_EXPECT_EQ(mock_server->feedback_received_, 2u, 3s, 10ms, (*executor_));
}
