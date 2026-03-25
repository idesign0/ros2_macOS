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

#include <apriltag_detector/detector_component.hpp>
#if __has_include(<cv_bridge/cv_bridge.hpp>)
#include <cv_bridge/cv_bridge.hpp>
#else
#include <cv_bridge/cv_bridge.h>
#endif
#include <image_transport/subscriber.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_components/register_node_macro.hpp>

namespace apriltag_detector
{
DetectorComponent::DetectorComponent(const rclcpp::NodeOptions & options)
: Node(
    "detector", rclcpp::NodeOptions(options)
                  .automatically_declare_parameters_from_overrides(true)),
  detector_loader_("apriltag_detector", "apriltag_detector::Detector")
{
  const std::string type = this->get_parameter_or("type", std::string("umich"));
  RCLCPP_INFO_STREAM(get_logger(), "tag detector type: " << type);
  detector_ = detector_loader_.createSharedInstance(
    "apriltag_detector_" + type + "::Detector");
  detector_->setFamily(get_parameter_or("tag_family", std::string("tf36h11")));
  in_transport_ = get_parameter_or("image_transport", std::string("raw"));

  get_parameter_or(
    "image_qos_profile", image_qos_profile_, std::string("default"));

  // only supported by the UMich detector
  detector_->setDecimateFactor(get_parameter_or("decimate_factor", 1.0));
  detector_->setQuadSigma(get_parameter_or("blur", 0.0));
  detector_->setNumberOfThreads(get_parameter_or("num_threads", 1));
  detector_->setMaxAllowedHammingDistance(
    get_parameter_or("max_allowed_hamming_distance", 0));
  // only supported by the MIT detector
  detector_->setBlackBorder(get_parameter_or("black_border_width", 1));

  rclcpp::PublisherOptions pub_options;
#ifdef USE_MATCHED_EVENTS
  pub_options.event_callbacks.matched_callback =
    [this](rclcpp::MatchedInfo & s) {
      if (is_subscribed_) {
        if (s.current_count == 0) {
          unsubscribe();
        }
      } else {
        if (s.current_count != 0) {
          subscribe();
        }
      }
    };
#endif
  detect_pub_ = this->create_publisher<ApriltagArray>(
    "tags", rclcpp::QoS(100), pub_options);

#ifndef USE_MATCHED_EVENTS
  // Since early ROS2 does not call back when subscribers come and go
  // must check by polling
  subscription_check_timer_ = rclcpp::create_timer(
    this, get_clock(), rclcpp::Duration(1, 0),
    std::bind(&DetectorComponent::subscriptionCheckTimerExpired, this));
#endif
}

DetectorComponent::~DetectorComponent()
{
  if (subscription_check_timer_) {
    subscription_check_timer_->cancel();
  }
}

rmw_qos_profile_t string_to_profile(const std::string & s)
{
  if (s == "sensor_data") {
    return (rmw_qos_profile_sensor_data);
  }
  return (rmw_qos_profile_default);
}

#ifdef IMAGE_TRANSPORT_USE_QOS
static rclcpp::QoS convert_profile(const rmw_qos_profile_t & p)
{
  return (rclcpp::QoS(rclcpp::QoSInitialization::from_rmw(p), p));
}
#else
static const rmw_qos_profile_t & convert_profile(const rmw_qos_profile_t & p)
{
  return (p);
}
#endif

void DetectorComponent::subscribe()
{
  const auto profile = string_to_profile(image_qos_profile_);
  image_sub_ = std::make_shared<image_transport::Subscriber>(
    image_transport::create_subscription(
#ifdef IMAGE_TRANSPORT_USE_NODEINTERFACE
      *this,
#else
      this,
#endif
      "image",
      std::bind(&DetectorComponent::callback, this, std::placeholders::_1),
      in_transport_, convert_profile(profile)));
  is_subscribed_ = true;
}

void DetectorComponent::unsubscribe()
{
  image_sub_->shutdown();
  is_subscribed_ = false;
}

void DetectorComponent::subscriptionCheckTimerExpired()
{
  if (detect_pub_->get_subscription_count()) {
    // -------------- subscribers ---------------------
    if (!is_subscribed_) {
      subscribe();
    }
  } else {
    // -------------- no subscribers -------------------
    if (is_subscribed_) {
      unsubscribe();
    }
  }
}

void DetectorComponent::callback(
  const sensor_msgs::msg::Image::ConstSharedPtr & msg)
{
  num_messages_++;
  if (detect_pub_->get_subscription_count() != 0) {
    cv_bridge::CvImageConstPtr cvImg;
    try {
      cvImg = cv_bridge::toCvShare(msg, "mono8");
    } catch (const cv_bridge::Exception & e) {
      if (msg->encoding == "8UC1") {
        // hack to muddle on when encoding is wrong
        std::shared_ptr<Image> img_copy(new Image(*msg));
        img_copy->encoding = "mono8";
        cvImg = cv_bridge::toCvShare(img_copy, "mono8");
      }
    }
    if (!cvImg) {
      RCLCPP_WARN(get_logger(), "cannot convert image to mono!");
      return;
    }
    auto array_msg =
      std::make_unique<apriltag_msgs::msg::AprilTagDetectionArray>();
    detector_->detect(cvImg->image, array_msg.get());
    array_msg->header = msg->header;
    num_tags_detected_ += array_msg->detections.size();
    detect_pub_->publish(std::move(array_msg));
  }
}
}  // namespace apriltag_detector

RCLCPP_COMPONENTS_REGISTER_NODE(apriltag_detector::DetectorComponent)
