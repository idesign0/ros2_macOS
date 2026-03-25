// Copyright (c) 2024 Fraunhofer IOSB and contributors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
//    * Neither the name of the Fraunhofer IOSB nor the names of its
//      contributors may be used to endorse or promote products derived from
//      this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include "camera_aravis2/camera_driver.h"

// ROS
#include <rcl_interfaces/msg/floating_point_range.hpp>
#include <rcl_interfaces/msg/integer_range.hpp>
#include <rclcpp/time.hpp>

/// Conversions from integers to Arv types.
static const char* ARV_BUFFER_STATUS_FROM_INT[] =
  {"ARV_BUFFER_STATUS_SUCCESS", "ARV_BUFFER_STATUS_CLEARED",
   "ARV_BUFFER_STATUS_TIMEOUT", "ARV_BUFFER_STATUS_MISSING_PACKETS",
   "ARV_BUFFER_STATUS_WRONG_PACKET_ID", "ARV_BUFFER_STATUS_SIZE_MISMATCH",
   "ARV_BUFFER_STATUS_FILLING", "ARV_BUFFER_STATUS_ABORTED"};

namespace camera_aravis2
{

//==================================================================================================
CameraDriver::CameraDriver(const std::string& name, const rclcpp::NodeOptions& options) :
  CameraAravisNodeBase(name, options),
  is_spawning_(false),
  p_white_balance_srv_(nullptr),
  p_parameter_callback_handle_(nullptr),
  is_diagnostics_published_(false),
  p_diagnostic_pub_(nullptr),
  current_num_subscribers_(0)
{
}

//==================================================================================================
CameraDriver::~CameraDriver()
{
}

//==================================================================================================
bool CameraDriver::isSpawning() const
{
    return (is_spawning_);
}

//==================================================================================================
void CameraDriver::setupParameters()
{
    //--- call method of parent class
    CameraAravisNodeBase::setupParameters();

    //--- stream parameters

    auto stream_names_desc = rcl_interfaces::msg::ParameterDescriptor{};
    stream_names_desc.description =
      "String list of names that are to be associated with each stream. "
      "If multiple streams are available, these names will be appended "
      "to the topic names in order to distinguish the different image "
      "streams. If omitted or less names are given than streams available, "
      "each stream will get given a name based on its ID, starting with 0.";
    stream_names_desc.read_only = true;
    declare_parameter<std::vector<std::string>>("stream_names", std::vector<std::string>({}),
                                                stream_names_desc);

    auto camera_info_urls_desc = rcl_interfaces::msg::ParameterDescriptor{};
    camera_info_urls_desc.description =
      "String list of urls to camera_info files associated with "
      "each stream. List should have the same length as the number "
      "of streams provided by the camera. If the number of URLs does "
      "not correspond to number of streams available, the minimum of "
      "both is used to set the number of streams that are to be established. "
      "If omitted, it is constructed from the camera GUID located within the "
      "current working directory, with the stream name separated by '_' appended "
      "to the file name, if more than one streams are instantiated.";
    camera_info_urls_desc.read_only = true;
    declare_parameter<std::vector<std::string>>("camera_info_urls", std::vector<std::string>({}),
                                                camera_info_urls_desc);

    auto frame_id_desc = rcl_interfaces::msg::ParameterDescriptor{};
    frame_id_desc.description =
      "Frame ID that is to be associated with the sensor and, in turn, "
      "with the image data. If multiple streams are supported by the "
      "camera, the given ID serves as a base string to which the "
      "stream name is appended, together with '_' as separator. If no "
      "frame ID is specified, the name of the node will be used.";
    frame_id_desc.read_only = true;
    declare_parameter<std::string>("frame_id", "", frame_id_desc);

    auto dyn_param_yaml_url_desc = rcl_interfaces::msg::ParameterDescriptor{};
    dyn_param_yaml_url_desc.description =
      "URL to yaml file specifying camera parameters that are to be made dynamically changeable. "
      "If left empty (as default) no dynamic parameters, apart from the camera_aravis-specific "
      "parameters will be available.";
    dyn_param_yaml_url_desc.read_only = true;
    declare_parameter<std::string>("dynamic_parameters_yaml_url", "",
                                   dyn_param_yaml_url_desc);

    auto diag_yaml_url_desc = rcl_interfaces::msg::ParameterDescriptor{};
    diag_yaml_url_desc.description =
      "URL to yaml file specifying the camera features which are to be monitored. If left "
      "empty (as default) no diagnostic features will be read and published.";
    diag_yaml_url_desc.read_only = true;
    declare_parameter<std::string>("diagnostic_yaml_url", "",
                                   diag_yaml_url_desc);

    auto diag_pub_rate_desc = rcl_interfaces::msg::ParameterDescriptor{};
    diag_pub_rate_desc.description =
      "Rate at which to read and publish the diagnostic data.";
    diag_pub_rate_desc.read_only = true;
    declare_parameter<double>("diagnostic_publishing_rate", 0.1, diag_pub_rate_desc);

    auto verbose_desc = rcl_interfaces::msg::ParameterDescriptor{};
    verbose_desc.description =
      "Activate verbose output.";
    declare_parameter<bool>("verbose", false, verbose_desc);

    //--- register parameter change callback
    p_parameter_callback_handle_ = add_on_set_parameters_callback(
      std::bind(&CameraDriver::handleDynamicParameterChange, this, std::placeholders::_1));
}

//==================================================================================================
bool CameraDriver::setupCameraStreamStructs()
{
    //--- get number of streams and associated names

    int num_streams = discoverNumberOfStreams();

    auto stream_names     = get_parameter("stream_names").as_string_array();
    auto camera_info_urls = get_parameter("camera_info_urls").as_string_array();
    auto base_frame_id    = get_parameter("frame_id").as_string();

    bool is_camera_info_url_param_empty = camera_info_urls.empty();

    //--- check if one camera_info_url is specified
    if (is_camera_info_url_param_empty)
    {
        RCLCPP_WARN(logger_, "No camera_info_url specified. Initializing from camera GUID.");

        // Here only empty strings are pushed into list as placeholders. The actual construction of
        // the url is done later, when the stream names are set.
        for (int i = 0; i < num_streams; i++)
            camera_info_urls.push_back("");
    }

    //--- check if number of camera_info_url corresponds to available number of streams
    if (static_cast<int>(camera_info_urls.size()) != num_streams)
    {
        if (static_cast<int>(camera_info_urls.size()) > num_streams)
        {
            camera_info_urls.resize(num_streams);
            RCLCPP_WARN(logger_,
                        "Insufficient number of streams supported by camera.");
            RCLCPP_WARN(logger_,
                        "Truncating 'camera_info_urls' to first %i elements.",
                        num_streams);
        }
        else
        {
            RCLCPP_WARN(logger_,
                        "Insufficient 'camera_info_urls' specified.");
            RCLCPP_WARN(logger_,
                        "only instantiating %i streams (corresponding to  number of "
                        "'camera_info_urls').",
                        static_cast<int>(camera_info_urls.size()));
            num_streams = static_cast<int>(camera_info_urls.size());
        }
    }

    //--- check if frame_id is empty
    if (base_frame_id.empty())
    {
        base_frame_id = this->get_fully_qualified_name();
        base_frame_id.replace(0, 1, "");  // remove first '/'
        std::replace(base_frame_id.begin(), base_frame_id.end(), '/', '_');
    }

    //--- set up structs
    const std::string GUID_STR = CameraAravisNodeBase::constructCameraGuidStr(p_camera_);
    streams_                   = std::vector<Stream>(num_streams);
    for (uint i = 0; i < streams_.size(); ++i)
    {
        Stream& stream = streams_[i];

        //--- if no or insufficient stream names are specified, use ID as name
        stream.name            = (i < stream_names.size())
                                   ? stream_names[i]
                                   : ("stream" + std::to_string(i));
        stream.sensor.frame_id = (!stream_names.empty() || num_streams > 1)
                                   ? base_frame_id + "_" + stream.name
                                   : base_frame_id;

        //--- get camera_info url
        if (is_camera_info_url_param_empty)
        {
            if (streams_.size() > 1)
                stream.camera_info_url = "file://" + GUID_STR + "_" + stream.name + ".yaml";
            else
                stream.camera_info_url = "file://" + GUID_STR + ".yaml";

            // replace whitespaces in url (coming from GUID) with '_'
            std::replace(stream.camera_info_url.begin(), stream.camera_info_url.end(), ' ', '_');
        }
        else
        {
            stream.camera_info_url = camera_info_urls[i];

            //--- add 'file://' to beginning of camera_info_url
            if (stream.camera_info_url.find("file://") != 0)
                stream.camera_info_url = "file://" + stream.camera_info_url;
        }

        //--- setup image topic and create publisher
        std::string topic_name = this->get_name();
        // p_transport            = new image_transport::ImageTransport(pnh);
        // if more than one stream available, add stream name
        if (!stream_names.empty() || num_streams > 1)
            topic_name += "/" + stream.name;
        topic_name += "/image_raw";

#ifndef WITH_MATCHED_EVENTS
        stream.camera_pub = image_transport::create_camera_publisher(this, topic_name);
#else
        rclcpp::PublisherOptions pub_options;
        pub_options.event_callbacks.matched_callback =
          std::bind(&CameraDriver::handleMessageSubscriptionChange, this,
                    std::placeholders::_1);
        stream.camera_pub =
          image_transport::create_camera_publisher(this, topic_name,
                                                   rmw_qos_profile_default, pub_options);
#endif

        //--- initialize camera_info manager
        // NOTE: Previously, separate node handles where used for CameraInfoManagers in case of a
        // Multi-source Camera. Uncertain if this is still relevant.
        stream.p_camera_info_manager.reset(
          new camera_info_manager::CameraInfoManager(this, stream.sensor.frame_id));
        bool is_successful = stream.p_camera_info_manager->loadCameraInfo(stream.camera_info_url);
        if (!is_successful)
            return false;
    }

    //--- check if at least one stream was initialized
    if (streams_.empty())
    {
        RCLCPP_FATAL(logger_, "Something went wrong in the initialization of the camera streams.");
        return false;
    }

    return true;
}

//==================================================================================================
void CameraDriver::filterParameterListNames(
  std::vector<std::pair<std::string, rclcpp::ParameterValue>>& parameter_list)
{
    int number;
    char new_name[100];
    for (auto& param : parameter_list)
    {
        if (sscanf(param.first.c_str(), "#%d_%s", &number, new_name) < 2)
            continue;

        param.first = std::string(new_name);
    }
}

//==================================================================================================
[[nodiscard]] bool CameraDriver::getDeviceControlParameterList(
  const std::string& param_name,
  std::vector<std::pair<std::string, rclcpp::ParameterValue>>& param_values) const
{
    return getNestedParameterList("DeviceControl", param_name, param_values);
}

//==================================================================================================
[[nodiscard]] bool CameraDriver::setDeviceControlSettings()
{
    GuardedGError err;

    rclcpp::ParameterValue tmp_param_value;
    std::vector<std::pair<std::string, rclcpp::ParameterValue>> tmp_param_values;

    bool is_parameter_set;

    //--- get and set given custom features at beginning of image format control
    RCLCPP_DEBUG(logger_, "Evaluating 'DeviceControl.*'.");
    is_parameter_set = getDeviceControlParameterList("", tmp_param_values);
    if (is_parameter_set)
        setFeatureValuesFromParameterList(tmp_param_values);

    return true;
}

//==================================================================================================
[[nodiscard]] bool CameraDriver::getTransportLayerControlParameter(
  const std::string& param_name,
  rclcpp::ParameterValue& param_value) const
{
    return getNestedParameter("TransportLayerControl", param_name, param_value);
}

//==================================================================================================
[[nodiscard]] bool CameraDriver::getTransportLayerControlParameterList(
  const std::string& param_name,
  std::vector<std::pair<std::string, rclcpp::ParameterValue>>& param_values) const
{
    return getNestedParameterList("TransportLayerControl", param_name, param_values);
}

//==================================================================================================
[[nodiscard]] bool CameraDriver::setTransportLayerControlSettings()
{
    bool isSuccessful = true;

    GuardedGError err;

    std::string tmp_feature_name;
    rclcpp::ParameterValue tmp_param_value;
    std::vector<std::pair<std::string, rclcpp::ParameterValue>> tmp_param_values;

    bool is_parameter_set;

    //--- get and set given custom features at beginning of transport layer control
    tmp_feature_name = "BEGIN";
    RCLCPP_DEBUG(logger_, "Evaluating 'TransportLayerControl.%s'.", tmp_feature_name.c_str());
    is_parameter_set = getTransportLayerControlParameterList(tmp_feature_name,
                                                             tmp_param_values);
    if (is_parameter_set)
    {
        filterParameterListNames(tmp_param_values);
        setFeatureValuesFromParameterList(tmp_param_values);
    }

    //--- set technology specific transport layer controls
    isSuccessful &= setTechSpecificTlControlSettings();

    //--- get and set given custom features at end of transport layer control
    tmp_feature_name = "END";
    RCLCPP_DEBUG(logger_, "Evaluating 'TransportLayerControl.%s'.",
                 tmp_feature_name.c_str());
    is_parameter_set = getTransportLayerControlParameterList(tmp_feature_name,
                                                             tmp_param_values);
    if (is_parameter_set)
    {
        filterParameterListNames(tmp_param_values);
        setFeatureValuesFromParameterList(tmp_param_values);
    }

    return isSuccessful;
}

//==================================================================================================
[[nodiscard]] bool CameraDriver::getImageFormatControlParameter(
  const std::string& param_name,
  rclcpp::ParameterValue& param_value) const
{
    return getNestedParameter("ImageFormatControl", param_name, param_value);
}

//==================================================================================================
[[nodiscard]] bool CameraDriver::getImageFormatControlParameterList(
  const std::string& param_name,
  std::vector<std::pair<std::string, rclcpp::ParameterValue>>& param_values) const
{
    return getNestedParameterList("ImageFormatControl", param_name, param_values);
}

//==================================================================================================
[[nodiscard]] bool CameraDriver::setImageFormatControlSettings()
{
    GuardedGError err;

    for (uint i = 0; i < streams_.size(); ++i)
    {
        // NOTE: Not all parameters are essential, which is why only the success of some parameters
        // are checked.
        bool is_successful = true;

        Stream& stream      = streams_[i];
        Sensor& sensor      = stream.sensor;
        ImageRoi& image_roi = stream.image_roi;

        std::string tmp_feature_name;
        rclcpp::ParameterValue tmp_param_value;
        std::vector<std::pair<std::string, rclcpp::ParameterValue>> tmp_param_values;
        gint64 tmp_min_int, tmp_max_int;

        bool is_parameter_set;

        //--- set source, if applicable
        if (streams_.size() > 1)
        {
            // TODO(boitumeloruf): make more source selector more generic
            std::string src_selector_val = "Source" + std::to_string(i);
            setFeatureValue<std::string>("SourceSelector", src_selector_val);
        }

        //--- get and set given custom features at beginning of image format control
        tmp_feature_name = "BEGIN";
        RCLCPP_DEBUG(logger_, "Evaluating 'ImageFormatControl.%s' for stream %i.",
                     tmp_feature_name.c_str(), i);
        is_parameter_set = getImageFormatControlParameterList(tmp_feature_name,
                                                              tmp_param_values);
        if (is_parameter_set)
        {
            filterParameterListNames(tmp_param_values);
            setFeatureValuesFromParameterList(tmp_param_values, i);
        }

        //--- for specific parameters, proceed as follows:
        //---   1. Check and read parameter from launch parameters
        //---   2. If provided in launch parameter, set value on device
        //---   3. Get the set value from the device and store it in corresponding struct
        //---   4. If specific parameter was given in the launch parameters, check against the
        //---       actual value which has been read from the camera and issue a warning message
        //---       if applicable
        //---
        //---   For bounded feature values (e.g. image size), get the bounds prior to setting
        //---   the value and use appropriate method that will truncate value to bounds

        //--- PixelFormat
        tmp_feature_name = "PixelFormat";
        RCLCPP_DEBUG(logger_, "Evaluating 'ImageFormatControl.%s' for stream %i.",
                     tmp_feature_name.c_str(), i);
        is_parameter_set = getImageFormatControlParameter(tmp_feature_name, tmp_param_value);
        if (is_parameter_set)
            setFeatureValueFromParameter<std::string>(tmp_feature_name, tmp_param_value, i);
        getFeatureValue<std::string>(tmp_feature_name, sensor.pixel_format);
        if (is_parameter_set &&
            !isParameterValueEqualTo<std::string>(tmp_param_value, sensor.pixel_format, i))
            config_warn_msgs_.push_back("Stream " + std::to_string(i) + ": " +
                                        "'" + tmp_feature_name + "' is not as specified.");

        //--- get conversion function and number of bits per pixel corresponding to pixel format
        const auto itr = CONVERSIONS_DICTIONARY.find(sensor.pixel_format);
        if (itr != CONVERSIONS_DICTIONARY.end())
            stream.cvt_pixel_format = itr->second;
        else
            config_warn_msgs_.push_back("Stream " + std::to_string(i) + ": " +
                                        "There is no known conversion from '" +
                                        sensor.pixel_format +
                                        "' to a usual ROS image encoding. " +
                                        "Likely you need to implement one.");

        //--- get bits per pixel
        sensor.n_bits_pixel =
          ARV_PIXEL_FORMAT_BIT_PER_PIXEL(arv_camera_get_pixel_format(p_camera_, err.ref()));
        CHECK_SUCCESS_GERROR_MSG(err, logger_, "In getting 'Bits per Pixel'.", is_successful);

        //--- get sensor size
        RCLCPP_DEBUG(logger_, "Evaluating 'ImageFormatControl.SensorWidth' and "
                              "'ImageFormatControl.SensorHeight' for stream %i.",
                     i);
        getFeatureValue<int>("SensorWidth", sensor.width);
        getFeatureValue<int>("SensorHeight", sensor.height);

        //--- horizontal and vertical flip
        tmp_feature_name = "ReverseX";
        RCLCPP_DEBUG(logger_, "Evaluating 'ImageFormatControl.%s' for stream %i.",
                     tmp_feature_name.c_str(), i);
        is_parameter_set = getImageFormatControlParameter(tmp_feature_name, tmp_param_value);
        if (is_parameter_set)
            setFeatureValueFromParameter<bool>(tmp_feature_name, tmp_param_value, i);
        getFeatureValue<bool>(tmp_feature_name, sensor.reverse_x);
        if (is_parameter_set &&
            !isParameterValueEqualTo<bool>(tmp_param_value, sensor.reverse_x, i))
            config_warn_msgs_.push_back("Stream " + std::to_string(i) + ": " +
                                        "'" + tmp_feature_name + "' is not as specified.");

        tmp_feature_name = "ReverseY";
        RCLCPP_DEBUG(logger_, "Evaluating 'ImageFormatControl.%s' for stream %i.",
                     tmp_feature_name.c_str(), i);
        is_parameter_set = getImageFormatControlParameter(tmp_feature_name, tmp_param_value);
        if (is_parameter_set)
            setFeatureValueFromParameter<bool>(tmp_feature_name, tmp_param_value, i);
        getFeatureValue<bool>(tmp_feature_name, sensor.reverse_y);
        if (is_parameter_set &&
            !isParameterValueEqualTo<bool>(tmp_param_value, sensor.reverse_y, i))
            config_warn_msgs_.push_back("Stream " + std::to_string(i) + ": " +
                                        "'" + tmp_feature_name + "' is not as specified.");

        //--- horizontal and vertical binning
        tmp_feature_name = "BinningHorizontal";
        RCLCPP_DEBUG(logger_, "Evaluating 'ImageFormatControl.%s' for stream %i.",
                     tmp_feature_name.c_str(), i);
        is_parameter_set = getImageFormatControlParameter(tmp_feature_name, tmp_param_value);
        if (is_parameter_set)
            setFeatureValueFromParameter<int64_t>(tmp_feature_name, tmp_param_value, i);
        getFeatureValue<int>(tmp_feature_name, sensor.binning_x);
        if (is_parameter_set &&
            !isParameterValueEqualTo<int64_t>(tmp_param_value, sensor.binning_x, i))
            config_warn_msgs_.push_back("Stream " + std::to_string(i) + ": " +
                                        "'" + tmp_feature_name + "' is not as specified.");

        tmp_feature_name = "BinningHorizontalMode";
        RCLCPP_DEBUG(logger_, "Evaluating 'ImageFormatControl.%s' for stream %i.",
                     tmp_feature_name.c_str(), i);
        is_parameter_set = getImageFormatControlParameter(tmp_feature_name, tmp_param_value);
        if (is_parameter_set)
            setFeatureValueFromParameter<std::string>(tmp_feature_name, tmp_param_value, i);
        getFeatureValue<std::string>(tmp_feature_name, sensor.binning_mode_x);
        if (is_parameter_set &&
            !isParameterValueEqualTo<std::string>(tmp_param_value, sensor.binning_mode_x, i))
            config_warn_msgs_.push_back("Stream " + std::to_string(i) + ": " +
                                        "'" + tmp_feature_name + "' is not as specified.");

        tmp_feature_name = "BinningVertical";
        RCLCPP_DEBUG(logger_, "Evaluating 'ImageFormatControl.%s' for stream %i.",
                     tmp_feature_name.c_str(), i);
        is_parameter_set = getImageFormatControlParameter(tmp_feature_name, tmp_param_value);
        if (is_parameter_set)
            setFeatureValueFromParameter<int64_t>(tmp_feature_name, tmp_param_value, i);
        getFeatureValue<int>(tmp_feature_name, sensor.binning_y);
        if (is_parameter_set &&
            !isParameterValueEqualTo<int64_t>(tmp_param_value, sensor.binning_y, i))
            config_warn_msgs_.push_back("Stream " + std::to_string(i) + ": " +
                                        "'" + tmp_feature_name + "' is not as specified.");

        tmp_feature_name = "BinningVerticalMode";
        RCLCPP_DEBUG(logger_, "Evaluating 'ImageFormatControl.%s' for stream %i.",
                     tmp_feature_name.c_str(), i);
        is_parameter_set = getImageFormatControlParameter(tmp_feature_name, tmp_param_value);
        if (is_parameter_set)
            setFeatureValueFromParameter<std::string>(tmp_feature_name, tmp_param_value, i);
        getFeatureValue<std::string>(tmp_feature_name, sensor.binning_mode_y);
        if (is_parameter_set &&
            !isParameterValueEqualTo<std::string>(tmp_param_value, sensor.binning_mode_y, i))
            config_warn_msgs_.push_back("Stream " + std::to_string(i) + ": " +
                                        "'" + tmp_feature_name + "' is not as specified.");

        //--- image roi width
        //--- separately get the feature bounds in order to have them even if the width is not
        //--- to be set
        tmp_feature_name = "Width";
        tmp_min_int      = 0;
        tmp_max_int      = sensor.width;
        RCLCPP_DEBUG(logger_, "Evaluating 'ImageFormatControl.%s' for stream %i.",
                     tmp_feature_name.c_str(), i);
        arv_device_get_integer_feature_bounds(p_device_, tmp_feature_name.c_str(),
                                              &tmp_min_int, &tmp_max_int, err.ref());
        image_roi.width_min = tmp_min_int,
        image_roi.width_max = tmp_max_int;
        CHECK_GERROR_MSG(err, logger_, "In getting bounds for feature '" + tmp_feature_name + "'.");

        is_parameter_set = getImageFormatControlParameter(tmp_feature_name, tmp_param_value);
        if (is_parameter_set)
            setFeatureValueFromParameter<int64_t>(
              tmp_feature_name, tmp_param_value, tmp_min_int, tmp_max_int, i);
        getFeatureValue<int>(tmp_feature_name, image_roi.width);
        if (is_parameter_set &&
            !isParameterValueEqualTo<int64_t>(tmp_param_value, image_roi.width, i))
            config_warn_msgs_.push_back("Stream " + std::to_string(i) + ": " +
                                        "'" + tmp_feature_name + "' is not as specified.");

        //--- image roi height
        //--- separately get the feature bounds in order to have them even if the height is not
        //--- to be set
        tmp_feature_name = "Height";
        tmp_min_int      = 0;
        tmp_max_int      = sensor.height;
        RCLCPP_DEBUG(logger_, "Evaluating 'ImageFormatControl.%s' for stream %i.",
                     tmp_feature_name.c_str(), i);
        arv_device_get_integer_feature_bounds(p_device_, tmp_feature_name.c_str(),
                                              &tmp_min_int, &tmp_max_int, err.ref());
        image_roi.height_min = tmp_min_int,
        image_roi.height_max = tmp_max_int;
        CHECK_GERROR_MSG(err, logger_, "In getting bounds for feature '" + tmp_feature_name + "'.");

        is_parameter_set = getImageFormatControlParameter(tmp_feature_name, tmp_param_value);
        if (is_parameter_set)
            setFeatureValueFromParameter<int64_t>(
              tmp_feature_name, tmp_param_value, tmp_min_int, tmp_max_int, i);
        getFeatureValue<int>(tmp_feature_name, image_roi.height);
        if (is_parameter_set &&
            !isParameterValueEqualTo<int64_t>(tmp_param_value, image_roi.height, i))
            config_warn_msgs_.push_back("Stream " + std::to_string(i) + ": " +
                                        "'" + tmp_feature_name + "' is not as specified.");

        //--- image roi offset x
        tmp_feature_name = "OffsetX";
        RCLCPP_DEBUG(logger_, "Evaluating 'ImageFormatControl.%s' for stream %i.",
                     tmp_feature_name.c_str(), i);
        is_parameter_set = getImageFormatControlParameter(tmp_feature_name, tmp_param_value);
        if (is_parameter_set)
            setFeatureValueFromParameter<int64_t>(tmp_feature_name, tmp_param_value, i);
        getFeatureValue<int>(tmp_feature_name, image_roi.x);
        if (is_parameter_set &&
            !isParameterValueEqualTo<int64_t>(tmp_param_value, image_roi.x, i))
            config_warn_msgs_.push_back("Stream " + std::to_string(i) + ": " +
                                        "'" + tmp_feature_name + "' is not as specified.");

        //--- image roi height
        tmp_feature_name = "OffsetY";
        RCLCPP_DEBUG(logger_, "Evaluating 'ImageFormatControl.%s' for stream %i.",
                     tmp_feature_name.c_str(), i);
        is_parameter_set = getImageFormatControlParameter(tmp_feature_name, tmp_param_value);
        if (is_parameter_set)
            setFeatureValueFromParameter<int64_t>(tmp_feature_name, tmp_param_value, i);
        getFeatureValue<int>(tmp_feature_name, image_roi.y);
        if (is_parameter_set &&
            !isParameterValueEqualTo<int64_t>(tmp_param_value, image_roi.y, i))
            config_warn_msgs_.push_back("Stream " + std::to_string(i) + ": " +
                                        "'" + tmp_feature_name + "' is not as specified.");

        //--- get and set given custom features at end of image format control
        tmp_feature_name = "END";
        RCLCPP_DEBUG(logger_, "Evaluating 'ImageFormatControl.%s' for stream %i.",
                     tmp_feature_name.c_str(), i);
        is_parameter_set = getImageFormatControlParameterList(tmp_feature_name,
                                                              tmp_param_values);
        if (is_parameter_set)
        {
            filterParameterListNames(tmp_param_values);
            setFeatureValuesFromParameterList(tmp_param_values, i);
        }

        // NOTE: Not all parameters are essential, which is why only the success of some parameters
        // are checked.
        if (!is_successful)
            return false;
    }

    return true;
}

//==================================================================================================
[[nodiscard]] bool CameraDriver::getAcquisitionControlParameter(
  const std::string& param_name,
  rclcpp::ParameterValue& param_value) const
{
    return getNestedParameter("AcquisitionControl", param_name, param_value);
}

//==================================================================================================
[[nodiscard]] bool CameraDriver::getAcquisitionControlParameterList(
  const std::string& param_name,
  std::vector<std::pair<std::string, rclcpp::ParameterValue>>& param_values) const
{
    return getNestedParameterList("AcquisitionControl", param_name, param_values);
}

//==================================================================================================
[[nodiscard]] bool CameraDriver::setAcquisitionControlSettings()
{
    GuardedGError err;

    for (uint i = 0; i < streams_.size(); ++i)
    {
        Stream& stream               = streams_[i];
        AcquisitionControl& acq_ctrl = stream.acquisition_control;

        std::string tmp_feature_name;
        rclcpp::ParameterValue tmp_param_value;
        std::vector<std::pair<std::string, rclcpp::ParameterValue>> tmp_param_values;

        gdouble tmp_min_dbl,
          tmp_max_dbl;

        bool is_parameter_set;

        //--- set source, if applicable
        if (streams_.size() > 1)
        {
            // TODO(boitumeloruf): make more source selector more generic
            std::string src_selector_val = "Source" + std::to_string(i);
            setFeatureValue<std::string>("SourceSelector", src_selector_val);
        }

        //--- get and set given custom features at beginning of acquisition control
        tmp_feature_name = "BEGIN";
        RCLCPP_DEBUG(logger_, "Evaluating 'AcquisitionControl.%s' for stream %i.",
                     tmp_feature_name.c_str(), i);
        is_parameter_set = getAcquisitionControlParameterList(tmp_feature_name,
                                                              tmp_param_values);
        if (is_parameter_set)
        {
            filterParameterListNames(tmp_param_values);
            setFeatureValuesFromParameterList(tmp_param_values, i);
        }

        //--- for specific parameters, proceed as follows:
        //---   1. Check and read parameter from launch parameters
        //---   2. If provided in launch parameter, set value on device
        //---   3. Get the set value from the device and store it in corresponding struct
        //---   4. If specific parameter was given in the launch parameters, check against the
        //---       actual value which has been read from the camera and issue a warning message
        //---       if applicable
        //---
        //---   For bounded feature values (e.g. frame rate), use function that will also get
        //---   the bounds and truncate the value accordingly

        //--- Acquisition Mode
        tmp_feature_name = "AcquisitionMode";
        RCLCPP_DEBUG(logger_, "Evaluating 'AcquisitionControl.%s' for stream %i.",
                     tmp_feature_name.c_str(), i);
        is_parameter_set = getAcquisitionControlParameter(tmp_feature_name, tmp_param_value);
        if (is_parameter_set)
            setFeatureValueFromParameter<std::string>(tmp_feature_name, tmp_param_value, i);
        getFeatureValue<std::string>(tmp_feature_name, acq_ctrl.acquisition_mode);
        if (is_parameter_set &&
            !isParameterValueEqualTo<std::string>(tmp_param_value, acq_ctrl.acquisition_mode, i))
            config_warn_msgs_.push_back("Stream " + std::to_string(i) + ": " +
                                        "'" + tmp_feature_name + "' is not as specified.");

        //--- Acquisition Frame Count
        //--- get and set only if acquisition mode is set to multi frame
        if (arv_acquisition_mode_from_string(acq_ctrl.acquisition_mode.c_str()) ==
            ARV_ACQUISITION_MODE_MULTI_FRAME)
        {
            tmp_feature_name = "AcquisitionFrameCount";
            RCLCPP_DEBUG(logger_, "Evaluating 'AcquisitionControl.%s' for stream %i.",
                         tmp_feature_name.c_str(), i);
            is_parameter_set = getAcquisitionControlParameter(tmp_feature_name, tmp_param_value);
            if (is_parameter_set)
                setFeatureValueFromParameter<int64_t>(tmp_feature_name, tmp_param_value, i);
        }
        getFeatureValue<int>(tmp_feature_name, acq_ctrl.frame_count);
        if (is_parameter_set &&
            !isParameterValueEqualTo<int64_t>(tmp_param_value, acq_ctrl.frame_count, i))
            config_warn_msgs_.push_back("Stream " + std::to_string(i) + ": " +
                                        "'" + tmp_feature_name + "' is not as specified.");

        //--- Exposure Mode
        tmp_feature_name = "ExposureMode";
        RCLCPP_DEBUG(logger_, "Evaluating 'AcquisitionControl.%s' for stream %i.",
                     tmp_feature_name.c_str(), i);
        is_parameter_set = getAcquisitionControlParameter(tmp_feature_name, tmp_param_value);
        if (is_parameter_set)
            setFeatureValueFromParameter<std::string>(tmp_feature_name, tmp_param_value, i);
        getFeatureValue<std::string>(tmp_feature_name, acq_ctrl.exposure_mode);
        if (is_parameter_set &&
            !isParameterValueEqualTo<std::string>(tmp_param_value, acq_ctrl.exposure_mode, i))
            config_warn_msgs_.push_back("Stream " + std::to_string(i) + ": " +
                                        "'" + tmp_feature_name + "' is not as specified.");

        //--- Exposure Auto
        tmp_feature_name = "ExposureAuto";
        RCLCPP_DEBUG(logger_, "Evaluating 'AcquisitionControl.%s' for stream %i.",
                     tmp_feature_name.c_str(), i);
        is_parameter_set = getAcquisitionControlParameter(tmp_feature_name, tmp_param_value);
        if (is_parameter_set)
            setFeatureValueFromParameter<std::string>(tmp_feature_name, tmp_param_value, i);
        getFeatureValue<std::string>(tmp_feature_name, acq_ctrl.exposure_auto);
        if (is_parameter_set &&
            !isParameterValueEqualTo<std::string>(tmp_param_value, acq_ctrl.exposure_auto, i))
            config_warn_msgs_.push_back("Stream " + std::to_string(i) + ": " +
                                        "'" + tmp_feature_name + "' is not as specified.");

        //--- Exposure Time
        //--- set only if auto exposure is off and exposure mode is set to timed
        tmp_feature_name = "ExposureTime";
        RCLCPP_DEBUG(logger_, "Evaluating 'AcquisitionControl.%s' for stream %i.",
                     tmp_feature_name.c_str(), i);
        is_parameter_set = getAcquisitionControlParameter(tmp_feature_name, tmp_param_value);
        if (arv_auto_from_string(acq_ctrl.exposure_auto.c_str()) ==
              ARV_AUTO_OFF &&
            arv_exposure_mode_from_string(acq_ctrl.exposure_mode.c_str()) ==
              ARV_EXPOSURE_MODE_TIMED &&
            is_parameter_set)
            setFeatureValueFromParameter<double>(tmp_feature_name, tmp_param_value, i);
        getFeatureValue<double>(tmp_feature_name, acq_ctrl.exposure_time);
        if (is_parameter_set &&
            !isParameterValueEqualTo<double>(tmp_param_value, acq_ctrl.exposure_time, i))
            config_warn_msgs_.push_back("Stream " + std::to_string(i) + ": " +
                                        "'" + tmp_feature_name + "' is not as specified.");

        //--- Acquisition Frame Rate
        //--- only set if it is enabled
        tmp_feature_name = "AcquisitionFrameRateEnable";
        RCLCPP_DEBUG(logger_, "Evaluating 'AcquisitionControl.%s' for stream %i.",
                     tmp_feature_name.c_str(), i);
        is_parameter_set = getAcquisitionControlParameter(tmp_feature_name, tmp_param_value);
        if (is_parameter_set)
            setFeatureValueFromParameter<bool>(tmp_feature_name, tmp_param_value, i);
        getFeatureValue<bool>(tmp_feature_name, acq_ctrl.is_frame_rate_enable);
        if (is_parameter_set &&
            !isParameterValueEqualTo<bool>(tmp_param_value, acq_ctrl.is_frame_rate_enable, i))
            config_warn_msgs_.push_back("Stream " + std::to_string(i) + ": " +
                                        "'" + tmp_feature_name + "' is not as specified.");

        tmp_feature_name = "AcquisitionFrameRate";
        RCLCPP_DEBUG(logger_, "Evaluating 'AcquisitionControl.%s' for stream %i.",
                     tmp_feature_name.c_str(), i);
        is_parameter_set = getAcquisitionControlParameter(tmp_feature_name, tmp_param_value);
        if (is_parameter_set)
        {
            if (acq_ctrl.is_frame_rate_enable)
            {
                tmp_min_dbl = 0;
                tmp_max_dbl = DBL_MAX;
                setBoundedFeatureValueFromParameter<double>(tmp_feature_name, tmp_param_value,
                                                            &tmp_min_dbl, &tmp_max_dbl, i);
                acq_ctrl.frame_rate_min = tmp_min_dbl;
                acq_ctrl.frame_rate_max = tmp_max_dbl;
            }
            else
            {
                config_warn_msgs_.push_back("Stream " + std::to_string(i) + ": " +
                                            "Could not set frame rate. " +
                                            "AcquisitionFrameRateEnable:" +
                                            ((acq_ctrl.is_frame_rate_enable)
                                               ? "true"
                                               : "false"));
            }
        }
        getFeatureValue<double>(tmp_feature_name, acq_ctrl.frame_rate);
        if (is_parameter_set &&
            acq_ctrl.is_frame_rate_enable &&
            !isParameterValueEqualTo<double>(tmp_param_value, acq_ctrl.frame_rate, i))
            config_warn_msgs_.push_back("Stream " + std::to_string(i) + ": " +
                                        "'" + tmp_feature_name + "' is not as specified.");

        //--- get and set given custom features at end of acquisition control
        tmp_feature_name = "END";
        RCLCPP_DEBUG(logger_, "Evaluating 'AcquisitionControl.%s' for stream %i.",
                     tmp_feature_name.c_str(), i);
        is_parameter_set = getAcquisitionControlParameterList(tmp_feature_name,
                                                              tmp_param_values);
        if (is_parameter_set)
        {
            filterParameterListNames(tmp_param_values);
            setFeatureValuesFromParameterList(tmp_param_values, i);
        }
    }

    return true;
}

//==================================================================================================
[[nodiscard]] bool CameraDriver::getAnalogControlParameter(
  const std::string& param_name,
  rclcpp::ParameterValue& param_value) const
{
    return getNestedParameter("AnalogControl", param_name, param_value);
}

//==================================================================================================
[[nodiscard]] bool CameraDriver::getAnalogControlParameterList(
  const std::string& param_name,
  std::vector<std::pair<std::string, rclcpp::ParameterValue>>& param_values) const
{
    return getNestedParameterList("AnalogControl", param_name, param_values);
}

//==================================================================================================
[[nodiscard]] bool CameraDriver::setAnalogControlSettings()
{
    GuardedGError err;

    for (uint i = 0; i < streams_.size(); ++i)
    {
        Stream& stream             = streams_[i];
        AnalogControl& analog_ctrl = stream.analog_control;

        std::string tmp_feature_name;
        rclcpp::ParameterValue tmp_param_value;
        std::vector<std::pair<std::string, rclcpp::ParameterValue>> tmp_param_values;

        gdouble tmp_min_dbl, tmp_max_dbl, tmp_value_dbl;

        bool is_parameter_set;

        //--- set source, if applicable
        if (streams_.size() > 1)
        {
            // TODO(boitumeloruf): make more source selector more generic
            std::string src_selector_val = "Source" + std::to_string(i);
            setFeatureValue<std::string>("SourceSelector", src_selector_val);
        }

        //--- get and set given custom features at beginning of analog control
        tmp_feature_name = "BEGIN";
        RCLCPP_DEBUG(logger_, "Evaluating 'AnalogControl.%s' for stream %i.",
                     tmp_feature_name.c_str(), i);
        is_parameter_set = getAnalogControlParameterList(tmp_feature_name,
                                                         tmp_param_values);
        if (is_parameter_set)
        {
            filterParameterListNames(tmp_param_values);
            setFeatureValuesFromParameterList(tmp_param_values, i);
        }

        //--- for specific auto parameters, proceed as follows:
        //---   1. Check and read parameter from launch parameters
        //---   2. If provided in launch parameter, set value on device
        //---   3. Get the set value from the device and store it in corresponding struct
        //---   4. If specific parameter was given in the launch parameters, check against the
        //---       actual value which has been read from the camera and issue a warning message
        //---       if applicable
        //---
        //--- if corresponding auto modes are deactivated, proceed as follows to set specific
        //--- values:
        //---   1. get nested parameter list for corresponding feature
        //---   2. parameter list will hold key-value pairs consisting of selector value and
        //---       actual value
        //---   3. thus, loop through nested parameter list
        //---       a) use key to set value of selector
        //---       b) set value to actual parameter by considering bounds
        //---       c) get accepted value, store into struct, compare against given parameter,
        //---           and issue warning message, if applicable

        //--- Gain
        tmp_feature_name = "GainAuto";
        RCLCPP_DEBUG(logger_, "Evaluating 'AnalogControl.%s' for stream %i.",
                     tmp_feature_name.c_str(), i);
        is_parameter_set = getAnalogControlParameter(tmp_feature_name, tmp_param_value);
        if (is_parameter_set)
            setFeatureValueFromParameter<std::string>(tmp_feature_name, tmp_param_value, i);
        getFeatureValue<std::string>(tmp_feature_name, analog_ctrl.gain_auto);
        if (is_parameter_set &&
            !isParameterValueEqualTo<std::string>(tmp_param_value, analog_ctrl.gain_auto, i))
            config_warn_msgs_.push_back("Stream " + std::to_string(i) + ": " +
                                        "'" + tmp_feature_name + "' is not as specified.");

        if (arv_auto_from_string(analog_ctrl.gain_auto.c_str()) == ARV_AUTO_OFF)
        {
            tmp_feature_name = "Gain";
            RCLCPP_DEBUG(logger_, "Evaluating 'AnalogControl.%s' for stream %i.",
                         tmp_feature_name.c_str(), i);
            is_parameter_set = getAnalogControlParameterList(tmp_feature_name,
                                                             tmp_param_values);
            if (is_parameter_set)
            {
                for (auto itr = tmp_param_values.begin(); itr != tmp_param_values.end(); ++itr)
                {
                    //--- set selector
                    setFeatureValue<std::string>("GainSelector", itr->first);

                    //--- set bounded feature
                    setBoundedFeatureValueFromParameter<double>(tmp_feature_name.c_str(),
                                                                itr->second,
                                                                &tmp_min_dbl, &tmp_max_dbl, i);
                    //--- get set value
                    getFeatureValue<double>(tmp_feature_name, tmp_value_dbl);
                    if (!isParameterValueEqualTo<double>(itr->second, tmp_value_dbl, i))
                        config_warn_msgs_.push_back("Stream " + std::to_string(i) + ": " +
                                                    "'" + tmp_feature_name + "." + itr->first +
                                                    "' is not as specified.");

                    //--- store into struct
                    analog_ctrl.gain.push_back(std::make_tuple(itr->first, tmp_value_dbl,
                                                               tmp_min_dbl, tmp_max_dbl));
                }
            }
        }

        //--- Black Level
        tmp_feature_name = "BlackLevelAuto";
        RCLCPP_DEBUG(logger_, "Evaluating 'AnalogControl.%s' for stream %i.",
                     tmp_feature_name.c_str(), i);
        is_parameter_set = getAnalogControlParameter(tmp_feature_name, tmp_param_value);
        if (is_parameter_set)
            setFeatureValueFromParameter<std::string>(tmp_feature_name, tmp_param_value, i);
        getFeatureValue<std::string>(tmp_feature_name, analog_ctrl.black_level_auto);
        if (is_parameter_set &&
            !isParameterValueEqualTo<std::string>(tmp_param_value, analog_ctrl.black_level_auto, i))
            config_warn_msgs_.push_back("Stream " + std::to_string(i) + ": " +
                                        "'" + tmp_feature_name + "' is not as specified.");

        if (arv_auto_from_string(analog_ctrl.black_level_auto.c_str()) == ARV_AUTO_OFF)
        {
            tmp_feature_name = "BlackLevel";
            RCLCPP_DEBUG(logger_, "Evaluating 'AnalogControl.%s' for stream %i.",
                         tmp_feature_name.c_str(), i);
            is_parameter_set = getAnalogControlParameterList(tmp_feature_name,
                                                             tmp_param_values);
            if (is_parameter_set)
            {
                for (auto itr = tmp_param_values.begin(); itr != tmp_param_values.end(); ++itr)
                {
                    //--- set selector
                    setFeatureValue<std::string>("BlackLevelSelector", itr->first);

                    //--- set bounded feature
                    setBoundedFeatureValueFromParameter<double>(tmp_feature_name.c_str(),
                                                                itr->second,
                                                                &tmp_min_dbl, &tmp_max_dbl, i);

                    //--- get set value
                    getFeatureValue<double>(tmp_feature_name, tmp_value_dbl);
                    if (!isParameterValueEqualTo<double>(itr->second, tmp_value_dbl, i))
                        config_warn_msgs_.push_back("Stream " + std::to_string(i) + ": " +
                                                    "'" + tmp_feature_name + "." + itr->first +
                                                    "' is not as specified.");

                    //--- store into struct
                    analog_ctrl.black_level.push_back(std::make_tuple(itr->first, tmp_value_dbl,
                                                                      tmp_min_dbl, tmp_max_dbl));
                }
            }
        }

        //--- White Balance
        tmp_feature_name = "BalanceWhiteAuto";
        RCLCPP_DEBUG(logger_, "Evaluating 'AnalogControl.%s' for stream %i.",
                     tmp_feature_name.c_str(), i);
        is_parameter_set = getAnalogControlParameter(tmp_feature_name, tmp_param_value);
        if (is_parameter_set)
            setFeatureValueFromParameter<std::string>(tmp_feature_name, tmp_param_value, i);
        getFeatureValue<std::string>(tmp_feature_name, analog_ctrl.balance_white_auto);
        if (is_parameter_set &&
            !isParameterValueEqualTo<std::string>(tmp_param_value,
                                                  analog_ctrl.balance_white_auto, i))
            config_warn_msgs_.push_back("Stream " + std::to_string(i) + ": " +
                                        "'" + tmp_feature_name + "' is not as specified.");

        if (arv_auto_from_string(analog_ctrl.balance_white_auto.c_str()) == ARV_AUTO_OFF)
        {
            tmp_feature_name = "BalanceRatio";
            RCLCPP_DEBUG(logger_, "Evaluating 'AnalogControl.%s' for stream %i.",
                         tmp_feature_name.c_str(), i);
            is_parameter_set = getAnalogControlParameterList(tmp_feature_name,
                                                             tmp_param_values);
            if (is_parameter_set)
            {
                for (auto itr = tmp_param_values.begin(); itr != tmp_param_values.end(); ++itr)
                {
                    //--- set selector
                    setFeatureValue<std::string>("BalanceRatioSelector", itr->first);

                    //--- set bounded feature
                    setBoundedFeatureValueFromParameter<double>(tmp_feature_name.c_str(),
                                                                itr->second,
                                                                &tmp_min_dbl, &tmp_max_dbl, i);

                    //--- get set value
                    getFeatureValue<double>(tmp_feature_name, tmp_value_dbl);
                    if (!isParameterValueEqualTo<double>(itr->second, tmp_value_dbl, i))
                        config_warn_msgs_.push_back("Stream " + std::to_string(i) + ": " +
                                                    "'" + tmp_feature_name + "." + itr->first +
                                                    "' is not as specified.");

                    //--- store into struct
                    analog_ctrl.balance_ratio.push_back(std::make_tuple(itr->first, tmp_value_dbl,
                                                                        tmp_min_dbl, tmp_max_dbl));
                }
            }
        }

        //--- get and set given custom features at end of analog control
        tmp_feature_name = "END";
        RCLCPP_DEBUG(logger_, "Evaluating 'AnalogControl.%s' for stream %i.",
                     tmp_feature_name.c_str(), i);
        is_parameter_set = getAnalogControlParameterList(tmp_feature_name,
                                                         tmp_param_values);
        if (is_parameter_set)
        {
            filterParameterListNames(tmp_param_values);
            setFeatureValuesFromParameterList(tmp_param_values, i);
        }
    }

    return true;
}

//==================================================================================================
bool CameraDriver::initializeServices()
{
    //--- initialize service to calculate white balance
    p_white_balance_srv_ =
      this->create_service<camera_aravis2_msgs::srv::CalculateWhiteBalance>(
        "~/calculate_white_balance_once",
        std::bind(&CameraDriver::onCalculateWhiteBalanceOnceTriggered, this,
                  std::placeholders::_1, std::placeholders::_2));
    return true;
}

#ifdef WITH_MATCHED_EVENTS
//==================================================================================================
void CameraDriver::handleMessageSubscriptionChange(rclcpp::MatchedInfo& iEventInfo)
{
    GuardedGError err;

    //--- evaluate whether to start or stop acquisition only if device is available and if the
    //--- node is initialized.
    if (p_device_ && this->is_initialized_)
    {
        //--- if the new subscriber count of the matched event is greater than 0 and if there were
        //--- no subscriber so far, start acquisition
        if (iEventInfo.current_count > 0 && current_num_subscribers_ == 0)
        {
            RCLCPP_INFO(logger_, "|-> Acquisition start.");

            arv_device_execute_command(p_device_, "AcquisitionStart", err.ref());
            CHECK_GERROR_MSG(err, logger_, "In executing 'AcquisitionStart'.");
        }
        //--- if the new subscriber count of the matched event is equal to 0 and if there were
        //--- subscribers until now, stop acquisition
        else if (iEventInfo.current_count == 0 && current_num_subscribers_ > 0)
        {
            RCLCPP_INFO(logger_, "->| Acquisition stop.");

            arv_device_execute_command(p_device_, "AcquisitionStop", err.ref());
            CHECK_GERROR_MSG(err, logger_, "In executing 'AcquisitionStop'.");
        }
    }
    else
    {
        RCLCPP_WARN(logger_, "Subscription change detected but no action taken. "
                             "Reason: p_device_ is NULL or node is not initialized.");
        return;
    }

    //--- get the maximum number of subscribers over all streams and assign to the member variable
    current_num_subscribers_ = 0;
    for (uint i = 0; i < streams_.size(); ++i)
    {
        current_num_subscribers_ =
          std::max(static_cast<int>(streams_[i].camera_pub.getNumSubscribers()),
                   current_num_subscribers_);
    }
}
#endif

//==================================================================================================
void CameraDriver::setupDynamicParameters()
{
    GuardedGError err;

    //--- get file url
    auto dyn_param_yaml_url_ = get_parameter("dynamic_parameters_yaml_url").as_string();

    //--- read diagnostic features from yaml file and initialize publishing thread
    if (!dyn_param_yaml_url_.empty())
    {
        YAML::Node dyn_params;

        try
        {
            dyn_params = YAML::LoadFile(dyn_param_yaml_url_);
        }
        catch (const YAML::BadFile& e)
        {
            config_warn_msgs_.push_back("YAML file cannot be loaded: " + std::string(e.what()));
            config_warn_msgs_.push_back("Dynamic parameters will not be available.");
            return;
        }
        catch (const YAML::ParserException& e)
        {
            config_warn_msgs_.push_back("YAML file is malformed: " + std::string(e.what()));
            config_warn_msgs_.push_back("Dynamic parameters will not be available.");
            return;
        }

        //--- if parameters are available in list, declare parameter accordingly.
        if (dyn_params.size() > 0)
        {
            int feature_idx = 1;

            //--- loop through feature list and get specified information
            for (auto feature_dict : dyn_params)
            {
                std::string feature_name = feature_dict["FeatureName"].IsDefined()
                                             ? feature_dict["FeatureName"].as<std::string>()
                                             : "";
                std::string feature_type = feature_dict["Type"].IsDefined()
                                             ? feature_dict["Type"].as<std::string>()
                                             : "";

                if (feature_name.empty() || feature_type.empty())
                {
                    config_warn_msgs_.push_back(
                      "Dynamic parameter at index " +
                      std::to_string(feature_idx) +
                      " does not have a field 'FeatureName' or 'Type'. "
                      "Parameter will be skipped.");
                }
                else
                {
                    if (!arv_device_is_feature_available(p_device_, feature_name.c_str(),
                                                         err.ref()))
                    {
                        config_warn_msgs_.push_back(
                          "Dynamic parameter with 'FeatureName' '" + feature_name +
                          " cannot be declared as feature is not available on "
                          "device.");
                    }
                    else
                    {
                        //--- if feature is available on device, declare it a parameter that can
                        //--- dynamically be changed

                        //--- convert type string to lower case
                        std::transform(feature_type.begin(), feature_type.end(),
                                       feature_type.begin(), [](unsigned char c)
                                       { return std::tolower(c); });

                        //--- floating point type
                        if (feature_type == "float" || feature_type == "double")
                        {
                            //--- set description
                            auto param_desc        = rcl_interfaces::msg::ParameterDescriptor{};
                            param_desc.description = feature_dict["Description"].as<std::string>();

                            //--- get bounds from device and from yaml file,
                            //--- and choose the intersection of both
                            rcl_interfaces::msg::FloatingPointRange fpRange;
                            fpRange.from_value = DBL_MIN;
                            fpRange.to_value   = DBL_MAX;
                            arv_device_get_float_feature_bounds(p_device_, feature_name.c_str(),
                                                                &fpRange.from_value,
                                                                &fpRange.to_value,
                                                                err.ref());

                            if (feature_dict["Min"].IsDefined())
                                fpRange.from_value = std::max(fpRange.from_value,
                                                              feature_dict["Min"].as<double>());
                            if (feature_dict["Max"].IsDefined())
                                fpRange.to_value = std::min(fpRange.to_value,
                                                            feature_dict["Max"].as<double>());

                            fpRange.step = 0;

                            if (fpRange.from_value != DBL_MIN && fpRange.to_value != DBL_MAX)
                                param_desc.floating_point_range = {fpRange};

                            //--- get current value as default and restrict it to 3 chars after
                            //--- decimal point
                            float defVal = 0;
                            getFeatureValue<float>(feature_name, defVal);
                            int defVal_int = static_cast<int>(defVal * 1000.f);
                            defVal         = static_cast<float>(defVal_int) / 1000;

                            //--- declare parameter
                            declare_parameter<float>(feature_name, defVal, param_desc);
                        }
                        //--- integer type
                        else if (feature_type == "int")
                        {
                            //--- set description
                            auto param_desc        = rcl_interfaces::msg::ParameterDescriptor{};
                            param_desc.description = feature_dict["Description"].as<std::string>();

                            //--- get bounds from device and from yaml file,
                            //--- and choose the intersection of both
                            rcl_interfaces::msg::IntegerRange intRange;
                            intRange.from_value = INT_MIN;
                            intRange.to_value   = INT_MAX;
                            arv_device_get_integer_feature_bounds(p_device_, feature_name.c_str(),
                                                                  &intRange.from_value,
                                                                  &intRange.to_value,
                                                                  err.ref());

                            if (feature_dict["Min"].IsDefined())
                                intRange.from_value = std::max(
                                  static_cast<int>(intRange.from_value),
                                  feature_dict["Min"].as<int>());
                            if (feature_dict["Max"].IsDefined())
                                intRange.to_value = std::min(
                                  static_cast<int>(intRange.to_value),
                                  feature_dict["Max"].as<int>());

                            intRange.step = 0;

                            if (intRange.from_value != INT_MIN && intRange.to_value != INT_MAX)
                                param_desc.integer_range = {intRange};

                            //--- get current value as default
                            int defVal = 0;
                            getFeatureValue<int>(feature_name, defVal);

                            //--- declare parameter
                            declare_parameter<int>(feature_name, defVal, param_desc);
                        }
                        //--- bool type
                        else if (feature_type == "bool")
                        {
                            //--- set description
                            auto param_desc        = rcl_interfaces::msg::ParameterDescriptor{};
                            param_desc.description = feature_dict["Description"].as<std::string>();

                            //--- get current value as default
                            bool defVal = false;
                            getFeatureValue<bool>(feature_name, defVal);

                            //--- declare parameter
                            declare_parameter<bool>(feature_name, defVal, param_desc);
                        }
                        //--- string or other type
                        else
                        {
                            //--- set description
                            auto param_desc        = rcl_interfaces::msg::ParameterDescriptor{};
                            param_desc.description = feature_dict["Description"].as<std::string>();

                            //--- get current value as default
                            std::string defVal = "";
                            getFeatureValue<std::string>(feature_name, defVal);

                            //--- declare parameter
                            declare_parameter<std::string>(feature_name, defVal, param_desc);
                        }

                        dynamic_parameters_names_.push_back(feature_name);
                    }

                    CHECK_GERROR_MSG(err, logger_,
                                     "In setting dynamic parameter '" + feature_name + "'.");
                }

                feature_idx++;
            }
        }
        else
        {
            config_warn_msgs_.push_back(
              "YAML file for dynamic parameters with no features specified.");
            config_warn_msgs_.push_back("Dynamic parameters will not be available.");
            return;
        }
    }
}

//==================================================================================================
rcl_interfaces::msg::SetParametersResult CameraDriver::handleDynamicParameterChange(
  const std::vector<rclcpp::Parameter>& iParameters)
{
    RCLCPP_DEBUG(logger_, "%s", __PRETTY_FUNCTION__);

    // Return value that holds information on success.
    rcl_interfaces::msg::SetParametersResult retVal;
    retVal.successful = true;

    //-- loop through all parameters and set information accordingly
    for (rclcpp::Parameter param : iParameters)
    {
        // parameter name (i.e. feature name)
        std::string param_name = param.get_name();

        //--- verbose
        if (param_name == "verbose")
        {
            is_verbose_enable_ = param.as_bool();
        }
        //--- dynamic camera parameters, check if parameter name is in list of specified dynamic
        //--- parameters
        else if (std::find(dynamic_parameters_names_.begin(), dynamic_parameters_names_.end(),
                           param_name) != dynamic_parameters_names_.end())
        {
            switch (param.get_type())
            {
            case rclcpp::PARAMETER_STRING:
                retVal.successful &=
                  setFeatureValue<std::string>(param_name, param.get_value<std::string>());
                break;

            case rclcpp::PARAMETER_BOOL:
                retVal.successful &=
                  setFeatureValue<bool>(param_name, param.get_value<bool>());
                break;

            case rclcpp::PARAMETER_INTEGER:
                retVal.successful &=
                  setFeatureValue<int>(param_name, param.get_value<int>());
                break;

            case rclcpp::PARAMETER_DOUBLE:
                retVal.successful &=
                  setFeatureValue<double>(param_name, param.get_value<double>());
                break;

            default:
                RCLCPP_WARN(logger_, "Dynamic parameter is of invalid type '%s'",
                            param_name.c_str());
                retVal.successful &= false;
                break;
            }
        }
    }

    return retVal;
}

//==================================================================================================
void CameraDriver::setupCameraDiagnosticPublisher()
{
    //--- get diagnostic settings
    auto diagnostic_yaml_url_     = get_parameter("diagnostic_yaml_url").as_string();
    auto diagnostic_publish_rate_ = get_parameter("diagnostic_publishing_rate").as_double();

    //--- read diagnostic features from yaml file and initialize publishing thread
    if (!diagnostic_yaml_url_.empty() && diagnostic_publish_rate_ > 0.0)
    {
        try
        {
            diagnostic_features_ = YAML::LoadFile(diagnostic_yaml_url_);
        }
        catch (const YAML::BadFile& e)
        {
            config_warn_msgs_.push_back("YAML file cannot be loaded: " + std::string(e.what()));
            config_warn_msgs_.push_back("Camera diagnostics will not be published.");
            return;
        }
        catch (const YAML::ParserException& e)
        {
            config_warn_msgs_.push_back("YAML file is malformed: " + std::string(e.what()));
            config_warn_msgs_.push_back("Camera diagnostics will not be published.");
            return;
        }

        //--- if diagnostic features are available, create publisher and initialize thread
        if (diagnostic_features_.size() > 0)
        {
            p_diagnostic_pub_ =
              this->create_publisher<camera_aravis2_msgs::msg::CameraDiagnostics>(
                "~/diagnostics", 1);

            is_diagnostics_published_ = true;
            diagnostic_thread_ =
              std::thread(&CameraDriver::publishCameraDiagnosticsLoop, this,
                          diagnostic_publish_rate_);
        }
        else
        {
            config_warn_msgs_.push_back("No diagnostic features specified.");
            config_warn_msgs_.push_back("Camera diagnostics will not be published.");
            return;
        }
    }
    else if (!diagnostic_yaml_url_.empty() && diagnostic_publish_rate_ <= 0.0)
    {
        config_warn_msgs_.push_back("Diagnostic_publish_rate is <= 0.0");
        config_warn_msgs_.push_back("Camera diagnostics will not be published.");
        return;
    }
}

//==================================================================================================
void CameraDriver::publishCameraDiagnosticsLoop(double rate) const
{
    camera_aravis2_msgs::msg::CameraDiagnostics cam_diagnostic_msg;
    cam_diagnostic_msg.header.frame_id = this->get_name();

    // function to get feature value at given name and of given type as string
    auto getFeatureValueAsStrFn = [&](const std::string& name, const std::string& type)
      -> std::string
    {
        std::string value_str = "";

        if (type == "float")
        {
            float float_val;
            getFeatureValue<float>(name, float_val);
            value_str = std::to_string(float_val);
        }
        else if (type == "int")
        {
            int int_val;
            getFeatureValue<int>(name, int_val);
            value_str = std::to_string(int_val);
        }
        else if (type == "bool")
        {
            bool bool_val;
            getFeatureValue<bool>(name, bool_val);
            value_str = (bool_val) ? "true" : "false";
        }
        else
        {
            getFeatureValue<std::string>(name, value_str);
        }

        return value_str;
    };

    // function to set feature value at given name and of given type from string
    auto setFeatureValueFromStrFn = [&](const std::string& name, const std::string& type,
                                        const std::string& value_str)
    {
        if (type == "float")
            setFeatureValue<float>(name, std::stod(value_str));
        else if (type == "int")
            setFeatureValue<int>(name, std::stoi(value_str));
        else if (type == "bool")
            setFeatureValue<bool>(name,
                                  (value_str == "true" ||
                                   value_str == "True" ||
                                   value_str == "TRUE"));
        else
            setFeatureValue<std::string>(name, value_str);
    };

    // function to check if feature is available
    auto isFeatureAvailableFn = [&](const std::string& name) -> bool
    {
        bool is_successful = true;
        GuardedGError err;
        bool is_feature_available =
          arv_device_is_feature_available(p_device_, name.c_str(), err.ref());
        CHECK_SUCCESS_GERROR(err, logger_, is_successful)

        return (is_feature_available && is_successful);
    };

    while (rclcpp::ok() && is_diagnostics_published_)
    {
        cam_diagnostic_msg.header.stamp = this->now();

        cam_diagnostic_msg.data.clear();

        int feature_idx = 1;
        for (auto feature_dict : diagnostic_features_)
        {
            std::string feature_name = feature_dict["FeatureName"].IsDefined()
                                         ? feature_dict["FeatureName"].as<std::string>()
                                         : "";
            std::string feature_type = feature_dict["Type"].IsDefined()
                                         ? feature_dict["Type"].as<std::string>()
                                         : "";

            if (feature_name.empty() || feature_type.empty())
            {
                RCLCPP_WARN_ONCE(logger_,
                                 "Diagnostic feature at index %i does not have a field "
                                 "'FeatureName' or 'Type'.",
                                 feature_idx);
                RCLCPP_WARN_ONCE(logger_, "Diagnostic feature will be skipped.");
            }
            else
            {
                // convert type string to lower case
                std::transform(feature_type.begin(), feature_type.end(), feature_type.begin(),
                               [](unsigned char c)
                               { return std::tolower(c); });

                // if feature name is found in implemented_feature list and if enabled
                if (isFeatureAvailableFn(feature_name))
                {
                    diagnostic_msgs::msg::KeyValue kv_pair;

                    // if 'selectors' which correspond to the feature_name are defined
                    if (feature_dict["Selectors"].IsDefined() &&
                        feature_dict["Selectors"].size() > 0)
                    {
                        int selectorIdx = 1;
                        for (auto selector_dict : feature_dict["Selectors"])
                        {
                            std::string selector_feature_name =
                              selector_dict["FeatureName"].IsDefined()
                                ? selector_dict["FeatureName"].as<std::string>()
                                : "";
                            std::string selector_type =
                              selector_dict["Type"].IsDefined()
                                ? selector_dict["Type"].as<std::string>()
                                : "";
                            std::string selector_value =
                              selector_dict["Value"].IsDefined()
                                ? selector_dict["Value"].as<std::string>()
                                : "";

                            if (selector_feature_name.empty() ||
                                selector_type.empty() ||
                                selector_value.empty())
                            {
                                RCLCPP_WARN_ONCE(logger_,
                                                 "Diagnostic feature selector at index %i of "
                                                 "feature at index %i does not have a "
                                                 "field 'FeatureName', 'Type' or 'Value'.",
                                                 selectorIdx, feature_idx);
                                RCLCPP_WARN_ONCE(logger_, "Diagnostic feature will be skipped.");
                            }
                            else
                            {
                                if (isFeatureAvailableFn(selector_feature_name))
                                {
                                    setFeatureValueFromStrFn(selector_feature_name, selector_type,
                                                             selector_value);

                                    kv_pair.key   = feature_name + "-" + selector_value;
                                    kv_pair.value = getFeatureValueAsStrFn(feature_name,
                                                                           feature_type);

                                    cam_diagnostic_msg.data.push_back(
                                      diagnostic_msgs::msg::KeyValue(kv_pair));
                                }
                                else
                                {
                                    RCLCPP_WARN_ONCE(logger_,
                                                     "Diagnostic feature selector with "
                                                     "name '%s' is not implemented.",
                                                     selector_feature_name.c_str());
                                }

                                selectorIdx++;
                            }
                        }
                    }
                    else
                    {
                        kv_pair.key   = feature_name;
                        kv_pair.value = getFeatureValueAsStrFn(feature_name, feature_type);

                        cam_diagnostic_msg.data.push_back(diagnostic_msgs::msg::KeyValue(kv_pair));
                    }
                }
                else
                {
                    RCLCPP_WARN_ONCE(logger_,
                                     "Diagnostic feature with name '%s' is not implemented.",
                                     feature_name.c_str());
                }
            }

            feature_idx++;
        }

        p_diagnostic_pub_->publish(cam_diagnostic_msg);

        rclcpp::sleep_for(std::chrono::nanoseconds(static_cast<int64_t>(std::pow(10, 9) / rate)));
    }
}

//==================================================================================================
void CameraDriver::spawnCameraStreams()
{
    GuardedGError err;

    // Number of opened streams
    int num_opened_streams = 0;

    for (uint i = 0; i < streams_.size(); i++)
    {
        Stream& stream = streams_[i];

        RCLCPP_INFO(logger_, "Spawning camera stream with ID %i (%s)", i,
                    stream.name.c_str());

        const int MAX_RETRIES = 60;
        int tryCount          = 1;
        while (is_spawning_ && tryCount <= MAX_RETRIES)
        {
            //--- select stream channel only if camera is a GigEVision device
            if (arv_camera_is_gv_device(p_camera_))
                arv_camera_gv_select_stream_channel(p_camera_, i, err.ref());

            stream.p_arv_stream = arv_camera_create_stream(p_camera_, nullptr, nullptr, err.ref());
            CHECK_GERROR_MSG(err, logger_, "In creating camera stream.");

            if (stream.p_arv_stream)
            {
                //--- Initialize buffers

                // stream payload size in bytes
                const auto STREAM_PAYLOAD_SIZE = arv_camera_get_payload(p_camera_, err.ref());
                CHECK_GERROR_MSG(err, logger_, "In getting payload size of stream.");

                // TODO: launch parameter for number of preallocated buffers
                stream.p_buffer_pool.reset(
                  new ImageBufferPool(logger_, stream.p_arv_stream,
                                      static_cast<guint>(STREAM_PAYLOAD_SIZE), 10));

                stream.is_buffer_processed = true;
                stream.buffer_processing_thread =
                  std::thread(&CameraDriver::processStreamBuffer, this, i);

                tuneArvStream(reinterpret_cast<ArvStream*>(stream.p_arv_stream));

                num_opened_streams++;
                break;
            }
            else
            {
                RCLCPP_WARN(logger_, "%s: Could not create image stream with ID %i (%s). "
                                     "Retrying (%i/%i) ...",
                            guid_.c_str(), i, stream.name.c_str(),
                            tryCount, MAX_RETRIES);
                rclcpp::sleep_for(std::chrono::seconds(1));
                tryCount++;
            }
        }

        //--- check if stream could be established
        if (!stream.p_arv_stream)
            RCLCPP_ERROR(logger_, "%s: Could not create image stream with ID %i (%s).",
                         guid_.c_str(), i, stream.name.c_str());
    }
    is_spawning_ = false;

    //--- if no streams are opened, shut down
    if (num_opened_streams == 0)
    {
        RCLCPP_FATAL(logger_, "Failed to open streams for camera %s.",
                     guid_.c_str());
        return;
    }

    //--- Connect signals with callbacks and activate emission of signals
    for (uint i = 0; i < streams_.size(); ++i)
    {
        const Stream& STREAM = streams_[i];

        if (!STREAM.p_arv_stream)
            continue;

        new_buffer_cb_data_ptrs.push_back(
          std::make_shared<std::pair<CameraDriver*, uint>>(
            std::make_pair(this, i)));

        g_signal_connect(STREAM.p_arv_stream, "new-buffer",
                         (GCallback)CameraDriver::handleNewBufferSignal,
                         new_buffer_cb_data_ptrs.back().get());

        arv_stream_set_emit_signals(STREAM.p_arv_stream, TRUE);
    }

    //--- print final output message
    std::string camera_guid_str = CameraAravisNodeBase::constructCameraGuidStr(p_camera_);
    RCLCPP_INFO(logger_, "Done initializing.");
    RCLCPP_INFO(logger_, "  Camera:        %s", camera_guid_str.c_str());
    if (arv_camera_is_gv_device(p_camera_) && CameraAravisNodeBase::isIpAddress(guid_))
        RCLCPP_INFO(logger_, "  IP:            %s", guid_.c_str());
    RCLCPP_INFO(logger_, "  Num. Streams:  (%i / %i)",
                num_opened_streams, static_cast<int>(streams_.size()));

#ifndef WITH_MATCHED_EVENTS
    //--- If matched events are not available, the number of subscribers are not dynamically
    //--- changed. Thus, set number of subscribers to 1 in order for the acquisition to be started
    //--- below.
    current_num_subscribers_ = 1;
#endif

    //--- When there are already subscribers to the image topic, start acquisition.
    if (current_num_subscribers_ > 0)
    {
        RCLCPP_INFO(logger_, "|-> Acquisition start at initialization.");

        arv_device_execute_command(p_device_, "AcquisitionStart", err.ref());
        CHECK_GERROR_MSG(err, logger_, "In executing 'AcquisitionStart'.");
    }

    this->is_initialized_ = true;
}

//==================================================================================================
void CameraDriver::processStreamBuffer(const uint stream_id)
{
    using namespace std::chrono_literals;

    Stream& stream = streams_[stream_id];

    RCLCPP_INFO(logger_, "Started processing thread for stream %i (%s)", stream_id,
                stream.name.c_str());

    while (stream.is_buffer_processed)
    {
        //--- pop buffer pointer from queue, blocking if no item is available
        std::pair<ArvBuffer*, sensor_msgs::msg::Image::SharedPtr> buffer_img_pair;
        stream.buffer_queue.pop(buffer_img_pair);

        //--- take ownership of pointers
        ArvBuffer* p_arv_buffer                      = std::get<0>(buffer_img_pair);
        sensor_msgs::msg::Image::SharedPtr p_img_msg = std::get<1>(buffer_img_pair);
        if (!p_arv_buffer || !p_img_msg)
            continue;

        //--- check that image roi corresponds to actual image in buffer
        if (adjustImageRoi(stream.image_roi, p_arv_buffer))
        {
            RCLCPP_WARN(logger_,
                        "Image region specified for stream %i (%s) doesn't match received. "
                        "Setting region to: x=%i y=%i width= %i height=%i.",
                        stream_id, stream.name.c_str(),
                        stream.image_roi.x, stream.image_roi.x,
                        stream.image_roi.width, stream.image_roi.height);
        }

        //--- set meta data of image message
        fillImageMsgMetadata(p_img_msg, p_arv_buffer, stream.sensor, stream.image_roi);

        //--- convert to ros format
        if (stream.cvt_pixel_format)
        {
            sensor_msgs::msg::Image::SharedPtr p_cvt_img_msg =
              stream.p_buffer_pool->getRecyclableImg();
            stream.cvt_pixel_format(p_img_msg, p_cvt_img_msg);
            p_img_msg = p_cvt_img_msg;
        }

        //--- fill camera_info message
        fillCameraInfoMsg(stream, p_img_msg);

        //--- publish
        stream.camera_pub.publish(p_img_msg, stream.p_cam_info_msg);

        //--- do post frame processing
        postFrameProcessingCallback(stream_id);
    }

    RCLCPP_INFO(logger_, "Finished processing thread for stream %i (%s)", stream_id,
                stream.name.c_str());
}

//==================================================================================================
bool CameraDriver::adjustImageRoi(ImageRoi& img_roi, ArvBuffer* p_buffer) const
{
    gint x, y, width, height;

    arv_buffer_get_image_region(p_buffer, &x, &y, &width, &height);

    if (x == img_roi.x && y == img_roi.y && width == img_roi.width && height == img_roi.height)
        return false;

    img_roi.x      = x;
    img_roi.y      = y;
    img_roi.width  = width;
    img_roi.height = height;

    return true;
}

//==================================================================================================
void CameraDriver::fillImageMsgMetadata(sensor_msgs::msg::Image::SharedPtr& p_img_msg,
                                        ArvBuffer* p_buffer,
                                        const Sensor& sensor,
                                        const ImageRoi& img_roi) const
{
    std::shared_ptr<GvTransportLayerControl> p_gv_tl_control =
      std::dynamic_pointer_cast<GvTransportLayerControl>(p_tl_control_);

    //--- fill header data

    //--- set ptp timestamp if available
    bool isBufferTimestampSet = false;
    if (arv_camera_is_gv_device(p_camera_))
    {
        if (p_gv_tl_control)
        {
            if (p_gv_tl_control->is_ptp_enable)
            {
                p_img_msg->header.stamp = rclcpp::Time(arv_buffer_get_timestamp(p_buffer));
                isBufferTimestampSet    = true;
            }
        }
        else
        {
            RCLCPP_WARN(logger_, "%s: Something went wrong when trying to cast pointer of type "
                                 "GenTransportLayerControl to GevTransportLayerControl. "
                                 "Using system timestamp.",
                        __PRETTY_FUNCTION__);
        }
    }

    if (!isBufferTimestampSet)
        p_img_msg->header.stamp = rclcpp::Time(arv_buffer_get_system_timestamp(p_buffer));
    p_img_msg->header.frame_id = sensor.frame_id;

    //--- fill image meta data
    p_img_msg->width    = img_roi.width;
    p_img_msg->height   = img_roi.height;
    p_img_msg->encoding = sensor.pixel_format;
    p_img_msg->step     = (img_roi.width * sensor.n_bits_pixel) / 8;
}

//==================================================================================================
void CameraDriver::fillCameraInfoMsg(Stream& stream,
                                     const sensor_msgs::msg::Image::SharedPtr& p_img_msg) const
{
    //--- reset pointer to camera_info message, if not already set
    if (!stream.p_cam_info_msg)
    {
        stream.p_cam_info_msg.reset(new sensor_msgs::msg::CameraInfo());

        //--- fill camera data only once
        (*stream.p_cam_info_msg) = stream.p_camera_info_manager->getCameraInfo();
    }

    //--- fill header data each time an image gets published
    stream.p_cam_info_msg->header = p_img_msg->header;

    //--- check if image size given in camera info corresponds to actual image size.
    if (stream.p_camera_info_manager->isCalibrated() &&
        (stream.p_cam_info_msg->width != p_img_msg->width ||
         stream.p_cam_info_msg->height != p_img_msg->height))
    {
        RCLCPP_WARN_ONCE(
          logger_,
          "The fields image_width and image_height (%ix%i) in the YAML specified by "
          "'camera_info_url' parameter seams to be inconsistent with the actual "
          "image size (%ix%i). Please set them there, because actual image size and specified "
          "image size can be different due to the region of interest (ROI) feature. In the YAML "
          "the image size should be the one on which the camera was calibrated. "
          "See CameraInfo.msg specification!",
          stream.p_cam_info_msg->width, stream.p_cam_info_msg->height,
          p_img_msg->width, p_img_msg->height);
        stream.p_cam_info_msg->width  = p_img_msg->width;
        stream.p_cam_info_msg->height = p_img_msg->height;
    }
}

//==================================================================================================
void CameraDriver::printCameraConfiguration() const
{
    rclcpp::ParameterValue tmp_param_value;
    std::vector<std::pair<std::string, rclcpp::ParameterValue>> tmp_param_values;

    std::string camera_guid_str = CameraAravisNodeBase::constructCameraGuidStr(p_camera_);

    RCLCPP_INFO(logger_, "======================================");
    RCLCPP_INFO(logger_, "Camera Configuration:");
    RCLCPP_INFO(logger_, "--------------------------------------");
    RCLCPP_INFO(logger_, "  GUID:                  %s", camera_guid_str.c_str());
    if (is_verbose_enable_)
    {
        RCLCPP_INFO(logger_, "  Type:                  %s",
                    arv_camera_is_gv_device(p_camera_)
                      ? "GigEVision"
                      : (arv_camera_is_uv_device(p_camera_)
                           ? "USB3Vision"
                           : "Other"));
        RCLCPP_INFO(logger_, "  Num. Streams:          %i",
                    static_cast<int>(streams_.size()));
    }

    std::shared_ptr<GvTransportLayerControl> p_gv_tl_control =
      std::dynamic_pointer_cast<GvTransportLayerControl>(p_tl_control_);
    if (arv_camera_is_gv_device(p_camera_) && p_gv_tl_control)
    {
        if (getTransportLayerControlParameter("GevSCPSPacketSize", tmp_param_value) ||
            is_verbose_enable_)
            RCLCPP_INFO(logger_, "  GEV Packet Size (B):   %li",
                        p_gv_tl_control->packet_size);

        if (getTransportLayerControlParameter("GevSCPD", tmp_param_value) ||
            is_verbose_enable_)
            RCLCPP_INFO(logger_, "  GEV Packet Delay:      %li",
                        p_gv_tl_control->inter_packet_delay);

        if (getTransportLayerControlParameter("PtpEnable", tmp_param_value) ||
            is_verbose_enable_)
        {
            RCLCPP_INFO(logger_, "  PTP Enabled:           %s",
                        (p_gv_tl_control->is_ptp_enable) ? "True" : "False");
            RCLCPP_INFO(logger_, "  PTP Status:            %s",
                        p_gv_tl_control->ptp_status.c_str());
        }
    }

    for (uint i = 0; i < streams_.size(); i++)
    {
        const Stream& STREAM               = streams_[i];
        const Sensor& SENSOR               = STREAM.sensor;
        const ImageRoi& ROI                = STREAM.image_roi;
        const AcquisitionControl& ACQ_CTRL = STREAM.acquisition_control;
        const AnalogControl& ANALOG_CTRL   = STREAM.analog_control;

        RCLCPP_INFO(logger_, "  - - - - - - - - - - - - - - - - - - ");
        RCLCPP_INFO(logger_, "  Stream %i:              %s", i, STREAM.name.c_str());

        if (is_verbose_enable_)
        {
            RCLCPP_INFO(logger_, "    Camera Info:         %s", STREAM.camera_info_url.c_str());
            RCLCPP_INFO(logger_, "    Topic:               %s",
                        STREAM.camera_pub.getTopic().c_str());
            RCLCPP_INFO(logger_, "    Frame ID:            %s", SENSOR.frame_id.c_str());
        }

        RCLCPP_INFO(logger_, "    Sensor Size:         %ix%i", SENSOR.width, SENSOR.height);

        RCLCPP_INFO(logger_, "    Pixel Format:        %s", SENSOR.pixel_format.c_str());

        if (is_verbose_enable_)
            RCLCPP_INFO(logger_, "    Bits/Pixel:          %i",
                        static_cast<int>(SENSOR.n_bits_pixel));

        if (getImageFormatControlParameter("ReverseX", tmp_param_value) ||
            getImageFormatControlParameter("ReverseY", tmp_param_value) ||
            is_verbose_enable_)
            RCLCPP_INFO(logger_, "    Reverse X,Y:         %s,%s",
                        (SENSOR.reverse_x) ? "True" : "False",
                        (SENSOR.reverse_y) ? "True" : "False");

        if (getImageFormatControlParameter("BinningHorizontal", tmp_param_value) ||
            getImageFormatControlParameter("BinningHorizontalMode", tmp_param_value) ||
            getImageFormatControlParameter("BinningVertical", tmp_param_value) ||
            getImageFormatControlParameter("BinningVerticalMode", tmp_param_value) ||
            is_verbose_enable_)
        {
            RCLCPP_INFO(logger_, "    Binning X,Y:         %i,%i",
                        SENSOR.binning_x, SENSOR.binning_y);
            RCLCPP_INFO(logger_, "    Binning Mode X,Y:    %s,%s",
                        SENSOR.binning_mode_x.c_str(), SENSOR.binning_mode_y.c_str());
        }

        if (getImageFormatControlParameter("OffsetX", tmp_param_value) ||
            getImageFormatControlParameter("OffsetY", tmp_param_value) ||
            is_verbose_enable_)
            RCLCPP_INFO(logger_, "    Image Offset X,Y:    %i,%i", ROI.x, ROI.y);

        RCLCPP_INFO(logger_, "    Image Size:          %ix%i", ROI.width, ROI.height);
        if (is_verbose_enable_)
            RCLCPP_INFO(logger_, "    Image Width Bound:   [%i,%i]",
                        ROI.width_min, ROI.width_max);
        if (is_verbose_enable_)
            RCLCPP_INFO(logger_, "    Image Height Bound:  [%i,%i]",
                        ROI.height_min, ROI.height_max);

        RCLCPP_INFO(logger_, "    Acquisition Mode:    %s", ACQ_CTRL.acquisition_mode.c_str());

        if (getAcquisitionControlParameter("AcquisitionFrameCount", tmp_param_value) ||
            arv_acquisition_mode_from_string(ACQ_CTRL.acquisition_mode.c_str()) ==
              ARV_ACQUISITION_MODE_MULTI_FRAME ||
            is_verbose_enable_)
            RCLCPP_INFO(logger_, "    Frame Count:         %i", ACQ_CTRL.frame_count);

        RCLCPP_INFO(logger_, "    Exposure Mode:       %s", ACQ_CTRL.exposure_mode.c_str());

        RCLCPP_INFO(logger_, "    Exposure Auto:       %s", ACQ_CTRL.exposure_auto.c_str());

        if ((arv_auto_from_string(ACQ_CTRL.exposure_auto.c_str()) ==
               ARV_AUTO_OFF &&
             arv_exposure_mode_from_string(ACQ_CTRL.exposure_mode.c_str()) ==
               ARV_EXPOSURE_MODE_TIMED) ||
            is_verbose_enable_)
            RCLCPP_INFO(logger_, "    Exposure Time (us):  %f", ACQ_CTRL.exposure_time);

        RCLCPP_INFO(logger_, "    Frame Rate Enable:   %s",
                    (ACQ_CTRL.is_frame_rate_enable) ? "True" : "False");

        RCLCPP_INFO(logger_, "    Frame Rate (Hz):     %f", ACQ_CTRL.frame_rate);

        if (is_verbose_enable_)
            RCLCPP_INFO(logger_, "    Frame Rate Bound:    [%f,%f]",
                        ACQ_CTRL.frame_rate_min, ACQ_CTRL.frame_rate_max);

        if (getAnalogControlParameter("GainAuto", tmp_param_value) ||
            is_verbose_enable_)
            RCLCPP_INFO(logger_, "    Gain Auto:           %s", ANALOG_CTRL.gain_auto.c_str());

        if (arv_auto_from_string(ANALOG_CTRL.gain_auto.c_str()) == ARV_AUTO_OFF &&
            (getAnalogControlParameterList("Gain", tmp_param_values) ||
             is_verbose_enable_))
        {
            for (auto itr = ANALOG_CTRL.gain.begin(); itr != ANALOG_CTRL.gain.end(); ++itr)
            {
                if (itr == ANALOG_CTRL.gain.begin())
                    RCLCPP_INFO(logger_, "    Gain:                %f (%s)", std::get<1>(*itr),
                                std::get<0>(*itr).c_str());
                else
                    RCLCPP_INFO(logger_, "                         %f (%s)", std::get<1>(*itr),
                                std::get<0>(*itr).c_str());
                if (is_verbose_enable_)
                    RCLCPP_INFO(logger_, "                         [%f,%f]", std::get<2>(*itr),
                                std::get<3>(*itr));
            }
        }

        if (getAnalogControlParameter("BlackLevelAuto", tmp_param_value) ||
            is_verbose_enable_)
            RCLCPP_INFO(logger_, "    Black Level Auto:    %s",
                        ANALOG_CTRL.black_level_auto.c_str());

        if (arv_auto_from_string(ANALOG_CTRL.black_level_auto.c_str()) == ARV_AUTO_OFF &&
            (getAnalogControlParameterList("BlackLevel", tmp_param_values) ||
             is_verbose_enable_))
        {
            for (auto itr = ANALOG_CTRL.black_level.begin(); itr != ANALOG_CTRL.black_level.end();
                 ++itr)
            {
                if (itr == ANALOG_CTRL.black_level.begin())
                    RCLCPP_INFO(logger_, "    Black Level:         %f (%s)", std::get<1>(*itr),
                                std::get<0>(*itr).c_str());
                else
                    RCLCPP_INFO(logger_, "                         %f (%s)", std::get<1>(*itr),
                                std::get<0>(*itr).c_str());
                if (is_verbose_enable_)
                    RCLCPP_INFO(logger_, "                         [%f,%f]", std::get<2>(*itr),
                                std::get<3>(*itr));
            }
        }

        if (getAnalogControlParameter("BalanceWhiteAuto", tmp_param_value) ||
            is_verbose_enable_)
            RCLCPP_INFO(logger_, "    Balance White Auto:  %s",
                        ANALOG_CTRL.balance_white_auto.c_str());

        if (arv_auto_from_string(ANALOG_CTRL.balance_white_auto.c_str()) == ARV_AUTO_OFF &&
            (getAnalogControlParameterList("BalanceRatio", tmp_param_values) ||
             is_verbose_enable_))
        {
            for (auto itr = ANALOG_CTRL.balance_ratio.begin();
                 itr != ANALOG_CTRL.balance_ratio.end(); ++itr)
            {
                if (itr == ANALOG_CTRL.balance_ratio.begin())
                    RCLCPP_INFO(logger_, "    Balance Ratio:       %f (%s)", std::get<1>(*itr),
                                std::get<0>(*itr).c_str());
                else
                    RCLCPP_INFO(logger_, "                         %f (%s)", std::get<1>(*itr),
                                std::get<0>(*itr).c_str());
                if (is_verbose_enable_)
                    RCLCPP_INFO(logger_, "                         [%f,%f]", std::get<2>(*itr),
                                std::get<3>(*itr));
            }
        }
    }

    RCLCPP_INFO(logger_, "======================================");

    for (std::string warn_msg : config_warn_msgs_)
    {
        RCLCPP_WARN(logger_, "%s", warn_msg.c_str());
    }
}

//==================================================================================================
void CameraDriver::printStreamStatistics() const
{
    for (uint i = 0; i < streams_.size(); i++)
    {
        const Stream& STREAM = streams_[i];

        if (!STREAM.p_arv_stream)
            continue;

        guint64 n_completed_buffers = 0;
        guint64 n_failures          = 0;
        guint64 n_underruns         = 0;
        arv_stream_get_statistics(STREAM.p_arv_stream,
                                  &n_completed_buffers, &n_failures, &n_underruns);

        RCLCPP_INFO(logger_, "Statistics for stream %i (%s):", i, STREAM.name.c_str());
        RCLCPP_INFO(logger_, "  Completed buffers = %li", (uint64_t)n_completed_buffers);
        RCLCPP_INFO(logger_, "  Failures          = %li", (uint64_t)n_failures);
        RCLCPP_INFO(logger_, "  Underruns         = %li", (uint64_t)n_underruns);

        if (arv_camera_is_gv_device(p_camera_))
        {
            guint64 n_resent;
            guint64 n_missing;

            arv_gv_stream_get_statistics(reinterpret_cast<ArvGvStream*>(STREAM.p_arv_stream),
                                         &n_resent, &n_missing);
            RCLCPP_INFO(logger_, "  Resent buffers    = %li", (uint64_t)n_resent);
            RCLCPP_INFO(logger_, "  Missing           = %li", (uint64_t)n_missing);
        }
    }
}

//==================================================================================================
void CameraDriver::onCalculateWhiteBalanceOnceTriggered(
  const std::shared_ptr<camera_aravis2_msgs::srv::CalculateWhiteBalance::Request> req,
  std::shared_ptr<camera_aravis2_msgs::srv::CalculateWhiteBalance::Response> res) const
{
    RCL_UNUSED(req);
    //--- set return values to default
    res->is_successful = false;
    //--- check for correct state of node
    if (!p_device_ || !this->is_initialized_)
        return;
    //--- set white balance mode to 'once' and get calculated values if successful
    if (setFeatureValue<std::string>("BalanceWhiteAuto", "Once"))
    {
        res->is_successful = true;
        RCLCPP_INFO(logger_, "Calculating white balance successful!");
        GuardedGError err;
        uint nRatioSelectorVals        = 0;
        const char** ratioSelectorVals = arv_camera_dup_available_enumerations_as_strings(
          this->p_camera_, "BalanceRatioSelector", &nRatioSelectorVals, err.ref());
        CHECK_GERROR_MSG(err, logger_,
                         "In getting values for feature 'BalanceRatioSelector'.");
        for (uint i = 0; i < nRatioSelectorVals; ++i)
        {
            diagnostic_msgs::msg::KeyValue kv_pair;
            kv_pair.key = std::string(ratioSelectorVals[i]);
            setFeatureValue<std::string>("BalanceRatioSelector", kv_pair.key);
            float ratioVal = 1.0;
            getFeatureValue<float>("BalanceRatio", ratioVal);
            kv_pair.value = std::to_string(ratioVal);
            res->balance_ratios.push_back(kv_pair);
            RCLCPP_INFO(logger_, "  BalanceRatio '%s':\t%s",
                        kv_pair.key.c_str(), kv_pair.value.c_str());
        }
        return;
    }
    else
    {
        RCLCPP_ERROR(logger_, "Trying to calculate white balance and setting auto mode to "
                              "'Once' failed!");
        return;
    }
}

//==================================================================================================
void CameraDriver::handleNewBufferSignal(ArvStream* p_stream, gpointer p_user_data)
{
    ///--- get data tuples from user data
    std::pair<CameraDriver*, uint>* p_data_tuple =
      (std::pair<CameraDriver*, uint>*)p_user_data;
    CameraDriver* p_ca_instance = p_data_tuple->first;
    uint stream_id              = p_data_tuple->second;

    Stream& stream = p_ca_instance->streams_[stream_id];

    ///--- get aravis buffer
    ArvBuffer* p_arv_buffer = arv_stream_try_pop_buffer(p_stream);

    //--- check if enough buffers are left, if not allocate one more for the next image
    gint n_available_buffers;
    arv_stream_get_n_buffers(p_stream, &n_available_buffers, NULL);
    if (n_available_buffers == 0)
        stream.p_buffer_pool->allocateBuffers(1);

    if (p_arv_buffer == nullptr)
        return;

    bool buffer_success = arv_buffer_get_status(p_arv_buffer) == ARV_BUFFER_STATUS_SUCCESS;
    bool buffer_pool    = static_cast<bool>(stream.p_buffer_pool);
    if (!buffer_success || !buffer_pool)
    {
        if (!buffer_success)
        {
            RCLCPP_WARN(p_ca_instance->logger_,
                        "(%s) Frame error: %s",
                        stream.sensor.frame_id.c_str(),
                        ARV_BUFFER_STATUS_FROM_INT[arv_buffer_get_status(p_arv_buffer)]);
        }

        arv_stream_push_buffer(p_stream, p_arv_buffer);
        return;
    }

    //--- push buffer pointers into concurrent queue to be processed by processing thread
    auto p_img_msg = (*stream.p_buffer_pool)[p_arv_buffer];
    stream.buffer_queue.push(std::make_pair(p_arv_buffer, p_img_msg));
}

}  // end namespace camera_aravis2
