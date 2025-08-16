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

#ifndef INTERACTIVE_MARKERS__INTERACTIVE_MARKER_SERVER_HPP_
#define INTERACTIVE_MARKERS__INTERACTIVE_MARKER_SERVER_HPP_

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

#include "rclcpp/rclcpp.hpp"
#include "visualization_msgs/msg/interactive_marker_feedback.hpp"
#include "visualization_msgs/msg/interactive_marker_update.hpp"
#include "visualization_msgs/srv/get_interactive_markers.hpp"

#include "interactive_markers/visibility_control.hpp"

namespace interactive_markers
{

/// A server to one or many clients (e.g. rviz) displaying a set of interactive markers.
/**
 * Note that changes made by calling insert(), erase(), setCallback() etc. are not applied until
 * calling applyChanges().
 */
class InteractiveMarkerServer
{
public:
  typedef visualization_msgs::msg::InteractiveMarkerFeedback::ConstSharedPtr FeedbackConstSharedPtr;
  typedef std::function<void (FeedbackConstSharedPtr)> FeedbackCallback;

  static const uint8_t DEFAULT_FEEDBACK_CB = 255;

  /// Constructor.
  /**
   * \param topic_namepsace The namespace for the ROS services and topics used for communication
   *   with interactive marker clients.
   * \param base_interface Node base interface.
   * \param clock_interface Node clock interface.
   * \param logging_interface Node logging interface.
   * \param topics_interface Node topics interface.
   * \param service_interface Node service interface.
   * \param update_pub_qos QoS settings for the update publisher.
   * \param feedback_sub_qos QoS settings for the feedback subscription.
   */
  INTERACTIVE_MARKERS_PUBLIC
  InteractiveMarkerServer(
    const std::string & topic_namespace,
    rclcpp::node_interfaces::NodeBaseInterface::SharedPtr base_interface,
    rclcpp::node_interfaces::NodeClockInterface::SharedPtr clock_interface,
    rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr logging_interface,
    rclcpp::node_interfaces::NodeTopicsInterface::SharedPtr topics_interface,
    rclcpp::node_interfaces::NodeServicesInterface::SharedPtr services_interface,
    const rclcpp::QoS & update_pub_qos = rclcpp::QoS(100),
    const rclcpp::QoS & feedback_sub_qos = rclcpp::QoS(1));

  template<typename NodePtr>
  InteractiveMarkerServer(
    const std::string & topic_namespace,
    NodePtr node,
    const rclcpp::QoS & update_pub_qos = rclcpp::QoS(100),
    const rclcpp::QoS & feedback_sub_qos = rclcpp::QoS(1))
  : InteractiveMarkerServer(
      topic_namespace,
      node->get_node_base_interface(),
      node->get_node_clock_interface(),
      node->get_node_logging_interface(),
      node->get_node_topics_interface(),
      node->get_node_services_interface(),
      update_pub_qos,
      feedback_sub_qos)
  {
  }

  /// Destruction of the interface will lead to all managed markers being cleared.
  INTERACTIVE_MARKERS_PUBLIC
  ~InteractiveMarkerServer();

  /// Add or replace a marker without changing its callback functions.
  /**
   * Note, changes to the marker will not take effect until you call applyChanges().
   * The callback changes immediately.
   *
   * \param marker The marker to be added or replaced.
   */
  INTERACTIVE_MARKERS_PUBLIC
  void insert(const visualization_msgs::msg::InteractiveMarker & marker);

  /// Add or replace a marker and its callback functions.
  /**
   * Note, changes to the marker will not take effect until you call applyChanges().
   * The callback changes immediately.
   *
   * \param marker The marker to be added or replaced.
   * \param feedback_callback Function to call on the arrival of a feedback message.
   * \param feedback_type Type of feedback for which to call the feedback callback.
   */
  INTERACTIVE_MARKERS_PUBLIC
  void insert(
    const visualization_msgs::msg::InteractiveMarker & marker,
    FeedbackCallback feedback_callback,
    uint8_t feedback_type = DEFAULT_FEEDBACK_CB);

  /// Update the pose of a marker by name.
  /**
   * Note, this change will not take effect until you call applyChanges().
   *
   * \param name Name of the interactive marker to update.
   * \param pose The new pose.
   * \param header Header replacement.
   *   Leave this empty to use the previous one.
   * \return true if a marker with the provided name exists, false otherwise.
   */
  INTERACTIVE_MARKERS_PUBLIC
  bool setPose(
    const std::string & name,
    const geometry_msgs::msg::Pose & pose,
    const std_msgs::msg::Header & header = std_msgs::msg::Header());

  /// Erase a marker by name.
  /**
   * Note, this change will not take effect until you call applyChanges().
   * \param name Name of the interactive marker to erase.
   * \return true if a marker with the provided name exists, false otherwise.
   */
  INTERACTIVE_MARKERS_PUBLIC
  bool erase(const std::string & name);

  /// Clear all markers.
  /**
   * Note, this change will not take effect until you call applyChanges().
   */
  INTERACTIVE_MARKERS_PUBLIC
  void clear();

  /// Return whether the server contains any markers.
  /**
   * Does not include markers inserted since the last applyChanges().
   *
   * \return true if the server contains no markers, false otherwise.
   */
  INTERACTIVE_MARKERS_PUBLIC
  bool empty() const;

  /// Return the number of markers contained in the server.
  /**
   * Does not include markers inserted since the last applyChanges().
   * \return The number of markers contained in the server.
   */
  INTERACTIVE_MARKERS_PUBLIC
  std::size_t size() const;

  /// Add or replace a callback function for a marker.
  /**
   * Note, this change will not take effect until you call applyChanges().
   * The server will try to call any type-specific callback first.
   * If none is set, it will call the default callback.
   * If a callback for the given type already exists, it will be replaced.
   * To unset a type-specific callback, pass in an empty one.
   *
   * \param name Name of an existing interactive marker.
   * \param feedback_callback Function to call on the arrival of a feedback message.
   * \param feedback_type Type of feedback for which to call the feedback callback.
   *   Leave this empty to make this the default callback.
   * \return true if the setting the callback was successful, false if the provided
   *   name does not match an existing marker.
   */
  INTERACTIVE_MARKERS_PUBLIC
  bool setCallback(
    const std::string & name,
    FeedbackCallback feedback_cb,
    uint8_t feedback_type = DEFAULT_FEEDBACK_CB);

  /// Apply changes made since the last call to this method and broadcast an update to all clients.
  INTERACTIVE_MARKERS_PUBLIC
  void applyChanges();

  /// Get a marker by name.
  /**
   * \param[in] name Name of the interactive marker.
   * \param[out] marker Output message.
   *   Not set if a marker with the provided name does not exist.
   * \return true if a marker with the provided name exists, false otherwise.
   */
  INTERACTIVE_MARKERS_PUBLIC
  bool get(std::string name, visualization_msgs::msg::InteractiveMarker & int_marker) const;

private:
  // Disable copying
  InteractiveMarkerServer(const InteractiveMarkerServer &) = delete;
  InteractiveMarkerServer & operator=(const InteractiveMarkerServer &) = delete;

  struct MarkerContext
  {
    rclcpp::Time last_feedback;
    std::string last_client_id;
    FeedbackCallback default_feedback_cb;
    std::unordered_map<uint8_t, FeedbackCallback> feedback_cbs;
    visualization_msgs::msg::InteractiveMarker int_marker;
  };

  typedef std::unordered_map<std::string, MarkerContext> M_MarkerContext;

  // represents an update to a single marker
  struct UpdateContext
  {
    enum
    {
      FULL_UPDATE,
      POSE_UPDATE,
      ERASE
    } update_type;
    visualization_msgs::msg::InteractiveMarker int_marker;
    FeedbackCallback default_feedback_cb;
    std::unordered_map<uint8_t, FeedbackCallback> feedback_cbs;
  };

  typedef std::unordered_map<std::string, UpdateContext> M_UpdateContext;

  void getInteractiveMarkersCallback(
    const std::shared_ptr<rmw_request_id_t> request_header,
    const std::shared_ptr<visualization_msgs::srv::GetInteractiveMarkers::Request> request,
    std::shared_ptr<visualization_msgs::srv::GetInteractiveMarkers::Response> response);

  // update marker pose & call user callback
  void processFeedback(
    visualization_msgs::msg::InteractiveMarkerFeedback::ConstSharedPtr feedback);

  // increase sequence number & publish an update
  void publish(visualization_msgs::msg::InteractiveMarkerUpdate & update);

  // Update pose, schedule update without locking
  void doSetPose(
    M_UpdateContext::iterator update_it,
    const std::string & name,
    const geometry_msgs::msg::Pose & pose,
    const std_msgs::msg::Header & header);

  // contains the current state of all markers
  M_MarkerContext marker_contexts_;

  // updates that have to be sent on the next publish
  M_UpdateContext pending_updates_;

  // topic namespace to use
  std::string topic_namespace_;

  mutable std::recursive_mutex mutex_;

  rclcpp::Service<visualization_msgs::srv::GetInteractiveMarkers>::SharedPtr
    get_interactive_markers_service_;
  rclcpp::Publisher<visualization_msgs::msg::InteractiveMarkerUpdate>::SharedPtr update_pub_;
  rclcpp::Subscription<visualization_msgs::msg::InteractiveMarkerFeedback>::SharedPtr feedback_sub_;

  rclcpp::Context::SharedPtr context_;
  rclcpp::Clock::SharedPtr clock_;
  rclcpp::Logger logger_;

  uint64_t sequence_number_;
};  // class InteractiveMarkerServer

}  // namespace interactive_markers

#endif  // INTERACTIVE_MARKERS__INTERACTIVE_MARKER_SERVER_HPP_
