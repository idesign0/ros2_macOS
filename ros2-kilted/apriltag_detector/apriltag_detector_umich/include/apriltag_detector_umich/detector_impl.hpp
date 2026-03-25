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

#ifndef APRILTAG_DETECTOR_UMICH__DETECTOR_IMPL_HPP_
#define APRILTAG_DETECTOR_UMICH__DETECTOR_IMPL_HPP_

#include <apriltag_msgs/msg/april_tag_detection_array.hpp>
#include <rclcpp/rclcpp.hpp>
#include <string>

namespace cv
{
class Mat;
}

namespace apriltag_detector_umich
{
class DetectorImpl
{
public:
  using ApriltagArray = apriltag_msgs::msg::AprilTagDetectionArray;
  DetectorImpl();
  ~DetectorImpl();

  void detect(const cv::Mat & imgMsg, ApriltagArray * tags);
  void setFamily(const std::string & fam);
  void setDecimateFactor(double);
  void setQuadSigma(double);
  void setNumberOfThreads(int);
  void setMaxAllowedHammingDistance(int);
  const std::string & getFamily() const { return (family_); }

private:
  void makeDetector();
  void destroyDetector();
  void resetDetector();
  // ------------------------  variables ------------------------------
  std::string family_{"tf36h11"};
  int max_allowed_hamming_distance_{0};
  void * tag_family_{nullptr};
  void * detector_{nullptr};
};

}  // namespace apriltag_detector_umich
#endif  // APRILTAG_DETECTOR_UMICH__DETECTOR_IMPL_HPP_
