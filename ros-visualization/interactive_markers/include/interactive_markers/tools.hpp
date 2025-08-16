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

#ifndef INTERACTIVE_MARKERS__TOOLS_HPP_
#define INTERACTIVE_MARKERS__TOOLS_HPP_

#include <string>

#include "visualization_msgs/msg/interactive_marker.hpp"

#include "interactive_markers/visibility_control.hpp"

namespace interactive_markers
{

/// Fill in default values and insert default controls when none are specified.
/**
 * This also calls uniqueifyControlNames().
 *
 * \param msg[inout] Interactive marker to be completed.
 * \param enable_autocomplete_transparency If false, auto-completed markers will have alpha = 1.0.
 */
INTERACTIVE_MARKERS_PUBLIC
void autoComplete(
  visualization_msgs::msg::InteractiveMarker & msg,
  bool enable_autocomplete_transparency = true);

/// Fill in default values and insert default controls when none are specified.
/**
 * \param msg[in] Interactive marker which contains the control.
 * \param control[inout] The control to be completed.
 * \param enable_autocomplete_transparency If false, auto-completed markers will have alpha = 1.0.
 */
INTERACTIVE_MARKERS_PUBLIC
void autoComplete(
  const visualization_msgs::msg::InteractiveMarker & msg,
  visualization_msgs::msg::InteractiveMarkerControl & control,
  bool enable_autocomplete_transparency = true);

/// Make sure all of the control names are unique within the given message.
/**
 * Appends "_u0", "_u1", etc. to repeated names (not including the first of each).
 *
 * \param msg[inout] Interactive marker for which control names are made unique.
 */
INTERACTIVE_MARKERS_PUBLIC
void uniqueifyControlNames(visualization_msgs::msg::InteractiveMarker & msg);

/// Make a default-style arrow marker.
/**
 * \param msg[in] The interactive marker that the arrow markers properties will be based on.
 * \param control[inout] The control where the arrow marker is inserted.
 * \param pos[in] How far from the center should the arrow be, and on which side.
 */
INTERACTIVE_MARKERS_PUBLIC
void makeArrow(
  const visualization_msgs::msg::InteractiveMarker & msg,
  visualization_msgs::msg::InteractiveMarkerControl & control,
  float pos);

/// Make a default-style disc marker (e.g for rotating).
/**
 * \param msg[in] The interactive marker that the disc markers properties will be based on.
 * \param control[inout] The control where the disc marker is inserted.
 * \param width[in] The width of the disc relative to its inner radius.
 */
INTERACTIVE_MARKERS_PUBLIC
void makeDisc(
  const visualization_msgs::msg::InteractiveMarker & msg,
  visualization_msgs::msg::InteractiveMarkerControl & control,
  double width = 0.3);

/// Make view facing button with text.
/**
 * \param msg[in] The interactive marker that the buttons properties will be based on.
 * \param control[inout] The control where the button is inserted.
 * \param text[in] The text to display on the button.
 */
INTERACTIVE_MARKERS_PUBLIC
void makeViewFacingButton(
  const visualization_msgs::msg::InteractiveMarker & msg,
  visualization_msgs::msg::InteractiveMarkerControl & control,
  std::string text);

/// Assign an RGB value to the given marker based on the given orientation.
/**
 * \param marker[inout] The marker to color.
 * \param quat[in] The orientation that determines the color.
 */
INTERACTIVE_MARKERS_PUBLIC
void assignDefaultColor(
  visualization_msgs::msg::Marker & marker,
  const geometry_msgs::msg::Quaternion & quat);

/// Create a control which shows the description of the interactive marker
/**
 * \param msg[in] The interactive marker to describe.
 * \return A control that shows the description of the provided interactive marker.
 */
INTERACTIVE_MARKERS_PUBLIC
visualization_msgs::msg::InteractiveMarkerControl makeTitle(
  const visualization_msgs::msg::InteractiveMarker & msg);

}  // namespace interactive_markers

#endif  // INTERACTIVE_MARKERS__TOOLS_HPP_
