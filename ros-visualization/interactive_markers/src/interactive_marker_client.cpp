// Copyright (c) 2011, Willow Garage, Inc.
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

// Author: David Gossow

#include <memory>
#include <mutex>
#include <string>
#include <utility>

#include "visualization_msgs/srv/get_interactive_markers.hpp"

#include "interactive_markers/exceptions.hpp"
#include "interactive_markers/interactive_marker_client.hpp"

using namespace std::placeholders;

namespace interactive_markers
{

const size_t MAX_UPDATE_QUEUE_SIZE = 100u;

InteractiveMarkerClient::~InteractiveMarkerClient()
{
}

void InteractiveMarkerClient::connect(std::string topic_namespace)
{
  changeState(STATE_IDLE);
  topic_namespace_ = topic_namespace;

  // Terminate any existing connection
  disconnect();

  // Don't do anything if no namespace is provided
  if (topic_namespace_.empty()) {
    return;
  }

  try {
    get_interactive_markers_client_ =
      rclcpp::create_client<visualization_msgs::srv::GetInteractiveMarkers>(
      node_base_interface_,
      graph_interface_,
      services_interface_,
      topic_namespace_ + "/get_interactive_markers",
      rmw_qos_profile_services_default,
      nullptr);

    feedback_pub_ = rclcpp::create_publisher<visualization_msgs::msg::InteractiveMarkerFeedback>(
      topics_interface_,
      topic_namespace_ + "/feedback",
      feedback_pub_qos_);

    update_sub_ = rclcpp::create_subscription<visualization_msgs::msg::InteractiveMarkerUpdate>(
      topics_interface_,
      topic_namespace_ + "/update",
      update_sub_qos_,
      std::bind(&InteractiveMarkerClient::processUpdate, this, _1));
  } catch (rclcpp::exceptions::InvalidNodeError & ex) {
    updateStatus(STATUS_ERROR, "Failed to connect: " + std::string(ex.what()));
    disconnect();
    return;
  } catch (rclcpp::exceptions::NameValidationError & ex) {
    updateStatus(STATUS_ERROR, "Failed to connect: " + std::string(ex.what()));
    disconnect();
    return;
  }

  updateStatus(STATUS_INFO, "Connected on namespace: " + topic_namespace_);
}

void InteractiveMarkerClient::disconnect()
{
  get_interactive_markers_client_.reset();
  feedback_pub_.reset();
  update_sub_.reset();
  reset();
}

void InteractiveMarkerClient::publishFeedback(
  visualization_msgs::msg::InteractiveMarkerFeedback feedback)
{
  feedback.client_id = client_id_;
  feedback_pub_->publish(feedback);
}

void InteractiveMarkerClient::update()
{
  if (!get_interactive_markers_client_) {
    // Disconnected
    return;
  }

  const bool server_ready = get_interactive_markers_client_->service_is_ready();

  switch (state_) {
    case STATE_IDLE:
      if (server_ready) {
        changeState(STATE_INITIALIZE);
      }
      break;

    case STATE_INITIALIZE:
      if (!server_ready) {
        updateStatus(STATUS_WARN, "Server not available during initialization, resetting");
        changeState(STATE_IDLE);
        break;
      }
      // If there's an unexpected error, reset
      if (!transformInitialMessage()) {
        changeState(STATE_IDLE);
        break;
      }
      if (checkInitializeFinished()) {
        changeState(STATE_RUNNING);
      }
      break;

    case STATE_RUNNING:
      if (!server_ready) {
        updateStatus(STATUS_WARN, "Server not available while running, resetting");
        changeState(STATE_IDLE);
        break;
      }
      // If there's an unexpected error, reset
      if (!transformUpdateMessages()) {
        changeState(STATE_IDLE);
        break;
      }
      pushUpdates();
      break;

    default:
      updateStatus(STATUS_ERROR, "Invalid state in update: " + std::to_string(getState()));
  }
}

void InteractiveMarkerClient::setTargetFrame(std::string target_frame)
{
  if (target_frame_ == target_frame) {
    return;
  }

  target_frame_ = target_frame;
  updateStatus(STATUS_INFO, "Target frame is now " + target_frame_);

  // Call reset for change to take effect
  // This will cause interactive markers to be requested again from the server
  // The additional request might be avoided by doing something else, but this is easier
  changeState(STATE_IDLE);
}

void InteractiveMarkerClient::setInitializeCallback(const InitializeCallback & cb)
{
  initialize_callback_ = cb;
}

void InteractiveMarkerClient::setUpdateCallback(const UpdateCallback & cb)
{
  update_callback_ = cb;
}

void InteractiveMarkerClient::setResetCallback(const ResetCallback & cb)
{
  reset_callback_ = cb;
}

void InteractiveMarkerClient::setStatusCallback(const StatusCallback & cb)
{
  status_callback_ = cb;
}

void InteractiveMarkerClient::reset()
{
  std::unique_lock<std::recursive_mutex> lock(mutex_);
  state_ = STATE_IDLE;
  first_update_ = true;
  initial_response_msg_.reset();
  update_queue_.clear();
  if (reset_callback_) {
    reset_callback_();
  }
}

void InteractiveMarkerClient::requestInteractiveMarkers()
{
  if (!get_interactive_markers_client_) {
    updateStatus(STATUS_ERROR, "Interactive markers requested when client is disconnected");
    return;
  }
  if (!get_interactive_markers_client_->wait_for_service(std::chrono::seconds(1))) {
    updateStatus(STATUS_WARN, "Service is not ready during request for interactive markers");
    return;
  }
  updateStatus(STATUS_INFO, "Sending request for interactive markers");

  auto callback = std::bind(&InteractiveMarkerClient::processInitialMessage, this, _1);
  auto request = std::make_shared<visualization_msgs::srv::GetInteractiveMarkers::Request>();
  get_interactive_markers_client_->async_send_request(
    request,
    callback);
  request_time_ = clock_->now();
}

void InteractiveMarkerClient::processInitialMessage(
  rclcpp::Client<visualization_msgs::srv::GetInteractiveMarkers>::SharedFuture future)
{
  updateStatus(STATUS_INFO, "Service response received for initialization");
  auto response = future.get();
  {
    std::unique_lock<std::recursive_mutex> lock(mutex_);
    initial_response_msg_ = std::make_shared<InitialMessageContext>(
      tf_buffer_core_, target_frame_, response, enable_autocomplete_transparency_);
  }
}

void InteractiveMarkerClient::processUpdate(
  visualization_msgs::msg::InteractiveMarkerUpdate::ConstSharedPtr msg)
{
  // Ignore legacy "keep alive" messages
  if (msg->type == msg->KEEP_ALIVE) {
    RCLCPP_WARN_ONCE(
      logger_,
      "KEEP_ALIVE message ignored. "
      "Servers are no longer expected to publish this type of message.");
    return;
  }

  if (!first_update_ && msg->seq_num != last_update_sequence_number_ + 1) {
    std::ostringstream oss;
    oss << "Update sequence number is out of order. " << last_update_sequence_number_ + 1 <<
      " (expected) vs. " << msg->seq_num << " (received)";
    updateStatus(STATUS_WARN, oss.str());
    // Change state to STATE_IDLE to cause reset
    changeState(STATE_IDLE);
    return;
  }

  updateStatus(
    STATUS_DEBUG, "Received update with sequence number " + std::to_string(msg->seq_num));

  first_update_ = false;
  last_update_sequence_number_ = msg->seq_num;

  if (update_queue_.size() > MAX_UPDATE_QUEUE_SIZE) {
    updateStatus(
      STATUS_WARN,
      "Update queue too large. Erasing message with sequence number " +
      std::to_string(update_queue_.begin()->msg->seq_num));
    update_queue_.pop_back();
  }

  update_queue_.push_front(
    UpdateMessageContext(
      tf_buffer_core_, target_frame_, msg, enable_autocomplete_transparency_));
}

bool InteractiveMarkerClient::transformInitialMessage()
{
  std::unique_lock<std::recursive_mutex> lock(mutex_);
  if (!initial_response_msg_) {
    // We haven't received a response yet
    return true;
  }

  try {
    initial_response_msg_->getTfTransforms();
  } catch (const exceptions::TransformError & e) {
    std::ostringstream oss;
    oss << "Resetting due to transform error: " << e.what();
    updateStatus(STATUS_DEBUG, oss.str());  // DEBUG here to reduce spam from repeated resetting
    return false;
  }

  return true;
}

bool InteractiveMarkerClient::transformUpdateMessages()
{
  std::unique_lock<std::recursive_mutex> lock(mutex_);
  for (auto it = update_queue_.begin(); it != update_queue_.end(); ++it) {
    try {
      it->getTfTransforms();
    } catch (const exceptions::TransformError & e) {
      std::ostringstream oss;
      oss << "Resetting due to transform error: " << e.what();
      updateStatus(STATUS_ERROR, oss.str());
      return false;
    }
  }
  return true;
}

bool InteractiveMarkerClient::checkInitializeFinished()
{
  std::unique_lock<std::recursive_mutex> lock(mutex_);
  if (!initial_response_msg_) {
    // We haven't received a response yet, check for timeout
    if ((clock_->now() - request_time_) > request_timeout_) {
      updateStatus(
        STATUS_WARN, "Did not receive response with interactive markers, resending request...");
      requestInteractiveMarkers();
    }

    return false;
  }

  const uint64_t & response_sequence_number = initial_response_msg_->msg->sequence_number;
  if (!initial_response_msg_->isReady()) {
    updateStatus(STATUS_DEBUG, "Initialization: Waiting for TF info");
    return false;
  }

  // Prune old update messages
  while (!update_queue_.empty() && update_queue_.back().msg->seq_num <= response_sequence_number) {
    updateStatus(
      STATUS_DEBUG,
      "Omitting update with sequence number " + std::to_string(update_queue_.back().msg->seq_num));
    update_queue_.pop_back();
  }

  if (initialize_callback_) {
    initialize_callback_(initial_response_msg_->msg);
  }

  updateStatus(STATUS_DEBUG, "Initialized");

  return true;
}

void InteractiveMarkerClient::pushUpdates()
{
  std::unique_lock<std::recursive_mutex> lock(mutex_);
  while (!update_queue_.empty() && update_queue_.back().isReady()) {
    visualization_msgs::msg::InteractiveMarkerUpdate::SharedPtr msg = update_queue_.back().msg;
    updateStatus(
      STATUS_DEBUG, "Pushing update with sequence number " + std::to_string(msg->seq_num));
    if (update_callback_) {
      update_callback_(msg);
    }
    update_queue_.pop_back();
  }
}

void InteractiveMarkerClient::changeState(const State & new_state)
{
  if (state_ == new_state) {
    return;
  }

  updateStatus(STATUS_DEBUG, "Change state to: " + std::to_string(new_state));

  switch (new_state) {
    case STATE_IDLE:
      reset();
      break;

    case STATE_INITIALIZE:
      requestInteractiveMarkers();
      break;

    case STATE_RUNNING:
      break;

    default:
      updateStatus(STATUS_ERROR, "Invalid state when changing state: " + std::to_string(new_state));
      return;
  }
  state_ = new_state;
}

void InteractiveMarkerClient::updateStatus(const Status status, const std::string & msg)
{
  switch (status) {
    case STATUS_DEBUG:
      RCLCPP_DEBUG(logger_, "%s", msg.c_str());
      break;
    case STATUS_INFO:
      RCLCPP_INFO(logger_, "%s", msg.c_str());
      break;
    case STATUS_WARN:
      RCLCPP_WARN(logger_, "%s", msg.c_str());
      break;
    case STATUS_ERROR:
      RCLCPP_ERROR(logger_, "%s", msg.c_str());
      break;
  }

  if (status_callback_) {
    status_callback_(status, msg);
  }
}

}  // namespace interactive_markers
