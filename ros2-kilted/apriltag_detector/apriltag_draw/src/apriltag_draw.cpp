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

#include <apriltag_draw/apriltag_draw.hpp>
#if __has_include(<cv_bridge/cv_bridge.hpp>)
#include <cv_bridge/cv_bridge.hpp>
#else
#include <cv_bridge/cv_bridge.h>
#endif
#include <opencv2/core/core.hpp>
#include <rclcpp_components/register_node_macro.hpp>

namespace apriltag_draw
{

static void draw_line(
  cv::Mat & img, double x0, double y0, double x1, double y1,
  const cv::Scalar & col, int lw)
{
  cv::line(
    img, {static_cast<int>(x0), static_cast<int>(y0)},
    {static_cast<int>(x1), static_cast<int>(y1)}, col, lw);
}

static void draw(
  cv::Mat & img, int id, const std::array<double, 2> & a,
  const std::array<std::array<double, 2>, 4> & c)
{
  int lw = 2;                                   // line width
  const auto X_COLOR = CV_RGB(255, 0, 0);       // red
  const auto Y_COLOR = CV_RGB(0, 255, 0);       // green
  const auto Z_COLOR = CV_RGB(0, 0, 255);       // blue
  const auto TEXT_COLOR = CV_RGB(255, 0, 255);  // purple
  draw_line(img, c[0][0], c[0][1], c[1][0], c[1][1], X_COLOR, 2 * lw);
  draw_line(img, c[0][0], c[0][1], c[3][0], c[3][1], Y_COLOR, lw);
  draw_line(img, c[2][0], c[2][1], c[3][0], c[3][1], Z_COLOR, lw);
  draw_line(img, c[1][0], c[1][1], c[2][0], c[2][1], Z_COLOR, lw);
  const auto lt = cv::LINE_AA;  // anti-aliased line

  // write id into center of tag
  const int x_off = 5;
  const int y_off = 5;
  cv::putText(
    img, std::to_string(id), cv::Point2f(a[0] - x_off, a[1] + y_off),
    cv::FONT_HERSHEY_SIMPLEX, 1, TEXT_COLOR, 2, lt);

  // draw corner circles
  const cv::Scalar cc[4] = {X_COLOR, Y_COLOR, Z_COLOR, TEXT_COLOR};
  const int R = 2;  // radius of corner circles
  for (int i = 0; i < 4; i++) {
    cv::circle(img, cv::Point2i(c[i][0], c[i][1]), R, cc[i], 1, lt);
  }
}

static cv::Mat draw_msg(const cv::Mat & img, const ApriltagArray & msg)
{
  cv::Mat colorImg;
  cv::cvtColor(img, colorImg, cv::COLOR_GRAY2BGR);
  const auto & all_tags = msg.detections;
  for (const auto & tag : all_tags) {
    const auto & c = tag.corners;
    const std::array<std::array<double, 2>, 4> corners{
      {{c[0].x, c[0].y}, {c[1].x, c[1].y}, {c[2].x, c[2].y}, {c[3].x, c[3].y}}};
    draw(colorImg, tag.id, {tag.centre.x, tag.centre.y}, corners);
  }
  return (colorImg);
}

static rmw_qos_profile_t string_to_profile(const std::string & s)
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

ApriltagDraw::ApriltagDraw(const rclcpp::NodeOptions & options)
: Node("apriltag_draw", options)
{
  transport_ = declare_parameter<std::string>("image_transport", "raw");
  qos_profile_ =
    string_to_profile(declare_parameter<std::string>("qos_profile", "default"));
  max_queue_size_ = declare_parameter<int>("max_queue_size", 200);
  // publish images
#ifdef USE_MATCHED_EVENTS
  rclcpp::PublisherOptions pub_options;
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
  image_pub_ = image_transport::create_publisher(
#ifdef IMAGE_TRANSPORT_USE_NODEINTERFACE
    *this,
#else
    this,
#endif
    "image_tags", convert_profile(rmw_qos_profile_default), pub_options);
#else
  image_pub_ = image_transport::create_publisher(
#ifdef IMAGE_TRANSPORT_USE_NODEINTERFACE
    *this,
#else
    this,
#endif
    "image_tags", convert_profile(rmw_qos_profile_default));

  // Since the early ROS2 image transport does not call back when
  // subscribers come and go: must check by polling
  subscription_check_timer_ = rclcpp::create_timer(
    this, get_clock(), rclcpp::Duration(1, 0),
    std::bind(&ApriltagDraw::subscriptionCheckTimerExpired, this));
#endif
}

ApriltagDraw::~ApriltagDraw()
{
  if (subscription_check_timer_) {
    subscription_check_timer_->cancel();
  }
}

void ApriltagDraw::subscribe()
{
  RCLCPP_INFO(this->get_logger(), "subscribing to tags and image!");
  tag_sub_ = create_subscription<ApriltagArray>(
    "tags", rclcpp::QoS(10),
    std::bind(&ApriltagDraw::tagCallback, this, std::placeholders::_1));
  image_sub_ = std::make_shared<image_transport::Subscriber>(
    image_transport::create_subscription(
#ifdef IMAGE_TRANSPORT_USE_NODEINTERFACE
      *this,
#else
      this,
#endif
      "image",
      std::bind(&ApriltagDraw::imageCallback, this, std::placeholders::_1),
      transport_, convert_profile(qos_profile_)));
  is_subscribed_ = true;
}

void ApriltagDraw::unsubscribe()
{
  tag_sub_.reset();
  image_sub_->shutdown();
  RCLCPP_INFO(this->get_logger(), "unsubscribing from tags and image!");
  is_subscribed_ = false;
}

void ApriltagDraw::subscriptionCheckTimerExpired()
{
  if (image_pub_.getNumSubscribers() != 0) {
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
void ApriltagDraw::processFrame(
  const ApriltagArray::ConstSharedPtr & tags, const Image::ConstSharedPtr & msg)
{
  if (image_pub_.getNumSubscribers() != 0) {
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
    cv::Mat color_img = draw_msg(cvImg->image, *tags);
    cv_bridge::CvImage pub_img(
      msg->header, sensor_msgs::image_encodings::BGR8, color_img);
    image_pub_.publish(pub_img.toImageMsg());
  }
}

void ApriltagDraw::imageCallback(const Image::ConstSharedPtr & img)
{
  if (img_buffer_.size() < static_cast<size_t>(max_queue_size_)) {
    img_buffer_.push_back(img);
  } else {
    RCLCPP_WARN_STREAM(
      get_logger(), "image buffer queue overrun: " << img_buffer_.size());
  }
  processBuffers();
}
void ApriltagDraw::tagCallback(const ApriltagArray::ConstSharedPtr & tags)
{
  if (tag_buffer_.size() < static_cast<size_t>(max_queue_size_)) {
    tag_buffer_.push_back(tags);
  } else {
    RCLCPP_WARN_STREAM(
      get_logger(), "tag buffer queue overrun: " << tag_buffer_.size());
  }
  processBuffers();
}

template <typename T>
rclcpp::Time get_time(const typename T::ConstSharedPtr & msg)
{
  return (rclcpp::Time(msg->header.stamp));
}

void ApriltagDraw::processBuffers()
{
  // Decided to write custom sync so people can see if messages are arriving.
  // If you run at high frame rate the detector can fall far behind and
  // the message_filter sync will just hang silently. By hand coding at least
  // there are warning messages when frames are dropped.

  if (tag_buffer_.empty()) {
    return;
  }
  while (!tag_buffer_.empty()) {
    const auto & tags = *(tag_buffer_.begin());
    const auto tag_time = get_time<ApriltagArray>(tags);
    // purge old images
    while (!img_buffer_.empty() &&
           get_time<Image>(*(img_buffer_.begin())) < tag_time) {
      img_buffer_.pop_front();
    }
    if (
      !img_buffer_.empty() &&
      get_time<Image>(*(img_buffer_.begin())) == tag_time) {
      processFrame(tags, *(img_buffer_.begin()));
      tag_buffer_.pop_front();
      img_buffer_.pop_front();
    }
    if (img_buffer_.empty()) {
      break;
    }
    // purge old tags
    const auto img_time = get_time<Image>(*(img_buffer_.begin()));
    while (!tag_buffer_.empty() &&
           get_time<ApriltagArray>(*(tag_buffer_.begin())) < img_time) {
      tag_buffer_.pop_front();
    }
  }
}

}  // namespace apriltag_draw

RCLCPP_COMPONENTS_REGISTER_NODE(apriltag_draw::ApriltagDraw)
