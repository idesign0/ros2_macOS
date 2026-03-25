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

#ifndef APRILTAG_DETECTOR_UMICH__DETECTOR_HPP_
#define APRILTAG_DETECTOR_UMICH__DETECTOR_HPP_

#include <apriltag_detector/detector.hpp>
#include <memory>
#include <rclcpp/rclcpp.hpp>
#include <string>

namespace apriltag_detector_umich
{
class DetectorImpl;  // forward decl
class Detector : public apriltag_detector::Detector
{
public:
  Detector();
  void detect(const cv::Mat & img, ApriltagArray * tags) final;
  void setFamily(const std::string & fam) final;
  void setBlackBorder(int) final {}
  void setDecimateFactor(double) final;
  void setQuadSigma(double) final;
  void setNumberOfThreads(int) final;
  void setMaxAllowedHammingDistance(int) final;
  const std::string & getFamily() const final;

private:
  std::shared_ptr<DetectorImpl> detector_{nullptr};
};
}  // namespace apriltag_detector_umich
#endif  // APRILTAG_DETECTOR_UMICH__DETECTOR_HPP_
