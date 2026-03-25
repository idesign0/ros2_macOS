/// Copyright (c) 2014,
/// Systems, Robotics and Vision Group
/// University of the Balearican Islands
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
///       Systems, Robotics and Vision Group, Univ. of the Balearican Islands
///     * Neither the name of Systems, Robotics and Vision Group, University of
///       the Balearican Islands nor the names of its contributors may be used
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

#include "avt_vimba_camera/avt_vimba_camera.hpp"
#include "VimbaC/Include/VimbaC.h"
#include "avt_vimba_camera/avt_vimba_api.hpp"

#include <rclcpp/rclcpp.hpp>
#include <diagnostic_msgs/msg/diagnostic_status.hpp>

#include <signal.h>

namespace avt_vimba_camera
{
static volatile int keepRunning = 1;

AvtVimbaCamera::AvtVimbaCamera(rclcpp::Node::SharedPtr owner_node)
  : nh_(owner_node), api_(owner_node->get_logger()), updater_(owner_node)
{
  clock_ = rclcpp::Clock(RCL_ROS_TIME);
  opened_ = false;     // camera connected to the api
  streaming_ = false;  // capturing frames
  on_init_ = true;     // on initialization phase
  on_init_config_ = false;
  force_stopped_ = false;
  camera_state_ = OPENING;

  // Features that affect the camera_info parameters
  cam_info_features_.emplace("Width");
  cam_info_features_.emplace("Height");
  cam_info_features_.emplace("BinningHorizontal");
  cam_info_features_.emplace("BinningVertical");
  cam_info_features_.emplace("DecimationHorizontal");
  cam_info_features_.emplace("DecimationVertical");

  updater_.setHardwareID("unknown");
  updater_.add(owner_node->get_name(), this, &AvtVimbaCamera::getCurrentState);
}

void AvtVimbaCamera::start(const std::string& ip_str, const std::string& guid_str, const std::string& frame_id,
                           const std::string& camera_info_url)
{
  if (opened_)
    return;

  frame_id_ = frame_id;
  info_man_ = std::shared_ptr<camera_info_manager::CameraInfoManager>(
      new camera_info_manager::CameraInfoManager(nh_.get(), frame_id, camera_info_url));
  updater_.broadcast(0, "Starting device with IP:" + ip_str + " or GUID:" + guid_str);

  // Determine which camera to use. Try IP first
  if (!ip_str.empty())
  {
    diagnostic_msg_ = "Trying to open camera by IP: " + ip_str;
    RCLCPP_INFO_STREAM(nh_->get_logger(), "Trying to open camera by IP: " << ip_str);
    vimba_camera_ptr_ = openCamera(ip_str);
    if (!vimba_camera_ptr_)
    {
      RCLCPP_WARN(nh_->get_logger(), "Camera pointer is empty. Returning...");
      return;
    }
    updater_.setHardwareID(ip_str);
    guid_ = ip_str;
    // If both guid and IP are available, open by IP and check guid
    if (!guid_str.empty())
    {
      std::string cam_guid_str;
      vimba_camera_ptr_->GetSerialNumber(cam_guid_str);
      if (!vimba_camera_ptr_)
      {
        RCLCPP_WARN(nh_->get_logger(), "Camera pointer is empty. Returning...");
        return;
      }
      assert(cam_guid_str == guid_str);
      updater_.setHardwareID(guid_str);
      guid_ = guid_str;
      diagnostic_msg_ = "GUID " + cam_guid_str + " matches for camera with IP: " + ip_str;
      RCLCPP_INFO_STREAM(nh_->get_logger(), "GUID " << cam_guid_str << " matches for camera with IP: " << ip_str);
    }
  }
  else if (!guid_str.empty())
  {
    // Only guid available
    diagnostic_msg_ = "Trying to open camera by ID: " + guid_str;
    RCLCPP_INFO_STREAM(nh_->get_logger(), "Trying to open camera by ID: " << guid_str);
    vimba_camera_ptr_ = openCamera(guid_str);
    updater_.setHardwareID(guid_str);
    guid_ = guid_str;
  }
  else
  {
    // No identifying info (GUID and IP) are available
    diagnostic_msg_ = "Can't connect to the camera: at least GUID or IP need to be set.";
    RCLCPP_ERROR(nh_->get_logger(), "Can't connect to the camera: at least GUID or IP need to be set.");
    camera_state_ = ERROR;
  }
  updater_.force_update();

  getFeatureValue("GevTimestampTickFrequency", vimba_timestamp_tick_freq_);

  // From the SynchronousGrab API example:
  // TODO Set the GeV packet size to the highest possible value
  VmbInterfaceType cam_int_type;
  vimba_camera_ptr_->GetInterfaceType(cam_int_type);
  if (cam_int_type == VmbInterfaceEthernet)
  {
    runCommand("GVSPAdjustPacketSize");
  }

  std::string trigger_source;
  getFeatureValue("TriggerSource", trigger_source);

  SP_SET(frame_obs_ptr_,
          new FrameObserver(vimba_camera_ptr_,
                            std::bind(&avt_vimba_camera::AvtVimbaCamera::frameCallback, this, std::placeholders::_1)));
  RCLCPP_INFO(nh_->get_logger(), "Ready to receive frames triggered by %s", trigger_source.c_str());
  camera_state_ = IDLE;

  updater_.force_update();

  initConfig();
}

void AvtVimbaCamera::stop()
{
  if (!opened_)
    return;
  vimba_camera_ptr_->Close();
  opened_ = false;
}

void AvtVimbaCamera::startImaging()
{
  if (!streaming_)
  {
    // Start streaming
    VmbErrorType err = vimba_camera_ptr_->StartContinuousImageAcquisition(3, IFrameObserverPtr(frame_obs_ptr_));
    if (err == VmbErrorSuccess)
    {
      diagnostic_msg_ = "Starting continuous image acquisition";
      RCLCPP_INFO_STREAM(nh_->get_logger(), "Starting continuous image acquisition ...");
      streaming_ = true;
      camera_state_ = OK;
    }
    else
    {
      diagnostic_msg_ = "Could not start continuous image acquisition. Error: " + api_.errorCodeToMessage(err);
      RCLCPP_ERROR_STREAM(nh_->get_logger(), "Could not start continuous image acquisition. "
                                                 << "\n Error: " << api_.errorCodeToMessage(err));
      camera_state_ = ERROR;
    }
  }
  else
  {
    RCLCPP_WARN_STREAM(nh_->get_logger(), "Start imaging called, but the camera is already imaging.");
  }
  updater_.force_update();
}

void AvtVimbaCamera::stopImaging()
{
  if (streaming_ || on_init_)
  {
    VmbErrorType err = vimba_camera_ptr_->StopContinuousImageAcquisition();
    if (err == VmbErrorSuccess)
    {
      diagnostic_msg_ = "Acquisition stopped";
      RCLCPP_INFO_STREAM(nh_->get_logger(), "Acquisition stoppped ...");
      streaming_ = false;
      camera_state_ = IDLE;
    }
    else
    {
      diagnostic_msg_ = "Could not stop image acquisition. Error: " + api_.errorCodeToMessage(err);
      RCLCPP_ERROR_STREAM(nh_->get_logger(), "Could not stop image acquisition."
                                                 << "\n Error: " << api_.errorCodeToMessage(err));
      camera_state_ = ERROR;
    }
  }
  else
  {
    RCLCPP_WARN_STREAM(nh_->get_logger(), "Stop imaging called, but the camera is already stopped.");
  }
  updater_.force_update();
}

CameraPtr AvtVimbaCamera::openCamera(const std::string& id_str)
{
  // Details:   The ID might be one of the following:
  //            "IP:169.254.12.13",
  //            "MAC:000f31000001",
  //            or a plain serial number: "1234567890".

  CameraPtr camera;
  VimbaSystem& vimba_system(VimbaSystem::GetInstance());

  // get camera
  VmbErrorType err = vimba_system.GetCameraByID(id_str.c_str(), camera);
  while (err != VmbErrorSuccess)
  {
    if (keepRunning)
    {
      RCLCPP_WARN_STREAM(nh_->get_logger(),
                         "Could not find camera using " << id_str << ". Retrying every two seconds ...");
      std::this_thread::sleep_for(std::chrono::milliseconds(2000));
      err = vimba_system.GetCameraByID(id_str.c_str(), camera);
    }
    else
    {
      RCLCPP_ERROR_STREAM(nh_->get_logger(),
                          "Could not find camera using " << id_str << "\n Error: " << api_.errorCodeToMessage(err));
      camera_state_ = CAMERA_NOT_FOUND;
      return camera;
    }
  }

  // open camera
  err = camera->Open(VmbAccessModeFull);
  while (err != VmbErrorSuccess && keepRunning)
  {
    if (keepRunning)
    {
      RCLCPP_WARN_STREAM(nh_->get_logger(), "Could not open camera. Retrying every two seconds ...");
      err = camera->Open(VmbAccessModeFull);
      std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }
    else
    {
      RCLCPP_ERROR_STREAM(nh_->get_logger(),
                          "Could not open camera " << id_str << "\n Error: " << api_.errorCodeToMessage(err));
      camera_state_ = CAMERA_NOT_FOUND;
      return camera;
    }
  }

  std::string cam_id, cam_name;
  camera->GetID(cam_id);
  camera->GetName(cam_name);
  RCLCPP_INFO_STREAM(nh_->get_logger(), "Opened connection to camera named " << cam_name << " with ID " << cam_id);

  std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  opened_ = true;
  camera_state_ = IDLE;
  return camera;
}

void AvtVimbaCamera::frameCallback(const FramePtr vimba_frame_ptr)
{
  std::unique_lock<std::mutex> lock(config_mutex_);
  camera_state_ = OK;
  diagnostic_msg_ = "Camera operating normally";

  // Call the callback implemented by other classes
  std::thread thread_callback = std::thread(userFrameCallback, vimba_frame_ptr);
  thread_callback.join();
}

CameraState AvtVimbaCamera::getCameraState() const
{
  return camera_state_;
}

double AvtVimbaCamera::getTimestamp()
{
  double timestamp = -1.0;
  if (runCommand("GevTimestampControlLatch"))
  {
    VmbInt64_t freq, ticks;
    getFeatureValue("GevTimestampTickFrequency", freq);
    getFeatureValue("GevTimestampValue", ticks);
    timestamp = static_cast<double>(ticks) / static_cast<double>(freq);
  }
  return timestamp;
}

double AvtVimbaCamera::getDeviceTemp()
{
  double temp = -1.0;
  if (setFeatureValue("DeviceTemperatureSelector", "Main") == VmbErrorSuccess)
  {
    getFeatureValue("DeviceTemperature", temp);
  }
  return temp;
}

int AvtVimbaCamera::getImageWidth()
{
  int width = -1;
  getFeatureValue("Width", width);
  return width;
}

int AvtVimbaCamera::getImageHeight()
{
  int height = -1;
  getFeatureValue("Height", height);
  return height;
}

int AvtVimbaCamera::getSensorWidth()
{
  int sensor_width = -1;
  getFeatureValue("SensorWidth", sensor_width);
  return sensor_width;
}

int AvtVimbaCamera::getSensorHeight()
{
  int sensor_height = -1;
  getFeatureValue("SensorHeight", sensor_height);
  return sensor_height;
}

int AvtVimbaCamera::getBinningOrDecimationX()
{
  int binning = -1;
  int decimation = -1;
  getFeatureValue("BinningHorizontal", binning);
  getFeatureValue("DecimationHorizontal", decimation);

  return std::max(binning, decimation);
}

int AvtVimbaCamera::getBinningOrDecimationY()
{
  int binning = -1;
  int decimation = -1;
  getFeatureValue("BinningVertical", binning);
  getFeatureValue("DecimationVertical", decimation);

  return std::max(binning, decimation);
}

sensor_msgs::msg::CameraInfo AvtVimbaCamera::getCameraInfo()
{
  return info_man_->getCameraInfo();
}

double AvtVimbaCamera::getTimestampRealTime(VmbUint64_t timestamp_ticks)
{
  return (static_cast<double>(timestamp_ticks)) / (static_cast<double>(vimba_timestamp_tick_freq_));
}

// Template function to SET a feature value from the camera
template <typename T>
VmbErrorType AvtVimbaCamera::setFeatureValue(const std::string& feature_str, const T& val)
{
  VmbErrorType err;
  FeaturePtr vimba_feature_ptr;
  err = vimba_camera_ptr_->GetFeatureByName(feature_str.c_str(), vimba_feature_ptr);
  if (err == VmbErrorSuccess)
  {
    bool writable;
    err = vimba_feature_ptr->IsWritable(writable);
    if (err == VmbErrorSuccess)
    {
      if (writable)
      {
        RCLCPP_DEBUG_STREAM(nh_->get_logger(), "Setting feature " << feature_str << " value " << val);
        VmbFeatureDataType data_type;
        err = vimba_feature_ptr->GetDataType(data_type);
        if (err == VmbErrorSuccess)
        {
          if (data_type == VmbFeatureDataEnum)
          {
            bool available;
            err = vimba_feature_ptr->IsValueAvailable(val, available);
            if (err == VmbErrorSuccess)
            {
              if (available)
              {
                err = vimba_feature_ptr->SetValue(val);
              }
              else
              {
                RCLCPP_WARN_STREAM(nh_->get_logger(), "Feature " << feature_str << " is available now.");
              }
            }
            else
            {
              RCLCPP_WARN_STREAM(nh_->get_logger(), "Feature " << feature_str << ": value unavailable\n\tERROR "
                                                               << api_.errorCodeToMessage(err));
            }
          }
          else
          {
            err = vimba_feature_ptr->SetValue(val);
          }
        }
        else
        {
          RCLCPP_WARN_STREAM(nh_->get_logger(),
                             "Feature " << feature_str << ": Bad data type\n\tERROR " << api_.errorCodeToMessage(err));
        }
      }
      else
      {
        RCLCPP_WARN_STREAM(nh_->get_logger(), "Feature " << feature_str << " is not writable.");
      }
    }
    else
    {
      RCLCPP_WARN_STREAM(nh_->get_logger(), "Feature " << feature_str << ": ERROR " << api_.errorCodeToMessage(err));
    }
  }
  else
  {
    RCLCPP_WARN_STREAM(nh_->get_logger(),
                       "Could not get feature " << feature_str << ", your camera probably doesn't support it.");
  }
  return err;
}

// Template function to GET a feature value from the camera
template <typename T>
bool AvtVimbaCamera::getFeatureValue(const std::string& feature_str, T& val)
{
  RCLCPP_DEBUG_STREAM(nh_->get_logger(), "Asking for feature " << feature_str);
  VmbErrorType err;
  FeaturePtr vimba_feature_ptr;
  VmbFeatureDataType data_type;
  err = vimba_camera_ptr_->GetFeatureByName(feature_str.c_str(), vimba_feature_ptr);
  if (err == VmbErrorSuccess)
  {
    bool readable;
    vimba_feature_ptr->IsReadable(readable);
    if (readable)
    {
      vimba_feature_ptr->GetDataType(data_type);
      if (err != VmbErrorSuccess)
      {
        std::cout << "[Could not get feature Data Type. Error code: " << err << "]" << std::endl;
      }
      else
      {
        switch (data_type)
        {
          case VmbFeatureDataBool:
            bool bValue;
            err = vimba_feature_ptr->GetValue(bValue);
            if (err == VmbErrorSuccess)
            {
              val = static_cast<T>(bValue);
            }
            break;
          case VmbFeatureDataFloat:
            double fValue;
            err = vimba_feature_ptr->GetValue(fValue);
            if (err == VmbErrorSuccess)
            {
              val = static_cast<T>(fValue);
            }
            break;
          case VmbFeatureDataInt:
            VmbInt64_t nValue;
            err = vimba_feature_ptr->GetValue(nValue);
            if (err == VmbErrorSuccess)
            {
              val = static_cast<T>(nValue);
            }
            break;
          default:
            break;
        }
        if (err != VmbErrorSuccess)
        {
          RCLCPP_WARN_STREAM(nh_->get_logger(),
                             "Could not get feature value. Error code: " << api_.errorCodeToMessage(err));
        }
      }
    }
    else
    {
      RCLCPP_WARN_STREAM(nh_->get_logger(), "Feature " << feature_str << " is not readable.");
    }
  }
  else
  {
    RCLCPP_WARN_STREAM(nh_->get_logger(), "Could not get feature " << feature_str);
  }
  return (err == VmbErrorSuccess);
}

// Function to GET a feature value from the camera, overloaded to strings
bool AvtVimbaCamera::getFeatureValue(const std::string& feature_str, std::string& val)
{
  RCLCPP_DEBUG_STREAM(nh_->get_logger(), "Asking for feature " << feature_str);
  VmbErrorType err;
  FeaturePtr vimba_feature_ptr;
  VmbFeatureDataType data_type;
  err = vimba_camera_ptr_->GetFeatureByName(feature_str.c_str(), vimba_feature_ptr);
  if (err == VmbErrorSuccess)
  {
    bool readable;
    vimba_feature_ptr->IsReadable(readable);
    if (readable)
    {
      vimba_feature_ptr->GetDataType(data_type);
      if (err != VmbErrorSuccess)
      {
        RCLCPP_ERROR_STREAM(nh_->get_logger(), "[Could not get feature Data Type. Error code: " << err << "]");
      }
      else
      {
        std::string strValue;
        switch (data_type)
        {
          case VmbFeatureDataEnum:
            err = vimba_feature_ptr->GetValue(strValue);
            if (err == VmbErrorSuccess)
            {
              val = strValue;
            }
            break;
          case VmbFeatureDataString:
            err = vimba_feature_ptr->GetValue(strValue);
            if (err == VmbErrorSuccess)
            {
              val = strValue;
            }
            break;
          default:
            break;
        }
        if (err != VmbErrorSuccess)
        {
          RCLCPP_WARN_STREAM(nh_->get_logger(),
                             "Could not get feature value. Error code: " << api_.errorCodeToMessage(err));
        }
      }
    }
    else
    {
      RCLCPP_WARN_STREAM(nh_->get_logger(), "Feature " << feature_str << " is not readable.");
    }
  }
  else
  {
    RCLCPP_WARN_STREAM(nh_->get_logger(), "Could not get feature " << feature_str);
  }
  return (err == VmbErrorSuccess);
}

// Tries to configure a camera feature.
// Updates the config value passed in if a different config is in use by the camera.
template <typename Vimba_Type, typename Std_Type>
void AvtVimbaCamera::configureFeature(const std::string& feature_str, const Vimba_Type& val_in, Std_Type& val_out)
{
  Vimba_Type actual_value;

  VmbErrorType return_value = setFeatureValue(feature_str, val_in);
  if (return_value == VmbErrorSuccess || return_value == VmbErrorInvalidValue)
  {
    getFeatureValue(feature_str, actual_value);
    if (val_in == actual_value)
    {
      RCLCPP_INFO_STREAM(nh_->get_logger(), " - " << feature_str << " set to " << actual_value);
    }
    else
    {
      RCLCPP_WARN_STREAM(nh_->get_logger(), " - Tried to set " << feature_str << " to " << val_in
                                                               << " but the camera used " << actual_value
                                                               << " instead");
      val_out = static_cast<Std_Type>(actual_value);
    }
  }
  else
  {
    RCLCPP_ERROR_STREAM(nh_->get_logger(), " - Failed to set " << feature_str << " to " << actual_value);
    val_out = static_cast<Std_Type>(val_in);
  }
}

// Overloaded for strings, template specialization doesn't currently compile with GCC
void AvtVimbaCamera::configureFeature(const std::string& feature_str, const std::string& val_in, std::string& val_out)
{
  std::string actual_value;

  VmbErrorType return_value = setFeatureValue(feature_str, val_in.c_str());
  if (return_value == VmbErrorSuccess || return_value == VmbErrorInvalidValue)
  {
    getFeatureValue(feature_str, actual_value);
    if (val_in == actual_value)
    {
      RCLCPP_INFO_STREAM(nh_->get_logger(), " - " << feature_str << " set to " << actual_value);
    }
    else
    {
      RCLCPP_WARN_STREAM(nh_->get_logger(), " - Tried to set " << feature_str << " to " << val_in
                                                               << " but the camera used " << actual_value
                                                               << " instead");
      val_out = actual_value;
    }
  }
  else
  {
    RCLCPP_ERROR_STREAM(nh_->get_logger(), " - Failed to set " << feature_str << " to " << actual_value);
    val_out = val_in;
  }
}

// Template function to RUN a command
bool AvtVimbaCamera::runCommand(const std::string& command_str)
{
  FeaturePtr feature_ptr;
  VmbErrorType err = vimba_camera_ptr_->GetFeatureByName(command_str.c_str(), feature_ptr);
  if (err == VmbErrorSuccess)
  {
    err = feature_ptr->RunCommand();
    if (err == VmbErrorSuccess)
    {
      bool is_command_done = false;
      do
      {
        err = feature_ptr->IsCommandDone(is_command_done);
        if (err != VmbErrorSuccess)
        {
          break;
        }
        RCLCPP_DEBUG_STREAM_THROTTLE(nh_->get_logger(), clock_, 1000,
                                     "Waiting for command " << command_str.c_str() << "...");
      } while (false == is_command_done);
      RCLCPP_DEBUG_STREAM(nh_->get_logger(), "Command " << command_str.c_str() << " done!");
      return true;
    }
    else
    {
      RCLCPP_WARN_STREAM(nh_->get_logger(),
                         "Could not run command " << command_str << ". Error: " << api_.errorCodeToMessage(err));
      return false;
    }
  }
  else
  {
    RCLCPP_WARN_STREAM(nh_->get_logger(),
                       "Could not get feature command " << command_str << ". Error: " << api_.errorCodeToMessage(err));
    return false;
  }
}

void AvtVimbaCamera::initConfig()
{
  // Add param callback to catch any changes to params during operation
  param_sub_ =
      nh_->add_on_set_parameters_callback(std::bind(&AvtVimbaCamera::parameterCallback, this, std::placeholders::_1));

  if (!opened_)
  {
    RCLCPP_ERROR(nh_->get_logger(), "Can't configure camera. It needs to be opened first");
    return;
  }

  VmbErrorType err;
  uint32_t writable_count = 0;

  // Fetch all features of our cam
  FeaturePtrVector features;
  err = vimba_camera_ptr_->GetFeatures(features);
  if (err == VmbErrorSuccess)
  {
    on_init_config_ = true;
    RCLCPP_INFO(nh_->get_logger(), "Configuring camera:");
    // Query all camera features to translate into ROS params
    for (const FeaturePtr feature : features)
    {
      std::string feature_name = "";
      bool is_writable = false;
      if (createParamFromFeature(feature, feature_name, is_writable))
      {
        writable_count = (is_writable) ? writable_count + 1 : writable_count;
      }
    }
  }
  else
  {
    RCLCPP_ERROR_STREAM(nh_->get_logger(), "Could not get features. Error code: " << api_.errorCodeToMessage(err));
  }
  updateCameraInfo();

  RCLCPP_INFO(nh_->get_logger(),
              "Found %d features on the camera, %u of which are writable. All features are exposed as ROS params",
              writable_features_.size(), writable_count);
  on_init_config_ = false;
}

bool AvtVimbaCamera::createParamFromFeature(const FeaturePtr feature, std::string& feature_name, bool& is_writable)
{
  VmbErrorType err;

  err = feature->GetName(feature_name);
  if (err != VmbErrorSuccess)
  {
    RCLCPP_ERROR_STREAM(nh_->get_logger(), "[Could not get feature Name. Error code: " << err << "]");
    return false;
  }

  err = feature->IsWritable(is_writable);
  if (err != VmbErrorSuccess)
  {
    RCLCPP_ERROR_STREAM(nh_->get_logger(), "[Could not get write access. Error code: " << err << "]");
  }
  writable_features_[feature_name] = is_writable;

  rcl_interfaces::msg::ParameterDescriptor descriptor;

  // Create description for ROS param with category and units.
  std::string annotated_description = "";

  std::string category;
  err = feature->GetCategory(category);
  if (err == VmbErrorSuccess && category != "")
  {
    annotated_description += "[" + category + "]";
  }

  std::string unit;
  err = feature->GetUnit(unit);
  if (err == VmbErrorSuccess && unit != "")
  {
    annotated_description += "(Unit: " + unit + ")";
  }

  std::string description;
  err = feature->GetDescription(description);
  if (err == VmbErrorSuccess)
  {
    annotated_description += " " + description;
  }

  descriptor.description = annotated_description;
  descriptor.read_only = !is_writable;

  // Not supported in ROS Foxy
  // descriptor.dynamic_typing = false;

  VmbFeatureDataType type;
  err = feature->GetDataType(type);
  if (err != VmbErrorSuccess)
  {
    RCLCPP_ERROR_STREAM(nh_->get_logger(), "Could not get feature Data Type. Error code: " << err);
  }
  else
  {
    switch (type)
    {
      case VmbFeatureDataBool: {
        bool initial_value = false;

        feature->GetValue(initial_value);

        nh_->declare_parameter<bool>(PARAM_NAMESPACE + feature_name, initial_value, descriptor);
        break;
      }
      case VmbFeatureDataInt: {
        VmbInt64_t initial_value = 0;
        VmbInt64_t minimum_value = std::numeric_limits<int64_t>::lowest();
        VmbInt64_t maximum_value = std::numeric_limits<int64_t>::max();
        VmbInt64_t step = 0.0;

        feature->GetValue(initial_value);
        feature->GetRange(minimum_value, maximum_value);
        feature->GetIncrement(step);
        initial_value = std::max(minimum_value, std::min(initial_value, maximum_value));

        rcl_interfaces::msg::IntegerRange int_range;
        int_range.from_value = minimum_value;
        int_range.to_value = maximum_value;
        int_range.step = step;
        descriptor.integer_range.push_back(int_range);

        nh_->declare_parameter<int64_t>(PARAM_NAMESPACE + feature_name, initial_value, descriptor);
        break;
      }
      case VmbFeatureDataFloat: {
        double initial_value = 0.0;
        double minimum_value = std::numeric_limits<double>::lowest();
        double maximum_value = std::numeric_limits<double>::max();
        double step = 0.0;

        feature->GetValue(initial_value);
        feature->GetRange(minimum_value, maximum_value);
        feature->GetIncrement(step);
        
        initial_value = std::max(maximum_value, std::min(initial_value, minimum_value));

        rcl_interfaces::msg::FloatingPointRange float_range;
        float_range.from_value = minimum_value;
        float_range.to_value = maximum_value;
        // TODO: step is not working when feature is set to Continuous
        // float_range.step = step;

        descriptor.floating_point_range.push_back(float_range);
        
        nh_->declare_parameter<double>(PARAM_NAMESPACE + feature_name, initial_value, descriptor);
        break;
      }
      case VmbFeatureDataString: {
        std::string initial_value = "";

        feature->GetValue(initial_value);

        nh_->declare_parameter<std::string>(PARAM_NAMESPACE + feature_name, initial_value, descriptor);
        break;
      }
      case VmbFeatureDataEnum: {
        std::string initial_value = "";
        EnumEntryVector enum_entries;

        feature->GetValue(initial_value);

        if (feature->GetEntries(enum_entries) == VmbErrorSuccess)
        {
          std::string enum_description = "Acceptable values: ";
          bool first = true;
          for (const auto& entry : enum_entries)
          {
            std::string entry_name;
            if (entry.GetName(entry_name) == VmbErrorSuccess)
            {
              entry_name = first ? entry_name : ", " + entry_name;
              enum_description += entry_name;
              first = false;
            }
          }
          descriptor.additional_constraints = enum_description;
        }

        nh_->declare_parameter<std::string>(PARAM_NAMESPACE + feature_name, initial_value, descriptor);
        break;
      }
      default: {
        break;
      }
    }
  }
  return true;
}

rcl_interfaces::msg::SetParametersResult
AvtVimbaCamera::parameterCallback(const std::vector<rclcpp::Parameter>& parameters)
{
  rcl_interfaces::msg::SetParametersResult result;
  result.successful = true;
  result.reason = "success";

  for (const auto& param : parameters)
  {
    std::string param_name = param.get_name();
    std::string feature_name = param_name.substr(param_name.find('/') + 1, std::string::npos);

    if (writable_features_.count(feature_name))
    {
      if (writable_features_[feature_name])
      {

        // Stop imaging since parameter changes can affect the image frames being received
        if (streaming_ && !on_init_config_)
        {
          stopImaging();
          std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        std::unique_lock<std::mutex> lock(config_mutex_);
        rclcpp::ParameterType param_type = param.get_type();

        switch (param_type)
        {
          case rclcpp::PARAMETER_BOOL: {
            bool value;
            configureFeature(feature_name, param.get_value<bool>(), value);
            break;
          }
          case rclcpp::PARAMETER_INTEGER: {
            int64_t value;
            configureFeature(feature_name, static_cast<VmbInt64_t>(param.get_value<int64_t>()), value);
            break;
          }
          case rclcpp::PARAMETER_DOUBLE: {
            double value;
            configureFeature(feature_name, param.get_value<double>(), value);
            break;
          }
          case rclcpp::PARAMETER_STRING: {
            std::string value;
            configureFeature(feature_name, param.get_value<std::string>(), value);
            break;
          }
          default:
            break;
        }

        // Update camera_info if certain features changed
        if (cam_info_features_.count(feature_name) && !on_init_config_)
        {
          updateCameraInfo();
        }

        if (!(on_init_config_ || force_stopped_))
        {
          startImaging();
        }
      }
      else
      {
        if (!on_init_config_)
        {
          result.reason = "Parameter " + param_name + " is a read-only camera feature";
          result.successful = false;
        }
      }
    }
  }

  return result;
}

void AvtVimbaCamera::updateCameraInfo()
{
  sensor_msgs::msg::CameraInfo ci = info_man_->getCameraInfo();

  // Set the operational parameters in CameraInfo (binning, ROI)
  int binning_or_decimation_x = getBinningOrDecimationX();
  int binning_or_decimation_y = getBinningOrDecimationY();

  // Set the operational parameters in CameraInfo (binning, ROI)
  int sensor_width = getSensorWidth();
  int sensor_height = getSensorHeight();

  if (sensor_width == -1 || sensor_height == -1)
  {
    RCLCPP_ERROR(nh_->get_logger(), "Could not determine sensor pixel dimensions, camera_info will be wrong");
  }

  ci.width = sensor_width;
  ci.height = sensor_height;
  ci.binning_x = binning_or_decimation_x;
  ci.binning_y = binning_or_decimation_y;

  // ROI is in unbinned coordinates, need to scale up
  ci.roi.width = getImageWidth() * binning_or_decimation_x;
  ci.roi.height = getImageHeight() * binning_or_decimation_y;
  ci.roi.x_offset = 0 * binning_or_decimation_x;
  ci.roi.y_offset = 0 * binning_or_decimation_y;

  bool roi_is_full_image = (ci.roi.width == ci.width && ci.roi.height == ci.height);
  ci.roi.do_rectify = !roi_is_full_image;

  // push the changes to manager
  info_man_->setCameraInfo(ci);

  RCLCPP_INFO(nh_->get_logger(), "Camera info updated");
}

void AvtVimbaCamera::getCurrentState(diagnostic_updater::DiagnosticStatusWrapper& stat)
{
  stat.add("ID", guid_);
  stat.add("Info", diagnostic_msg_);
  stat.add("Temperature", getDeviceTemp());

  switch (camera_state_)
  {
    case OPENING:
      stat.summary(diagnostic_msgs::msg::DiagnosticStatus::WARN, "Opening camera");
      break;
    case IDLE:
      stat.summary(diagnostic_msgs::msg::DiagnosticStatus::OK, "Camera is idle");
      break;
    case OK:
      stat.summary(diagnostic_msgs::msg::DiagnosticStatus::OK, "Camera is streaming");
      break;
    case CAMERA_NOT_FOUND:
      stat.summaryf(diagnostic_msgs::msg::DiagnosticStatus::ERROR, "Cannot find requested camera %s", guid_.c_str());
      break;
    case FORMAT_ERROR:
      stat.summary(diagnostic_msgs::msg::DiagnosticStatus::ERROR, "Problem retrieving frame");
      break;
    case ERROR:
      stat.summary(diagnostic_msgs::msg::DiagnosticStatus::ERROR, "Camera has encountered an error");
      break;
    default:
      break;
  }
}

bool AvtVimbaCamera::loadCameraSettings(const std::string& filename)
{
  stopImaging();
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  vimba_camera_ptr_->LoadSaveSettingsSetup(VmbFeaturePersistNoLUT, 5, 4);
  auto err = vimba_camera_ptr_->LoadCameraSettings(filename);

  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  startImaging();

  if (err != VmbErrorSuccess)
  {
    RCLCPP_ERROR(nh_->get_logger(), "Failed to load camera settings from %s", filename.c_str());
    return false;
  }
  RCLCPP_INFO(nh_->get_logger(), "Loaded camera settings from %s", filename.c_str());
  return true;
}

bool AvtVimbaCamera::saveCameraSettings(const std::string& filename)
{
  stopImaging();
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  vimba_camera_ptr_->LoadSaveSettingsSetup(VmbFeaturePersistNoLUT, 5, 4);
  auto err = vimba_camera_ptr_->SaveCameraSettings(filename);

  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  startImaging();

  if (err != VmbErrorSuccess)
  {
    RCLCPP_ERROR(nh_->get_logger(), "Failed to save camera settings to %s", filename.c_str());
    return false;
  }
  RCLCPP_INFO(nh_->get_logger(), "Saved camera settings to %s", filename.c_str());
  return true;
}

}  // namespace avt_vimba_camera
