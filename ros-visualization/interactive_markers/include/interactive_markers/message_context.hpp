// Copyright (c) 2012, Willow Garage, Inc.
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

#ifndef INTERACTIVE_MARKERS__MESSAGE_CONTEXT_HPP_
#define INTERACTIVE_MARKERS__MESSAGE_CONTEXT_HPP_

#include <list>
#include <memory>
#include <string>
#include <vector>

#include "visualization_msgs/msg/interactive_marker_init.hpp"
#include "visualization_msgs/msg/interactive_marker_update.hpp"

namespace tf2
{
class BufferCoreInterface;
}

namespace interactive_markers
{

template<class MsgT>
class MessageContext
{
public:
  MessageContext(
    std::shared_ptr<tf2::BufferCoreInterface> tf_buffer_core,
    const std::string & target_frame,
    typename MsgT::ConstSharedPtr msg,
    bool enable_autocomplete_transparency = true);

  MessageContext(const MessageContext &) = default;

  MessageContext<MsgT> & operator=(const MessageContext<MsgT> & other);

  // transform all messages with timestamp into target frame
  void getTfTransforms();

  typename MsgT::SharedPtr msg;

  // return true if tf info is complete
  bool isReady();

private:
  void init();

  bool getTransform(std_msgs::msg::Header & header, geometry_msgs::msg::Pose & pose_msg);

  void getTfTransforms(
    std::vector<visualization_msgs::msg::InteractiveMarker> & msg_vec,
    std::list<size_t> & indices);
  void getTfTransforms(
    std::vector<visualization_msgs::msg::InteractiveMarkerPose> & msg_vec,
    std::list<size_t> & indices);

  // array indices of marker/pose updates with missing tf info
  std::list<size_t> open_marker_idx_;
  std::list<size_t> open_pose_idx_;
  std::shared_ptr<tf2::BufferCoreInterface> tf_buffer_core_;
  std::string target_frame_;
  bool enable_autocomplete_transparency_;
};  // class MessageContext

}  // namespace interactive_markers

#endif  // INTERACTIVE_MARKERS__MESSAGE_CONTEXT_HPP_
