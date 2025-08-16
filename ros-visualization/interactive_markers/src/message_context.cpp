// Copyright (c) 2011, Willow Garage, Inc.
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

#include <inttypes.h>

#include <list>
#include <memory>
#include <string>
#include <vector>

#include "rcutils/logging_macros.h"
#include "tf2/buffer_core_interface.h"
#include "tf2_geometry_msgs/tf2_geometry_msgs.hpp"
#include "rclcpp/time.hpp"
#include "visualization_msgs/srv/get_interactive_markers.hpp"

#include "interactive_markers/exceptions.hpp"
#include "interactive_markers/message_context.hpp"
#include "interactive_markers/tools.hpp"

namespace interactive_markers
{

template<class MsgT>
MessageContext<MsgT>::MessageContext(
  std::shared_ptr<tf2::BufferCoreInterface> tf_buffer_core,
  const std::string & target_frame,
  typename MsgT::ConstSharedPtr _msg,
  bool enable_autocomplete_transparency)
: tf_buffer_core_(tf_buffer_core),
  target_frame_(target_frame),
  enable_autocomplete_transparency_(enable_autocomplete_transparency)
{
  // copy message, as we will be modifying it
  msg = std::make_shared<MsgT>(*_msg);

  init();
}

template<class MsgT>
MessageContext<MsgT> & MessageContext<MsgT>::operator=(const MessageContext<MsgT> & other)
{
  open_marker_idx_ = other.open_marker_idx_;
  open_pose_idx_ = other.open_pose_idx_;
  target_frame_ = other.target_frame_;
  enable_autocomplete_transparency_ = other.enable_autocomplete_transparency_;
  return *this;
}

template<class MsgT>
bool MessageContext<MsgT>::getTransform(
  std_msgs::msg::Header & header,
  geometry_msgs::msg::Pose & pose_msg)
{
  try {
    if (header.frame_id != target_frame_) {
      // get transform
      geometry_msgs::msg::TransformStamped transform = tf_buffer_core_->lookupTransform(
        target_frame_, header.frame_id, tf2::timeFromSec(rclcpp::Time(header.stamp).seconds()));
      RCUTILS_LOG_DEBUG(
        "Transform %s -> %s at time %f is ready.",
        header.frame_id.c_str(), target_frame_.c_str(), rclcpp::Time(header.stamp).seconds());

      // if timestamp is given, transform message into target frame
      if (header.stamp != rclcpp::Time(0)) {
        geometry_msgs::msg::PoseStamped pose_stamped_msg;
        pose_stamped_msg.header = header;
        pose_stamped_msg.pose = pose_msg;
        tf2::doTransform(pose_stamped_msg, pose_stamped_msg, transform);
        // store transformed pose in original message
        pose_msg = pose_stamped_msg.pose;
        RCUTILS_LOG_DEBUG("Changing %s to %s", header.frame_id.c_str(), target_frame_.c_str());
        header.frame_id = target_frame_;
      }
    }
  } catch (tf2::ExtrapolationException &) {
    // Get latest common time
    // Call lookupTransform with time=0 and use the stamp on the resultant transform.
    geometry_msgs::msg::TransformStamped transform =
      tf_buffer_core_->lookupTransform(
      target_frame_, header.frame_id, tf2::TimePoint());
    rclcpp::Time latest_time = transform.header.stamp;

    if (latest_time != rclcpp::Time(0) && latest_time > header.stamp) {
      std::ostringstream oss;
      oss << "The message contains an old timestamp and cannot be transformed " <<
        "('" << header.frame_id << "' to '" << target_frame_ << "' at time " <<
        rclcpp::Time(header.stamp).seconds() << ").";
      throw exceptions::TransformError(oss.str());
    }
    return false;
  } catch (tf2::TransformException & e) {
    throw exceptions::TransformError(e.what());
  }
  return true;
}

template<class MsgT>
void MessageContext<MsgT>::getTfTransforms(
  std::vector<visualization_msgs::msg::InteractiveMarker> & msg_vec, std::list<size_t> & indices)
{
  std::list<size_t>::iterator idx_it;
  for (idx_it = indices.begin(); idx_it != indices.end(); ) {
    visualization_msgs::msg::InteractiveMarker & im_msg = msg_vec[*idx_it];
    // transform interactive marker
    bool success = getTransform(im_msg.header, im_msg.pose);
    // transform regular markers
    for (unsigned c = 0; c < im_msg.controls.size(); c++) {
      visualization_msgs::msg::InteractiveMarkerControl & ctrl_msg = im_msg.controls[c];
      for (unsigned m = 0; m < ctrl_msg.markers.size(); m++) {
        visualization_msgs::msg::Marker & marker_msg = ctrl_msg.markers[m];
        if (!marker_msg.header.frame_id.empty()) {
          success = success && getTransform(marker_msg.header, marker_msg.pose);
        }
      }
    }

    if (success) {
      idx_it = indices.erase(idx_it);
    } else {
      RCUTILS_LOG_DEBUG(
        "Transform %s -> %s at time %f is not ready.",
        im_msg.header.frame_id.c_str(), target_frame_.c_str(), rclcpp::Time(
          im_msg.header.stamp).seconds());
      ++idx_it;
    }
  }
}

template<class MsgT>
void MessageContext<MsgT>::getTfTransforms(
  std::vector<visualization_msgs::msg::InteractiveMarkerPose> & msg_vec,
  std::list<size_t> & indices)
{
  std::list<size_t>::iterator idx_it;
  for (idx_it = indices.begin(); idx_it != indices.end(); ) {
    visualization_msgs::msg::InteractiveMarkerPose & msg = msg_vec[*idx_it];
    if (getTransform(msg.header, msg.pose)) {
      idx_it = indices.erase(idx_it);
    } else {
      RCUTILS_LOG_DEBUG(
        "Transform %s -> %s at time %f is not ready.",
        msg.header.frame_id.c_str(), target_frame_.c_str(), rclcpp::Time(
          msg.header.stamp).seconds());
      ++idx_it;
    }
  }
}

template<class MsgT>
bool MessageContext<MsgT>::isReady()
{
  return open_marker_idx_.empty() && open_pose_idx_.empty();
}

template<>
void MessageContext<visualization_msgs::msg::InteractiveMarkerUpdate>::init()
{
  // mark all transforms as being missing
  for (size_t i = 0; i < msg->markers.size(); i++) {
    open_marker_idx_.push_back(i);
  }
  for (size_t i = 0; i < msg->poses.size(); i++) {
    open_pose_idx_.push_back(i);
  }
  for (unsigned i = 0; i < msg->markers.size(); i++) {
    autoComplete(msg->markers[i], enable_autocomplete_transparency_);
  }
  for (unsigned i = 0; i < msg->poses.size(); i++) {
    // correct empty orientation
    if (msg->poses[i].pose.orientation.w == 0 && msg->poses[i].pose.orientation.x == 0 &&
      msg->poses[i].pose.orientation.y == 0 && msg->poses[i].pose.orientation.z == 0)
    {
      msg->poses[i].pose.orientation.w = 1;
    }
  }
}

template<>
void MessageContext<visualization_msgs::srv::GetInteractiveMarkers::Response>::init()
{
  // mark all transforms as being missing
  for (size_t i = 0; i < msg->markers.size(); i++) {
    open_marker_idx_.push_back(i);
  }
  for (unsigned i = 0; i < msg->markers.size(); i++) {
    autoComplete(msg->markers[i], enable_autocomplete_transparency_);
  }
}

template<>
void MessageContext<visualization_msgs::msg::InteractiveMarkerUpdate>::getTfTransforms()
{
  getTfTransforms(msg->markers, open_marker_idx_);
  getTfTransforms(msg->poses, open_pose_idx_);
  if (isReady()) {
    RCUTILS_LOG_DEBUG("Update message with seq_num=%" PRIu64 " is ready.", msg->seq_num);
  }
}

template<>
void MessageContext<visualization_msgs::srv::GetInteractiveMarkers::Response>::getTfTransforms()
{
  getTfTransforms(msg->markers, open_marker_idx_);
  if (isReady()) {
    RCUTILS_LOG_DEBUG("Response message with seq_num=%" PRIu64 " is ready.", msg->sequence_number);
  }
}

// explicit template instantiation
template class MessageContext<visualization_msgs::msg::InteractiveMarkerUpdate>;
template class MessageContext<visualization_msgs::srv::GetInteractiveMarkers::Response>;

}  // namespace interactive_markers
