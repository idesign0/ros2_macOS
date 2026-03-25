/// Copyright (c) 2014,
/// Systems, Robotics and Vision Group
/// University of the Balearic Islands
/// All rights reserved.
///
/// Redistribution and use in source and binary forms, with or without
/// modification, are permitted provided that the following conditions are met:
///     * Redistributions of source code must retain the above copyright
///       notice, this list of conditions and the following disclaimer.
///     * Redistributions in binary form must reproduce the above copyright
///       notice, this list of conditions and the following disclaimer in the
///       documentation and/or other materials provided with the distribution.
///     * All advertising materials mentioning features or use of this software
///       must display the following acknowledgement:
///       This product includes software developed by
///       Systems, Robotics and Vision Group, Univ. of the Balearic Islands
///     * Neither the name of Systems, Robotics and Vision Group, University of
///       the Balearic Islands nor the names of its contributors may be used
///       to endorse or promote products derived from this software without
///       specific prior written permission.
///
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
/// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
/// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
/// ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
/// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
/// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
/// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
/// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
/// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
/// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef AVT_VIMBA_CAMERA_H
#define AVT_VIMBA_CAMERA_H

#include <VimbaCPP/Include/VimbaCPP.h>

#include "avt_vimba_camera/frame_observer.hpp"
#include "avt_vimba_camera/avt_vimba_api.hpp"

#include <diagnostic_updater/diagnostic_updater.hpp>
#include <diagnostic_updater/publisher.hpp>
#include <sensor_msgs/msg/camera_info.hpp>
#include <camera_info_manager/camera_info_manager.hpp>

#include <string>
#include <mutex>

using AVT::VmbAPI::CameraPtr;
using AVT::VmbAPI::FramePtr;
using AVT::VmbAPI::IFrameObserverPtr;
using AVT::VmbAPI::VimbaSystem;

namespace avt_vimba_camera
{

enum CameraState
{
  OPENING,
  IDLE,
  CAMERA_NOT_FOUND,
  FORMAT_ERROR,
  ERROR,
  OK
};

const std::string PARAM_NAMESPACE = "feature/";

class AvtVimbaCamera
{
public:
  typedef std::function<void(const FramePtr)> frameCallbackFunc;

  // AvtVimbaCamera(rclcpp::Node* owner_node);
  AvtVimbaCamera(rclcpp::Node::SharedPtr owner_node);
  void start(const std::string& ip_str, const std::string& guid_str, const std::string& frame_id,
             const std::string& camera_info_url);
  void stop();
  void initConfig();
  void startImaging();
  void stopImaging();

  bool loadCameraSettings(const std::string& filename);
  bool saveCameraSettings(const std::string& filename);

  // Utility functions
  double getTimestampRealTime(VmbUint64_t timestamp_ticks);
  bool isOpened()
  {
    return opened_;
  }

  // Getters
  CameraState getCameraState() const;
  double getTimestamp();
  double getDeviceTemp();
  int getImageWidth();
  int getImageHeight();
  int getSensorWidth();
  int getSensorHeight();
  int getBinningOrDecimationX();
  int getBinningOrDecimationY();
  sensor_msgs::msg::CameraInfo getCameraInfo();

  // Setters
  void setCallback(frameCallbackFunc callback)
  {
    userFrameCallback = callback;
  }
  void setForceStop(bool force_stop)
  {
    force_stopped_ = force_stop;
  }

private:
  rclcpp::Node::SharedPtr nh_;
  rclcpp::Clock clock_;
  AvtVimbaApi api_;

  // IFrame Observer
  SP_DECL(FrameObserver) frame_obs_ptr_;
  // The currently streaming camera
  CameraPtr vimba_camera_ptr_;
  // Current frame
  FramePtr vimba_frame_ptr_;
  // Tick frequency of the on-board clock. Equal to 1 GHz when PTP is in use.
  VmbInt64_t vimba_timestamp_tick_freq_ = 1;

  // Mutex
  std::mutex config_mutex_;

  CameraState camera_state_;
  bool opened_;
  bool streaming_;
  bool force_stopped_;
  bool on_init_;
  bool on_init_config_;
  std::string guid_;
  std::string frame_id_;
  std::map<std::string, bool> writable_features_;
  std::set<std::string> cam_info_features_;

  std::shared_ptr<camera_info_manager::CameraInfoManager> info_man_;
  diagnostic_updater::Updater updater_;
  std::string diagnostic_msg_;
  rclcpp::node_interfaces::OnSetParametersCallbackHandle::SharedPtr param_sub_;

  CameraPtr openCamera(const std::string& id_str);

  frameCallbackFunc userFrameCallback;
  void frameCallback(const FramePtr vimba_frame_ptr);

  template <typename T>
  VmbErrorType setFeatureValue(const std::string& feature_str, const T& val);
  template <typename T>
  bool getFeatureValue(const std::string& feature_str, T& val);
  bool getFeatureValue(const std::string& feature_str, std::string& val);
  template <typename Vimba_Type, typename Std_Type>
  void configureFeature(const std::string& feature_str, const Vimba_Type& val_in, Std_Type& val_out);
  void configureFeature(const std::string& feature_str, const std::string& val_in, std::string& val_out);
  bool runCommand(const std::string& command_str);
  bool createParamFromFeature(const FeaturePtr feature, std::string& feature_name, bool& is_writable);
  rcl_interfaces::msg::SetParametersResult parameterCallback(const std::vector<rclcpp::Parameter>& parameters);
  void updateCameraInfo();
  void getCurrentState(diagnostic_updater::DiagnosticStatusWrapper& stat);
};
}  // namespace avt_vimba_camera
#endif
