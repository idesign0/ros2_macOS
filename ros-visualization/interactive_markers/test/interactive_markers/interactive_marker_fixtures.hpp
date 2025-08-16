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

#ifndef INTERACTIVE_MARKERS__INTERACTIVE_MARKER_FIXTURES_HPP_
#define INTERACTIVE_MARKERS__INTERACTIVE_MARKER_FIXTURES_HPP_
#include <vector>

#include "visualization_msgs/msg/interactive_marker.hpp"
#include "visualization_msgs/msg/interactive_marker_control.hpp"
#include "visualization_msgs/msg/menu_entry.hpp"

std::vector<visualization_msgs::msg::InteractiveMarker>
get_interactive_markers()
{
  std::vector<visualization_msgs::msg::InteractiveMarker> markers;
  {
    visualization_msgs::msg::InteractiveMarker marker;
    marker.name = "test_marker_0";
    marker.header.frame_id = "test_frame_id";
    markers.push_back(marker);
  }
  {
    visualization_msgs::msg::InteractiveMarker marker;
    marker.name = "test_marker_1";
    marker.header.frame_id = "test_frame_id";
    marker.pose.position.x = 1.0;
    marker.pose.orientation.w = 1.0;
    marker.description = "My test marker description";
    marker.scale = 3.14f;
    visualization_msgs::msg::MenuEntry menu_entry;
    menu_entry.id = 42;
    menu_entry.title = "My test menu title";
    menu_entry.command = "Some test command to be run";
    marker.menu_entries.push_back(menu_entry);
    visualization_msgs::msg::InteractiveMarkerControl control;
    control.name = "test_control_name";
    control.orientation.w = 1.0;
    control.always_visible = true;
    control.description = "My test control description";
    marker.controls.push_back(control);
    markers.push_back(marker);
  }
  return markers;
}

#endif  // INTERACTIVE_MARKERS__INTERACTIVE_MARKER_FIXTURES_HPP_
