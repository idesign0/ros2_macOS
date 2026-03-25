// Copyright 2025 Fictionlab sp. z o.o.
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

#include "aruco_opencv/parameters.hpp"

#include <map>
#include <string>
#include <vector>

#include <opencv2/aruco.hpp>
#include <opencv2/calib3d.hpp>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"

#include "aruco_opencv/utils.hpp"

namespace aruco_opencv
{

template<typename T>
inline void declare_param(
  rclcpp_lifecycle::LifecycleNode & node, std::string param_name,
  T default_value, bool dynamic = false)
{
  rcl_interfaces::msg::ParameterDescriptor descriptor;
  descriptor.read_only = !dynamic;

  node.declare_parameter(param_name, rclcpp::ParameterValue(default_value), descriptor);
}

template<typename T>
inline void get_param(
  rclcpp_lifecycle::LifecycleNode & node, std::string param_name, T & out_value,
  std::string log_info = "")
{
  node.get_parameter(param_name, out_value);

  if (!log_info.empty()) {
    RCLCPP_INFO_STREAM(node.get_logger(), log_info << out_value);
  }
}

inline void declare_param_int_range(
  rclcpp_lifecycle::LifecycleNode & node, std::string param_name,
  int default_value, int min_value, int max_value)
{
  rcl_interfaces::msg::ParameterDescriptor descriptor;

  auto range = rcl_interfaces::msg::IntegerRange();
  range.from_value = min_value;
  range.to_value = max_value;

  descriptor.integer_range.push_back(range);

  node.declare_parameter(param_name, default_value, descriptor);
}

inline void declare_param_double_range(
  rclcpp_lifecycle::LifecycleNode & node, std::string param_name,
  double default_value, double min_value, double max_value)
{
  rcl_interfaces::msg::ParameterDescriptor descriptor;

  auto range = rcl_interfaces::msg::FloatingPointRange();
  range.from_value = min_value;
  range.to_value = max_value;

  descriptor.floating_point_range.push_back(range);

  node.declare_parameter(param_name, default_value, descriptor);
}

void declare_all_parameters(rclcpp_lifecycle::LifecycleNode & node)
{
  declare_core_parameters(node);
  declare_aruco_parameters(node);
  declare_detector_parameters(node);
}

void declare_core_parameters(rclcpp_lifecycle::LifecycleNode & node)
{
  declare_param(node, "cam_base_topic", std::string("camera/image_raw"));
  declare_param(node, "image_is_rectified", false, false);
  declare_param(node, "output_frame", std::string(""));
  declare_param(node, "marker_dict", std::string("4X4_50"));
  declare_param(node, "image_sub_compressed", false);
  declare_param(node, "image_sub_qos.reliability",
      static_cast<int>(RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT));
  declare_param(node, "image_sub_qos.durability",
      static_cast<int>(RMW_QOS_POLICY_DURABILITY_VOLATILE));
  declare_param(node, "image_sub_qos.depth", 1);
  declare_param(node, "publish_tf", true, true);
  declare_param(node, "board_descriptions_path", std::string(""));
}

void declare_aruco_parameters(rclcpp_lifecycle::LifecycleNode & node)
{
  #if CV_VERSION_MAJOR > 4 || CV_VERSION_MAJOR == 4 && CV_VERSION_MINOR >= 7
  auto default_parameters = cv::makePtr<cv::aruco::DetectorParameters>();
  #else
  auto default_parameters = cv::aruco::DetectorParameters::create();
  #endif

  declare_param_int_range(node,
    "aruco.adaptiveThreshWinSizeMin", default_parameters->adaptiveThreshWinSizeMin, 3, 100);
  declare_param_int_range(node,
    "aruco.adaptiveThreshWinSizeMax", default_parameters->adaptiveThreshWinSizeMax, 3, 100);
  declare_param_int_range(node,
    "aruco.adaptiveThreshWinSizeStep", default_parameters->adaptiveThreshWinSizeStep, 1, 100);
  declare_param_double_range(node,
    "aruco.adaptiveThreshConstant", default_parameters->adaptiveThreshConstant, 0.0, 100.0);
  declare_param_double_range(node,
    "aruco.minMarkerPerimeterRate", default_parameters->minMarkerPerimeterRate, 0.0, 4.0);
  declare_param_double_range(node,
    "aruco.maxMarkerPerimeterRate", default_parameters->maxMarkerPerimeterRate, 0.0, 4.0);
  declare_param_double_range(node,
    "aruco.polygonalApproxAccuracyRate",
    default_parameters->polygonalApproxAccuracyRate, 0.0, 0.3);
  declare_param_double_range(node,
    "aruco.minCornerDistanceRate", default_parameters->minCornerDistanceRate, 0.0, 0.25);
  declare_param_int_range(node,
    "aruco.minDistanceToBorder", default_parameters->minDistanceToBorder, 0, 100);
  declare_param_double_range(node,
    "aruco.minMarkerDistanceRate", default_parameters->minMarkerDistanceRate, 0.0, 0.25);
  declare_param_int_range(node,
    "aruco.markerBorderBits", default_parameters->markerBorderBits, 1, 3);
  declare_param_int_range(node,
    "aruco.perspectiveRemovePixelPerCell",
    default_parameters->perspectiveRemovePixelPerCell, 1, 20);
  declare_param_double_range(node,
    "aruco.perspectiveRemoveIgnoredMarginPerCell",
    default_parameters->perspectiveRemoveIgnoredMarginPerCell, 0.0, 0.5);
  declare_param_double_range(node,
    "aruco.maxErroneousBitsInBorderRate",
    default_parameters->maxErroneousBitsInBorderRate, 0.0, 1.0);
  declare_param_double_range(node,
    "aruco.minOtsuStdDev", default_parameters->minOtsuStdDev, 0.0, 30.0);
  declare_param_double_range(node,
    "aruco.errorCorrectionRate", default_parameters->errorCorrectionRate, 0.0, 1.0);
  declare_param_int_range(node,
    "aruco.cornerRefinementMethod", default_parameters->cornerRefinementMethod, 0, 2);
  declare_param_int_range(node,
    "aruco.cornerRefinementWinSize", default_parameters->cornerRefinementWinSize, 2, 10);
  declare_param_int_range(node,
    "aruco.cornerRefinementMaxIterations",
    default_parameters->cornerRefinementMaxIterations, 1, 100);
  declare_param_double_range(node,
    "aruco.cornerRefinementMinAccuracy",
    default_parameters->cornerRefinementMinAccuracy, 0.01, 1.0);
  declare_param(node,
    "aruco.detectInvertedMarker", default_parameters->detectInvertedMarker, true);

  #if CV_VERSION_MAJOR > 4 || CV_VERSION_MAJOR == 4 && CV_VERSION_MINOR >= 6
  declare_param(node,
    "aruco.useAruco3Detection", default_parameters->useAruco3Detection, true);
  declare_param_int_range(node,
    "aruco.minSideLengthCanonicalImg",
    default_parameters->minSideLengthCanonicalImg, 1, 100);
  declare_param_double_range(node,
    "aruco.minMarkerLengthRatioOriginalImg",
    default_parameters->minMarkerLengthRatioOriginalImg, 0.0, 1.0);
  #endif
}

void declare_detector_parameters(rclcpp_lifecycle::LifecycleNode & node)
{
  declare_param(node, "marker_size", 0.15, true);
  declare_param(node, "pose_selector.strategy", std::string("REPROJECTION_ERROR"), true);
  declare_param(node, "pose_selector.debug", false, true);
}

CoreParams retrieve_core_parameters(rclcpp_lifecycle::LifecycleNode & node)
{
  CoreParams out{};
  get_param(node, "cam_base_topic", out.cam_base_topic, "Camera Base Topic: ");
  node.get_parameter("image_is_rectified", out.image_is_rectified);
  node.get_parameter("output_frame", out.output_frame);
  get_param(node, "marker_dict", out.marker_dict, "Marker Dictionary name: ");
  node.get_parameter("image_sub_compressed", out.image_sub_compressed);
  node.get_parameter("image_sub_qos.reliability", out.qos_rel);
  node.get_parameter("image_sub_qos.durability", out.qos_dur);
  node.get_parameter("image_sub_qos.depth", out.qos_depth);
  node.get_parameter("publish_tf", out.publish_tf);
  node.get_parameter("board_descriptions_path", out.board_descriptions_path);
  return out;
}

void retrieve_aruco_parameters(
  rclcpp_lifecycle::LifecycleNode & node,
  cv::Ptr<cv::aruco::DetectorParameters> & detector_parameters,
  bool log_values)
{
  node.get_parameter(
    "aruco.adaptiveThreshWinSizeMin", detector_parameters->adaptiveThreshWinSizeMin);
  node.get_parameter(
    "aruco.adaptiveThreshWinSizeMax", detector_parameters->adaptiveThreshWinSizeMax);
  node.get_parameter(
    "aruco.adaptiveThreshWinSizeStep", detector_parameters->adaptiveThreshWinSizeStep);
  node.get_parameter(
    "aruco.adaptiveThreshConstant", detector_parameters->adaptiveThreshConstant);
  node.get_parameter(
    "aruco.minMarkerPerimeterRate", detector_parameters->minMarkerPerimeterRate);
  node.get_parameter(
    "aruco.maxMarkerPerimeterRate", detector_parameters->maxMarkerPerimeterRate);
  node.get_parameter(
    "aruco.polygonalApproxAccuracyRate", detector_parameters->polygonalApproxAccuracyRate);
  node.get_parameter(
    "aruco.minCornerDistanceRate", detector_parameters->minCornerDistanceRate);
  node.get_parameter(
    "aruco.minDistanceToBorder", detector_parameters->minDistanceToBorder);
  node.get_parameter(
    "aruco.minMarkerDistanceRate", detector_parameters->minMarkerDistanceRate);
  node.get_parameter(
    "aruco.markerBorderBits", detector_parameters->markerBorderBits);
  node.get_parameter(
    "aruco.perspectiveRemovePixelPerCell", detector_parameters->perspectiveRemovePixelPerCell);
  node.get_parameter(
    "aruco.perspectiveRemoveIgnoredMarginPerCell",
    detector_parameters->perspectiveRemoveIgnoredMarginPerCell);
  node.get_parameter(
    "aruco.maxErroneousBitsInBorderRate", detector_parameters->maxErroneousBitsInBorderRate);
  node.get_parameter(
    "aruco.minOtsuStdDev", detector_parameters->minOtsuStdDev);
  node.get_parameter(
    "aruco.errorCorrectionRate", detector_parameters->errorCorrectionRate);

  #if CV_VERSION_MAJOR > 4 || CV_VERSION_MAJOR == 4 && CV_VERSION_MINOR >= 7
  int refine_method = 0;
  node.get_parameter("aruco.cornerRefinementMethod", refine_method);
  detector_parameters->cornerRefinementMethod =
    static_cast<cv::aruco::CornerRefineMethod>(refine_method);
  #else
  node.get_parameter(
    "aruco.cornerRefinementMethod", detector_parameters->cornerRefinementMethod);
  #endif

  node.get_parameter(
    "aruco.cornerRefinementWinSize", detector_parameters->cornerRefinementWinSize);
  node.get_parameter(
    "aruco.cornerRefinementMaxIterations", detector_parameters->cornerRefinementMaxIterations);
  node.get_parameter(
    "aruco.cornerRefinementMinAccuracy", detector_parameters->cornerRefinementMinAccuracy);
  node.get_parameter(
    "aruco.detectInvertedMarker", detector_parameters->detectInvertedMarker);

  #if CV_VERSION_MAJOR > 4 || CV_VERSION_MAJOR == 4 && CV_VERSION_MINOR >= 6
  node.get_parameter(
    "aruco.useAruco3Detection", detector_parameters->useAruco3Detection);
  node.get_parameter(
    "aruco.minSideLengthCanonicalImg", detector_parameters->minSideLengthCanonicalImg);
  node.get_parameter(
    "aruco.minMarkerLengthRatioOriginalImg", detector_parameters->minMarkerLengthRatioOriginalImg);
  #endif

  if (log_values) {
    RCLCPP_INFO_STREAM(
      node.get_logger(),
      " * adaptiveThreshWinSizeMin: " << detector_parameters->adaptiveThreshWinSizeMin);
    RCLCPP_INFO_STREAM(
      node.get_logger(),
      " * adaptiveThreshWinSizeMax: " << detector_parameters->adaptiveThreshWinSizeMax);
    RCLCPP_INFO_STREAM(
      node.get_logger(),
      " * adaptiveThreshWinSizeStep: " << detector_parameters->adaptiveThreshWinSizeStep);
    RCLCPP_INFO_STREAM(
      node.get_logger(),
      " * adaptiveThreshConstant: " << detector_parameters->adaptiveThreshConstant);
    RCLCPP_INFO_STREAM(
      node.get_logger(),
      " * minMarkerPerimeterRate: " << detector_parameters->minMarkerPerimeterRate);
    RCLCPP_INFO_STREAM(
      node.get_logger(),
      " * maxMarkerPerimeterRate: " << detector_parameters->maxMarkerPerimeterRate);
    RCLCPP_INFO_STREAM(
      node.get_logger(),
      " * polygonalApproxAccuracyRate: " << detector_parameters->polygonalApproxAccuracyRate);
    RCLCPP_INFO_STREAM(
      node.get_logger(),
      " * minCornerDistanceRate: " << detector_parameters->minCornerDistanceRate);
    RCLCPP_INFO_STREAM(
      node.get_logger(),
      " * minDistanceToBorder: " << detector_parameters->minDistanceToBorder);
    RCLCPP_INFO_STREAM(
      node.get_logger(),
      " * minMarkerDistanceRate: " << detector_parameters->minMarkerDistanceRate);
    RCLCPP_INFO_STREAM(
      node.get_logger(),
      " * markerBorderBits: " << detector_parameters->markerBorderBits);
    RCLCPP_INFO_STREAM(
      node.get_logger(),
      " * perspectiveRemovePixelPerCell: " << detector_parameters->perspectiveRemovePixelPerCell);
    RCLCPP_INFO_STREAM(
      node.get_logger(),
      " * perspectiveRemoveIgnoredMarginPerCell: " <<
        detector_parameters->perspectiveRemoveIgnoredMarginPerCell);
    RCLCPP_INFO_STREAM(
      node.get_logger(),
      " * maxErroneousBitsInBorderRate: " << detector_parameters->maxErroneousBitsInBorderRate);
    RCLCPP_INFO_STREAM(
      node.get_logger(),
      " * minOtsuStdDev: " << detector_parameters->minOtsuStdDev);
    RCLCPP_INFO_STREAM(
      node.get_logger(),
      " * errorCorrectionRate: " << detector_parameters->errorCorrectionRate);
    std::map<int, std::string> crmethod = {{0, "NONE"}, {1, "SUBPIX"}, {2, "CONTOUR"}};
    RCLCPP_INFO_STREAM(
      node.get_logger(),
      " * cornerRefinementMethod: " << detector_parameters->cornerRefinementMethod << " (" <<
        crmethod[detector_parameters->cornerRefinementMethod] << ")");
    RCLCPP_INFO_STREAM(
      node.get_logger(),
      " * cornerRefinementWinSize: " << detector_parameters->cornerRefinementWinSize);
    RCLCPP_INFO_STREAM(
      node.get_logger(),
      " * cornerRefinementMaxIterations: " << detector_parameters->cornerRefinementMaxIterations);
    RCLCPP_INFO_STREAM(
      node.get_logger(),
      " * cornerRefinementMinAccuracy: " << detector_parameters->cornerRefinementMinAccuracy);
    RCLCPP_INFO_STREAM(
      node.get_logger(),
      " * detectInvertedMarker: " <<
        (detector_parameters->detectInvertedMarker ? "TRUE" : "FALSE"));

    #if CV_VERSION_MAJOR > 4 || CV_VERSION_MAJOR == 4 && CV_VERSION_MINOR >= 6
    RCLCPP_INFO_STREAM(
      node.get_logger(),
      " * useAruco3Detection: " <<
        (detector_parameters->useAruco3Detection ? "TRUE" : "FALSE"));
    RCLCPP_INFO_STREAM(
      node.get_logger(),
      " * minSideLengthCanonicalImg: " << detector_parameters->minSideLengthCanonicalImg);
    RCLCPP_INFO_STREAM(
      node.get_logger(),
      " * minMarkerLengthRatioOriginalImg: " <<
        detector_parameters->minMarkerLengthRatioOriginalImg);
    #endif
  }
}

DetectorParams retrieve_detector_parameters(rclcpp_lifecycle::LifecycleNode & node)
{
  DetectorParams out{};
  std::string strategy_name;
  node.get_parameter("marker_size", out.marker_size);
  node.get_parameter("pose_selector.strategy", strategy_name);
  node.get_parameter("pose_selector.debug", out.pose_selector.debug);
  out.pose_selector.strategy = parse_selector_strategy(strategy_name);
  return out;
}

rcl_interfaces::msg::SetParametersResult validate_core_parameters(
  const std::vector<rclcpp::Parameter> & parameters)
{
  rcl_interfaces::msg::SetParametersResult result;
  result.successful = true;
  for (const auto & param : parameters) {
    if (param.get_name() == "image_sub_qos.depth" && param.as_int() < 1) {
      result.successful = false;
      result.reason = "image_sub_qos.depth must be >= 1";
      return result;
    }
  }

  return result;
}

rcl_interfaces::msg::SetParametersResult validate_detector_parameters(
  const std::vector<rclcpp::Parameter> & parameters)
{
  rcl_interfaces::msg::SetParametersResult result;
  result.successful = true;
  for (const auto & param : parameters) {
    if (param.get_name() == "marker_size" && param.as_double() <= 0.0) {
      result.successful = false;
      result.reason = "marker_size must be positive";
      return result;
    }
    if (param.get_name() == "pose_selector.strategy") {
      std::string strategy = param.as_string();
      if (strategy != "REPROJECTION_ERROR" && strategy != "PLANE_NORMAL_PARALLEL") {
        result.successful = false;
        result.reason =
          "pose_selector.strategy must be one of: REPROJECTION_ERROR, PLANE_NORMAL_PARALLEL";
        return result;
      }
    }
  }
  return result;
}

void update_dynamic_parameters(
  rclcpp_lifecycle::LifecycleNode & node,
  const std::vector<rclcpp::Parameter> & parameters,
  DetectorParams & detector_params,
  cv::Ptr<cv::aruco::DetectorParameters> & aruco_parameters)
{
  bool aruco_param_changed = false;
  for (auto & param : parameters) {
    if (param.get_name() == "marker_size") {
      detector_params.marker_size = param.as_double();
    } else if (param.get_name() == "pose_selector.strategy") {
      detector_params.pose_selector.strategy = parse_selector_strategy(param.as_string());
    } else if (param.get_name() == "pose_selector.debug") {
      detector_params.pose_selector.debug = param.as_bool();
    } else if (param.get_name().rfind("aruco", 0) == 0) {
      aruco_param_changed = true;
    } else {
      // Unknown parameter, ignore
      continue;
    }

    RCLCPP_INFO_STREAM(
        node.get_logger(),
        "Parameter \"" << param.get_name() << "\" changed to " << param.value_to_string());
  }

  if (aruco_param_changed) {
    retrieve_aruco_parameters(node, aruco_parameters);
  }
}

}  // namespace aruco_opencv
