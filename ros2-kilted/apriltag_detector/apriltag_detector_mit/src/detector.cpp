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

#include <apriltag_mit/apriltag_mit.h>

#include <apriltag_detector_mit/detector.hpp>
#include <pluginlib/class_list_macros.hpp>

static rclcpp::Logger get_logger()
{
  return (rclcpp::get_logger("mit_detector"));
}

static const std::unordered_map<std::string, AprilTags::TagCodes>
  family_code_map = {
    {{"tf36h11", AprilTags::tag_codes_36h11},
     {"tf25h9", AprilTags::tag_codes_25h9},
     {"tf16h5", AprilTags::tag_codes_16h5}}};

static std::shared_ptr<AprilTags::TagDetector> makeDetector(
  const std::string & fam)
{
  const auto it = family_code_map.find(fam);
  if (it == family_code_map.end()) {
    RCLCPP_ERROR_STREAM(get_logger(), "invalid tag family: " << fam);
    throw std::runtime_error("invalid tag family: " + fam);
  }
  return (std::make_shared<AprilTags::TagDetector>(it->second));
}

namespace apriltag_detector_mit
{

Detector::Detector()
{
  detector_ = makeDetector(family_);
  detector_->set_black_border(black_border_bits_);
}

Detector::~Detector() {}

void Detector::setFamily(const std::string & fam)
{
  if (fam != family_) {
    family_ = fam;
    detector_ = makeDetector(fam);  // frees old one
    detector_->set_black_border(black_border_bits_);
  }
}

void Detector::setBlackBorder(int width)
{
  black_border_bits_ = width;
  detector_->set_black_border(black_border_bits_);
}

void Detector::detect(const cv::Mat & img, ApriltagArray * tags)
{
  auto detections = detector_->ExtractTags(img);

  // convert to  Apriltag message
  tags->detections.resize(detections.size());

  for (size_t i = 0; i < detections.size(); i++) {
    const AprilTags::TagDetection & td = detections[i];
    auto & apriltag = tags->detections[i];
    apriltag.family = family_;
    apriltag.id = td.id;
    apriltag.hamming = td.hamming_distance;
    apriltag.goodness = 0;         // what is that?,
    apriltag.decision_margin = 0;  // not supported by MIT detector
    apriltag.centre.x = td.cxy.x;
    apriltag.centre.y = td.cxy.y;
    for (size_t c = 0; c < 4; ++c) {
      apriltag.corners[c].x = td.p[c].x;
      apriltag.corners[c].y = td.p[c].y;
    }
  }
}
}  // namespace apriltag_detector_mit

PLUGINLIB_EXPORT_CLASS(
  apriltag_detector_mit::Detector, apriltag_detector::Detector)
