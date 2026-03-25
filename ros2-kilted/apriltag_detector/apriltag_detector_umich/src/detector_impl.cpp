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

#include <apriltag.h>
#include <tag16h5.h>
#include <tag25h9.h>
#include <tag36h11.h>
#include <tagCircle21h7.h>
#include <tagCircle49h12.h>
#include <tagCustom48h12.h>
#include <tagStandard41h12.h>
#include <tagStandard52h13.h>

#include <apriltag_detector_umich/detector_impl.hpp>
#include <opencv2/core/core.hpp>
#include <pluginlib/class_list_macros.hpp>

static rclcpp::Logger get_logger()
{
  return (rclcpp::get_logger("apriltag_umich"));
}

static apriltag_detector_t * recast(void * p)
{
  return (reinterpret_cast<apriltag_detector_t *>(p));
}

static apriltag_family_t * to_fam(void * p)
{
  return (reinterpret_cast<apriltag_family_t *>(p));
}

static apriltag_family * make_tag_family(const std::string & name)
{
  apriltag_family * tf{0};
  if (name == "tf36h11") {
    tf = tag36h11_create();
  } else if (name == "tf25h9") {
    tf = tag25h9_create();
  } else if (name == "tf16h5") {
    tf = tag16h5_create();
  } else if (name == "tfCircle21h7") {
    tf = tagCircle21h7_create();
  } else if (name == "tfCircle49h12") {
    tf = tagCircle49h12_create();
  } else if (name == "tfStandard41h12") {
    tf = tagStandard41h12_create();
  } else if (name == "tfStandard52h13") {
    tf = tagStandard52h13_create();
  } else if (name == "tfCustom48h12") {
    tf = tagCustom48h12_create();
  } else {
    tf = NULL;
  }
  return (tf);
}

static void destroy_tag_family(const std::string & name, apriltag_family * tf)
{
  if (name == "tf36h11") {
    tag36h11_destroy(tf);
  } else if (name == "tf25h9") {
    tag25h9_destroy(tf);
  } else if (name == "tf16h5") {
    tag16h5_destroy(tf);
  } else if (name == "tfCircle21h7") {
    tagCircle21h7_destroy(tf);
  } else if (name == "tfCircle49h12") {
    tagCircle49h12_destroy(tf);
  } else if (name == "tfStandard41h12") {
    tagStandard41h12_destroy(tf);
  } else if (name == "tfStandard52h13") {
    tagStandard52h13_destroy(tf);
  } else if (name == "tfCustom48h12") {
    tagCustom48h12_destroy(tf);
  }
}

namespace apriltag_detector_umich
{

DetectorImpl::DetectorImpl() { makeDetector(); }

DetectorImpl::~DetectorImpl() { destroyDetector(); }

void DetectorImpl::resetDetector()
{
  destroyDetector();
  makeDetector();
}

void DetectorImpl::makeDetector()
{
  detector_ = apriltag_detector_create();
  tag_family_ = make_tag_family(family_);
  if (!tag_family_) {
    RCLCPP_ERROR_STREAM(get_logger(), "invalid tag family: " << family_);
    throw(std::runtime_error("invalid tag family specified!"));
  }
  apriltag_detector_add_family_bits(
    recast(detector_), to_fam(tag_family_), max_allowed_hamming_distance_);
}

void DetectorImpl::destroyDetector()
{
  if (detector_) {
    apriltag_detector_destroy(recast(detector_));
    detector_ = nullptr;
  }
  if (tag_family_) {
    destroy_tag_family(family_, to_fam(tag_family_));
    tag_family_ = nullptr;
  }
}

void DetectorImpl::setDecimateFactor(double f)
{
  recast(detector_)->quad_decimate = f;
}

void DetectorImpl::setQuadSigma(double blur)
{
  recast(detector_)->quad_sigma = blur;
}

void DetectorImpl::setNumberOfThreads(int n)
{
  recast(detector_)->nthreads = n;
}

void DetectorImpl::setFamily(const std::string & fam)
{
  if (family_ != fam) {
    family_ = fam;
    resetDetector();
  }
}

void DetectorImpl::setMaxAllowedHammingDistance(int h)
{
  max_allowed_hamming_distance_ = h;
  resetDetector();
}

void DetectorImpl::detect(const cv::Mat & img, ApriltagArray * tags)
{
  image_u8_t imgu8{
    img.cols, img.rows, static_cast<int32_t>(img.step), img.data};
  zarray_t * detections = apriltag_detector_detect(recast(detector_), &imgu8);
  const auto numDetections = zarray_size(detections);
  for (int i = 0; i < numDetections; ++i) {
    apriltag_detection_t * t;
    zarray_get(detections, i, &t);
    apriltag_msgs::msg::AprilTagDetection tag;
    tag.id = t->id;
    tag.hamming = t->hamming;
    tag.family = family_;
    tag.decision_margin = t->decision_margin;
    for (int j = 0; j < 4; ++j) {
      tag.corners[j].x = t->p[j][0];
      tag.corners[j].y = t->p[j][1];
    }
    tag.goodness = 0;  // what is that?,
    tag.centre.x = t->c[0];
    tag.centre.y = t->c[1];
    tags->detections.push_back(tag);
  }
  apriltag_detections_destroy(detections);
}
}  // namespace apriltag_detector_umich
