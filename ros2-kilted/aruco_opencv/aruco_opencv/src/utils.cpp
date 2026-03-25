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

#include "tf2/convert.hpp"
#include "tf2_geometry_msgs/tf2_geometry_msgs.hpp"

#include "aruco_opencv/parameters.hpp"
#include "aruco_opencv/utils.hpp"

namespace aruco_opencv
{

geometry_msgs::msg::Pose convert_rvec_tvec(const cv::Vec3d & rvec, const cv::Vec3d & tvec)
{
  geometry_msgs::msg::Pose pose_out;

  cv::Mat rot(3, 3, CV_64FC1);
  cv::Rodrigues(rvec, rot);

  tf2::Matrix3x3 tf_rot(rot.at<double>(0, 0), rot.at<double>(0, 1), rot.at<double>(0, 2),
    rot.at<double>(1, 0), rot.at<double>(1, 1), rot.at<double>(1, 2),
    rot.at<double>(2, 0), rot.at<double>(2, 1), rot.at<double>(2, 2));
  tf2::Quaternion tf_quat;
  tf_rot.getRotation(tf_quat);

  pose_out.position.x = tvec[0];
  pose_out.position.y = tvec[1];
  pose_out.position.z = tvec[2];
  tf2::convert(tf_quat, pose_out.orientation);

  return pose_out;
}

const std::unordered_map<std::string, ArucoDictType> ARUCO_DICT_MAP = {
  {"4X4_50", ArucoDictType::DICT_4X4_50},
  {"4X4_100", ArucoDictType::DICT_4X4_100},
  {"4X4_250", ArucoDictType::DICT_4X4_250},
  {"4X4_1000", ArucoDictType::DICT_4X4_1000},
  {"5X5_50", ArucoDictType::DICT_5X5_50},
  {"5X5_100", ArucoDictType::DICT_5X5_100},
  {"5X5_250", ArucoDictType::DICT_5X5_250},
  {"5X5_1000", ArucoDictType::DICT_5X5_1000},
  {"6X6_50", ArucoDictType::DICT_6X6_50},
  {"6X6_100", ArucoDictType::DICT_6X6_100},
  {"6X6_250", ArucoDictType::DICT_6X6_250},
  {"6X6_1000", ArucoDictType::DICT_6X6_1000},
  {"7X7_50", ArucoDictType::DICT_7X7_50},
  {"7X7_100", ArucoDictType::DICT_7X7_100},
  {"7X7_250", ArucoDictType::DICT_7X7_250},
  {"7X7_1000", ArucoDictType::DICT_7X7_1000},
  {"ARUCO_ORIGINAL", ArucoDictType::DICT_ARUCO_ORIGINAL},
  {"APRILTAG_16h5", ArucoDictType::DICT_APRILTAG_16h5},
  {"APRILTAG_25h9", ArucoDictType::DICT_APRILTAG_25h9},
  {"APRILTAG_36h10", ArucoDictType::DICT_APRILTAG_36h10},
  {"APRILTAG_36h11", ArucoDictType::DICT_APRILTAG_36h11},
};

PoseSelectorStrategy parse_selector_strategy(const std::string & name)
{
  if (name == "PLANE_NORMAL_PARALLEL") {
    return PoseSelectorStrategy::PLANE_NORMAL_PARALLEL;
  }
  return PoseSelectorStrategy::REPROJECTION_ERROR;
}

std::string pose_selector_strategy_to_string(PoseSelectorStrategy strategy)
{
  switch (strategy) {
    case PoseSelectorStrategy::REPROJECTION_ERROR:
      return "REPROJECTION_ERROR";
    case PoseSelectorStrategy::PLANE_NORMAL_PARALLEL:
      return "PLANE_NORMAL_PARALLEL";
    default:
      return "UNKNOWN";
  }
}

}  // namespace aruco_opencv
