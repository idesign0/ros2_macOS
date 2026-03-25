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

#include <mutex>
#include <chrono>

#include <opencv2/aruco.hpp>
#include <opencv2/calib3d.hpp>

#include "yaml-cpp/yaml.h"

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "cv_bridge/cv_bridge.hpp"
#include "tf2_geometry_msgs/tf2_geometry_msgs.hpp"
#include "tf2_ros/buffer.h"
#include "tf2_ros/transform_listener.h"
#include "tf2_ros/transform_broadcaster.h"
#include "rcl_interfaces/msg/set_parameters_result.hpp"
#include "sensor_msgs/msg/camera_info.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "image_transport/camera_common.hpp"

#include "aruco_opencv_msgs/msg/aruco_detection.hpp"
#include "aruco_opencv_msgs/msg/board_pose.hpp"

#include "aruco_opencv/utils.hpp"
#include "aruco_opencv/parameters.hpp"
#include "aruco_opencv/detector.hpp"
#include "aruco_opencv/board_loader.hpp"

using rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface;

namespace aruco_opencv
{

class ArucoTracker : public rclcpp_lifecycle::LifecycleNode
{
  // Parameters
  CoreParams params_;
  DetectorParams detector_params_;
  cv::Ptr<cv::aruco::DetectorParameters> aruco_parameters_;
  bool transform_poses_;

  // ROS
  OnSetParametersCallbackHandle::SharedPtr on_set_parameter_callback_handle_;
  PostSetParametersCallbackHandle::SharedPtr post_set_parameter_callback_handle_;
  rclcpp_lifecycle::LifecyclePublisher<aruco_opencv_msgs::msg::ArucoDetection>::SharedPtr
    detection_pub_;
  rclcpp_lifecycle::LifecyclePublisher<sensor_msgs::msg::Image>::SharedPtr debug_pub_;
  rclcpp::Subscription<sensor_msgs::msg::CameraInfo>::SharedPtr cam_info_sub_;
  rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr img_sub_;
  rclcpp::Subscription<sensor_msgs::msg::CompressedImage>::SharedPtr compressed_img_sub_;
  rclcpp::Time last_msg_stamp_;
  bool cam_info_retrieved_ = false;
  rclcpp::Time callback_start_time_;

  // Aruco
  std::vector<std::pair<std::string, cv::Ptr<cv::aruco::Board>>> boards_;
  std::unique_ptr<ArucoDetector> detector_;

  // Tf2
  std::shared_ptr<tf2_ros::Buffer> tf_buffer_;
  std::shared_ptr<tf2_ros::TransformListener> tf_listener_;
  std::shared_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;

public:
  explicit ArucoTracker(rclcpp::NodeOptions options)
  : LifecycleNode("aruco_tracker", options)
  {
    declare_parameters();
  }

  LifecycleNodeInterface::CallbackReturn on_configure(const rclcpp_lifecycle::State &)
  {
    RCLCPP_INFO(get_logger(), "Configuring");

    #if CV_VERSION_MAJOR > 4 || CV_VERSION_MAJOR == 4 && CV_VERSION_MINOR >= 7
    aruco_parameters_ = cv::makePtr<cv::aruco::DetectorParameters>();
    #else
    aruco_parameters_ = cv::aruco::DetectorParameters::create();
    #endif

    retrieve_parameters();

    if (ARUCO_DICT_MAP.find(params_.marker_dict) == ARUCO_DICT_MAP.end()) {
      RCLCPP_ERROR_STREAM(get_logger(), "Unsupported dictionary name: " << params_.marker_dict);
      return LifecycleNodeInterface::CallbackReturn::FAILURE;
    }

    detector_ = std::make_unique<ArucoDetector>(get_logger().get_child("ArucoDetector"));
    detector_->set_dictionary(params_.marker_dict);
    detector_->set_detector_parameters(detector_params_);
    detector_->set_aruco_parameters(aruco_parameters_);

    if (!params_.board_descriptions_path.empty()) {
      load_boards();
    }
    detector_->set_boards(boards_);

    if (params_.publish_tf) {
      tf_broadcaster_ = std::make_shared<tf2_ros::TransformBroadcaster>(*this);
    }

    detection_pub_ = create_publisher<aruco_opencv_msgs::msg::ArucoDetection>(
      "aruco_detections", 5);
    debug_pub_ = create_publisher<sensor_msgs::msg::Image>("~/debug", 5);

    return LifecycleNodeInterface::CallbackReturn::SUCCESS;
  }

  LifecycleNodeInterface::CallbackReturn on_activate(const rclcpp_lifecycle::State & state)
  {
    RCLCPP_INFO(get_logger(), "Activating");

    if (transform_poses_) {
      tf_buffer_ = std::make_shared<tf2_ros::Buffer>(get_clock());
      tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);
    }

    LifecycleNode::on_activate(state);

    detection_pub_->on_activate();
    debug_pub_->on_activate();

    on_set_parameter_callback_handle_ = add_on_set_parameters_callback(
      std::bind(&ArucoTracker::callback_on_set_parameters, this, std::placeholders::_1));
    post_set_parameter_callback_handle_ = add_post_set_parameters_callback(
      std::bind(&ArucoTracker::callback_post_set_parameters, this, std::placeholders::_1));

    RCLCPP_INFO(get_logger(), "Waiting for first camera info...");

    cam_info_retrieved_ = false;

    std::string image_topic = rclcpp::expand_topic_or_service_name(
      params_.cam_base_topic, this->get_name(), this->get_namespace());
    std::string cam_info_topic = image_transport::getCameraInfoTopic(image_topic);

    cam_info_sub_ = create_subscription<sensor_msgs::msg::CameraInfo>(
      cam_info_topic, 1,
      std::bind(&ArucoTracker::callback_camera_info, this, std::placeholders::_1));

    rmw_qos_profile_t image_sub_qos = rmw_qos_profile_default;
    image_sub_qos.reliability =
      static_cast<rmw_qos_reliability_policy_t>(params_.qos_rel);
    image_sub_qos.durability = static_cast<rmw_qos_durability_policy_t>(params_.qos_dur);
    image_sub_qos.depth = params_.qos_depth;

    auto qos = rclcpp::QoS(rclcpp::QoSInitialization::from_rmw(image_sub_qos), image_sub_qos);

    if (params_.image_sub_compressed) {
      compressed_img_sub_ = create_subscription<sensor_msgs::msg::CompressedImage>(
        image_topic + "/compressed", qos, std::bind(
          &ArucoTracker::callback_compressed_image, this, std::placeholders::_1));
    } else {
      img_sub_ = create_subscription<sensor_msgs::msg::Image>(
        image_topic, qos, std::bind(
          &ArucoTracker::callback_image, this, std::placeholders::_1));
    }

    return LifecycleNodeInterface::CallbackReturn::SUCCESS;
  }

  LifecycleNodeInterface::CallbackReturn on_deactivate(const rclcpp_lifecycle::State & state)
  {
    RCLCPP_INFO(get_logger(), "Deactivating");

    on_set_parameter_callback_handle_.reset();
    post_set_parameter_callback_handle_.reset();
    cam_info_sub_.reset();
    img_sub_.reset();
    compressed_img_sub_.reset();
    tf_listener_.reset();
    tf_buffer_.reset();

    detection_pub_->on_deactivate();
    debug_pub_->on_deactivate();

    return LifecycleNodeInterface::CallbackReturn::SUCCESS;
  }

  LifecycleNodeInterface::CallbackReturn on_cleanup(const rclcpp_lifecycle::State &)
  {
    RCLCPP_INFO(get_logger(), "Cleaning up");

    tf_broadcaster_.reset();
    aruco_parameters_.reset();
    detector_.reset();
    detection_pub_.reset();
    debug_pub_.reset();
    boards_.clear();

    return LifecycleNodeInterface::CallbackReturn::SUCCESS;
  }

  LifecycleNodeInterface::CallbackReturn on_shutdown(const rclcpp_lifecycle::State & state)
  {
    RCLCPP_INFO(get_logger(), "Shutting down");

    on_set_parameter_callback_handle_.reset();
    post_set_parameter_callback_handle_.reset();
    cam_info_sub_.reset();
    img_sub_.reset();
    compressed_img_sub_.reset();
    tf_listener_.reset();
    tf_buffer_.reset();
    tf_broadcaster_.reset();
    aruco_parameters_.reset();
    detector_.reset();
    detection_pub_.reset();
    debug_pub_.reset();
    boards_.clear();

    return LifecycleNodeInterface::CallbackReturn::SUCCESS;
  }

protected:
  void declare_parameters()
  {
    declare_all_parameters(*this);
  }

  void retrieve_parameters()
  {
    params_ = retrieve_core_parameters(*this);
    detector_params_ = retrieve_detector_parameters(*this);

    RCLCPP_INFO_STREAM(
      get_logger(), "Assume images are rectified: " << (params_.image_is_rectified ? "YES" : "NO"));
    if (params_.output_frame.empty()) {
      RCLCPP_INFO(get_logger(), "Marker detections will be published in the camera frame");
      transform_poses_ = false;
    } else {
      RCLCPP_INFO(
        get_logger(), "Marker detections will be transformed to \'%s\' frame",
        params_.output_frame.c_str());
      transform_poses_ = true;
    }
    RCLCPP_INFO_STREAM(get_logger(),
        "TF publishing is " << (params_.publish_tf ? "enabled" : "disabled"));
    RCLCPP_INFO_STREAM(get_logger(), "Marker size: " << detector_params_.marker_size << " meters");
    RCLCPP_INFO_STREAM(get_logger(),
        "Pose selector strategy: " <<
        pose_selector_strategy_to_string(detector_params_.pose_selector.strategy));
    RCLCPP_INFO(get_logger(), "Aruco Parameters:");

    retrieve_aruco_parameters(*this, aruco_parameters_, true);
  }

  rcl_interfaces::msg::SetParametersResult callback_on_set_parameters(
    const std::vector<rclcpp::Parameter> & parameters)
  {
    auto result = validate_core_parameters(parameters);
    if (!result.successful) {
      RCLCPP_ERROR_STREAM(get_logger(), result.reason);
    }
    result = validate_detector_parameters(parameters);
    if (!result.successful) {
      RCLCPP_ERROR_STREAM(get_logger(), result.reason);
    }
    return result;
  }

  void callback_post_set_parameters(const std::vector<rclcpp::Parameter> & parameters)
  {
    update_dynamic_parameters(*this, parameters, detector_params_, aruco_parameters_);

    detector_->set_detector_parameters(detector_params_);
    detector_->set_aruco_parameters(aruco_parameters_);
  }

  void load_boards()
  {
    RCLCPP_INFO_STREAM(get_logger(),
        "Trying to load board descriptions from " << params_.board_descriptions_path);
    std::string err;
    std::vector<std::pair<std::string, cv::Ptr<cv::aruco::Board>>> loaded;
    if (!BoardLoader::load_from_file(params_.board_descriptions_path, detector_->get_dictionary(),
        loaded, err))
    {
      RCLCPP_ERROR_STREAM(get_logger(), err);
      return;
    }
    boards_ = std::move(loaded);
    for (const auto & b : boards_) {
      RCLCPP_INFO_STREAM(get_logger(),
          "Successfully loaded configuration for board '" << b.first << "'");
    }
  }

  void callback_camera_info(const sensor_msgs::msg::CameraInfo::ConstSharedPtr cam_info)
  {
    detector_->update_camera_info(*cam_info, params_.image_is_rectified);

    if (!cam_info_retrieved_) {
      RCLCPP_INFO(get_logger(), "First camera info retrieved.");
      cam_info_retrieved_ = true;
    }
  }

  void callback_compressed_image(const sensor_msgs::msg::CompressedImage::ConstSharedPtr img_msg)
  {
    if (!should_process_img_msg(img_msg)) {
      return;
    }

    auto cv_ptr = cv_bridge::toCvCopy(img_msg, "bgr8");
    process_image(cv_ptr);
  }

  void callback_image(const sensor_msgs::msg::Image::ConstSharedPtr img_msg)
  {
    if (!should_process_img_msg(img_msg)) {
      return;
    }

    auto cv_ptr = cv_bridge::toCvShare(img_msg);
    process_image(cv_ptr);
  }

  template<typename ImgMsgT>
  bool should_process_img_msg(ImgMsgT img_msg)
  {
    RCLCPP_DEBUG_STREAM(get_logger(), "Image message address [SUBSCRIBE]:\t" << img_msg.get());

    if (!cam_info_retrieved_) {
      RCLCPP_DEBUG(get_logger(), "Camera info not retrieved yet. Ignoring image...");
      return false;
    }

    if (img_msg->header.stamp == last_msg_stamp_) {
      RCLCPP_DEBUG(
        get_logger(),
        "The new image has the same timestamp as the previous one (duplicate frame?). Ignoring...");
      return false;
    }

    last_msg_stamp_ = img_msg->header.stamp;

    // We're ready to go, remember the current time to measure callback performance.
    callback_start_time_ = get_clock()->now();

    return true;
  }

  void process_image(const cv_bridge::CvImageConstPtr & cv_ptr)
  {
    std::vector<int> marker_ids;
    std::vector<std::vector<cv::Point2f>> marker_corners;
    detector_->detect(cv_ptr->image, marker_ids, marker_corners);

    int n_markers = marker_ids.size();
    std::vector<cv::Vec3d> rvec_final, tvec_final;

    aruco_opencv_msgs::msg::ArucoDetection detection;
    detection.header.frame_id = cv_ptr->header.frame_id;
    detection.header.stamp = cv_ptr->header.stamp;

    detector_->estimate_marker_poses(marker_ids, marker_corners, detection.markers, rvec_final,
        tvec_final);

    detector_->estimate_board_poses(marker_ids, marker_corners, detection.boards, rvec_final,
        tvec_final);

    if (transform_poses_ && (detection.markers.size() > 0 || detection.boards.size() > 0)) {
      detection.header.frame_id = params_.output_frame;
      geometry_msgs::msg::TransformStamped cam_to_output;
      // Retrieve camera -> output_frame transform
      try {
        cam_to_output = tf_buffer_->lookupTransform(
          params_.output_frame, cv_ptr->header.frame_id,
          cv_ptr->header.stamp, rclcpp::Duration::from_seconds(1.0));
      } catch (tf2::TransformException & ex) {
        RCLCPP_ERROR_STREAM(get_logger(), ex.what());
        return;
      }
      for (auto & marker_pose : detection.markers) {
        tf2::doTransform(marker_pose.pose, marker_pose.pose, cam_to_output);
      }
      for (auto & board_pose : detection.boards) {
        tf2::doTransform(board_pose.pose, board_pose.pose, cam_to_output);
      }
    }

    if (params_.publish_tf && detection.markers.size() > 0) {
      std::vector<geometry_msgs::msg::TransformStamped> transforms;
      for (auto & marker_pose : detection.markers) {
        geometry_msgs::msg::TransformStamped transform;
        transform.header.stamp = detection.header.stamp;
        transform.header.frame_id = detection.header.frame_id;
        transform.child_frame_id = std::string("marker_") + std::to_string(marker_pose.marker_id);
        tf2::Transform tf_transform;
        tf2::fromMsg(marker_pose.pose, tf_transform);
        transform.transform = tf2::toMsg(tf_transform);
        transforms.push_back(transform);
      }
      for (auto & board_pose : detection.boards) {
        geometry_msgs::msg::TransformStamped transform;
        transform.header.stamp = detection.header.stamp;
        transform.header.frame_id = detection.header.frame_id;
        transform.child_frame_id = std::string("board_") + board_pose.board_name;
        tf2::Transform tf_transform;
        tf2::fromMsg(board_pose.pose, tf_transform);
        transform.transform = tf2::toMsg(tf_transform);
        transforms.push_back(transform);
      }
      tf_broadcaster_->sendTransform(transforms);
    }

    detection_pub_->publish(detection);

    if (debug_pub_->get_subscription_count() > 0) {
      auto debug_cv_ptr = std::make_shared<cv_bridge::CvImage>();
      debug_cv_ptr->header = cv_ptr->header;
      debug_cv_ptr->encoding = cv_ptr->encoding;
      debug_cv_ptr->image = cv_ptr->image.clone();
      cv::aruco::drawDetectedMarkers(debug_cv_ptr->image, marker_corners, marker_ids);
      {
        cv::Mat camera_matrix, distortion_coeffs;
        detector_->get_intrinsics(camera_matrix, distortion_coeffs);
        for (size_t i = 0; i < rvec_final.size(); i++) {
          cv::drawFrameAxes(
            debug_cv_ptr->image, camera_matrix, distortion_coeffs, rvec_final[i],
            tvec_final[i], 0.2, 3);
        }
      }
      std::unique_ptr<sensor_msgs::msg::Image> debug_img =
        std::make_unique<sensor_msgs::msg::Image>();
      debug_cv_ptr->toImageMsg(*debug_img);
      debug_pub_->publish(std::move(debug_img));
    }

    auto callback_end_time = get_clock()->now();
    double whole_callback_duration = (callback_end_time - callback_start_time_).seconds();
    double image_send_duration = (callback_start_time_ - cv_ptr->header.stamp).seconds();

    RCLCPP_DEBUG(
      get_logger(), "Image callback completed. The callback started %.4f s after the image"
      " frame was grabbed and completed its execution in %.4f s.", image_send_duration,
      whole_callback_duration);
  }
};

class ArucoTrackerAutostart : public ArucoTracker
{
public:
  explicit ArucoTrackerAutostart(rclcpp::NodeOptions options)
  : ArucoTracker(options)
  {
    auto new_state = configure();
    if (new_state.label() == "inactive") {
      activate();
    }
  }
};

}  // namespace aruco_opencv

#include "rclcpp_components/register_node_macro.hpp"
RCLCPP_COMPONENTS_REGISTER_NODE(aruco_opencv::ArucoTracker)
RCLCPP_COMPONENTS_REGISTER_NODE(aruco_opencv::ArucoTrackerAutostart)
