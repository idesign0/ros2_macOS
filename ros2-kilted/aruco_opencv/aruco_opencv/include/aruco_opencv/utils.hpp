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

#include <string>
#include <unordered_map>

#include <opencv2/aruco.hpp>
#include <opencv2/calib3d.hpp>

#include "geometry_msgs/msg/pose.hpp"
#include "cv_bridge/cv_bridge.hpp"

#include "aruco_opencv/parameters.hpp"

namespace aruco_opencv
{

geometry_msgs::msg::Pose convert_rvec_tvec(const cv::Vec3d & rvec, const cv::Vec3d & tvec);

#if CV_VERSION_MAJOR > 4 || CV_VERSION_MAJOR == 4 && CV_VERSION_MINOR >= 7
using ArucoDictType = cv::aruco::PredefinedDictionaryType;
#else
using ArucoDictType = cv::aruco::PREDEFINED_DICTIONARY_NAME;
#endif

extern const std::unordered_map<std::string, ArucoDictType> ARUCO_DICT_MAP;

PoseSelectorStrategy parse_selector_strategy(const std::string & name);
std::string pose_selector_strategy_to_string(PoseSelectorStrategy strategy);

}  // namespace aruco_opencv
