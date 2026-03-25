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

#include <apriltag_detector_umich/detector.hpp>
#include <apriltag_detector_umich/detector_impl.hpp>
#include <pluginlib/class_list_macros.hpp>

namespace apriltag_detector_umich
{
Detector::Detector() : detector_(new DetectorImpl()) {}

void Detector::setDecimateFactor(double f) { detector_->setDecimateFactor(f); }

void Detector::setQuadSigma(double blur) { detector_->setQuadSigma(blur); }

void Detector::setNumberOfThreads(int n) { detector_->setNumberOfThreads(n); }

void Detector::setMaxAllowedHammingDistance(int n)
{
  detector_->setMaxAllowedHammingDistance(n);
}
void Detector::setFamily(const std::string & fam) { detector_->setFamily(fam); }
const std::string & Detector::getFamily() const
{
  return (detector_->getFamily());
}

void Detector::detect(const cv::Mat & img, ApriltagArray * tags)
{
  detector_->detect(img, tags);
}
}  // namespace apriltag_detector_umich

PLUGINLIB_EXPORT_CLASS(
  apriltag_detector_umich::Detector, apriltag_detector::Detector)
