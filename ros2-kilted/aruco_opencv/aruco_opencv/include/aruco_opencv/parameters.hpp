// Copyright 2022-2025 Fictionlab sp. z o.o.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#pragma once

#include <map>
#include <string>
#include <vector>

#include <opencv2/aruco.hpp>
#include <opencv2/calib3d.hpp>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"

namespace aruco_opencv
{

struct CoreParams
{
  std::string cam_base_topic;
  bool image_is_rectified;
  std::string output_frame;
  std::string marker_dict;
  bool image_sub_compressed;
  int qos_rel;
  int qos_dur;
  int qos_depth;
  bool publish_tf;
  std::string board_descriptions_path;
};

/// @brief Strategy for selecting the best pose among multiple candidates
enum class PoseSelectorStrategy
{
  /// Select pose with the lowest reprojection error
  REPROJECTION_ERROR,
  /// Select pose with the plane normal most parallel to camera view direction
  PLANE_NORMAL_PARALLEL,
};

/// @brief Configuration for pose selection
struct PoseSelectorConfig
{
  /// Strategy to use for pose selection
  PoseSelectorStrategy strategy = PoseSelectorStrategy::REPROJECTION_ERROR;
  /// Enable debug output
  bool debug = false;
};

struct DetectorParams
{
  double marker_size;
  PoseSelectorConfig pose_selector{};
};

void declare_all_parameters(rclcpp_lifecycle::LifecycleNode & node);
void declare_core_parameters(rclcpp_lifecycle::LifecycleNode & node);
void declare_aruco_parameters(rclcpp_lifecycle::LifecycleNode & node);
void declare_detector_parameters(rclcpp_lifecycle::LifecycleNode & node);

CoreParams retrieve_core_parameters(rclcpp_lifecycle::LifecycleNode & node);
void retrieve_aruco_parameters(
  rclcpp_lifecycle::LifecycleNode & node,
  cv::Ptr<cv::aruco::DetectorParameters> & detector_parameters,
  bool log_values = false);
DetectorParams retrieve_detector_parameters(rclcpp_lifecycle::LifecycleNode & node);

rcl_interfaces::msg::SetParametersResult validate_core_parameters(
  const std::vector<rclcpp::Parameter> & parameters);
rcl_interfaces::msg::SetParametersResult validate_detector_parameters(
  const std::vector<rclcpp::Parameter> & parameters);

void update_dynamic_parameters(
  rclcpp_lifecycle::LifecycleNode & node,
  const std::vector<rclcpp::Parameter> & parameters,
  DetectorParams & detector_params,
  cv::Ptr<cv::aruco::DetectorParameters> & aruco_parameters);

}  // namespace aruco_opencv
