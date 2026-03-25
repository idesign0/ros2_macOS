// -*-c++-*---------------------------------------------------------------------------------------
// Copyright 2024 Bernd Pfrommer <bernd.pfrommer@gmail.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef APRILTAG_DETECTOR__DETECTOR_HPP_
#define APRILTAG_DETECTOR__DETECTOR_HPP_

#include <apriltag_msgs/msg/april_tag_detection_array.hpp>
#include <string>

namespace cv
{
class Mat;
}

namespace apriltag_detector
{
class Detector
{
public:
  using ApriltagArray = apriltag_msgs::msg::AprilTagDetectionArray;
  virtual ~Detector() {}
  virtual void detect(const cv::Mat & img, ApriltagArray * tags) = 0;
  virtual void setFamily(const std::string & fam) = 0;
  virtual void setBlackBorder(int width) = 0;
  virtual void setDecimateFactor(double factor) = 0;
  virtual void setQuadSigma(double blur) = 0;
  virtual void setNumberOfThreads(int i) = 0;
  virtual void setMaxAllowedHammingDistance(int) = 0;
  virtual const std::string & getFamily() const = 0;
};
}  // namespace apriltag_detector
#endif  // APRILTAG_DETECTOR__DETECTOR_HPP_
