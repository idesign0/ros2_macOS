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

#include "camera_aravis2/camera_driver_gv.h"

// Std
#include <chrono>
#include <iostream>
#include <thread>

// ROS
#include <rcl_interfaces/msg/floating_point_range.hpp>
#include <rcl_interfaces/msg/integer_range.hpp>
#include <rclcpp/time.hpp>

// camera_aravis2
#include "camera_aravis2/conversion_utils.h"
#include "camera_aravis2/error.h"

namespace camera_aravis2
{

//==================================================================================================
CameraDriverGv::CameraDriverGv(const rclcpp::NodeOptions& options) :
  CameraDriver("camera_driver_gv", options)
{
    //--- setup parameters
    setupParameters();

    is_verbose_enable_ = get_parameter("verbose").as_bool();

    //--- get parameter overrides, i.e. all parameters, including those that are not declared
    parameter_overrides_ = this->get_node_parameters_interface()->get_parameter_overrides();

    //--- open camera device
    CHECK_SUCCESS(discoverAndOpenCameraDevice(), logger_);

    //--- check if GEV Device.
    if (!arv_camera_is_gv_device(p_camera_))
    {
        RCLCPP_FATAL(logger_, "Camera is no GigE-Vision Device.");
        return;
    }

    //--- set up structs holding relevant information of camera streams
    CHECK_SUCCESS(setupCameraStreamStructs(), logger_);

    //--- set device control settings
    CHECK_SUCCESS(setDeviceControlSettings(), logger_);

    //--- set transport layer control settings
    p_gv_tl_control_ = std::make_shared<GvTransportLayerControl>();
    p_tl_control_    = std::dynamic_pointer_cast<GenTransportLayerControl>(p_gv_tl_control_);

    CHECK_SUCCESS(setTransportLayerControlSettings(), logger_);

    //--- set image format control settings
    CHECK_SUCCESS(setImageFormatControlSettings(), logger_);

    //--- set acquisition control settings
    CHECK_SUCCESS(setAcquisitionControlSettings(), logger_);

    //--- set analog control settings
    CHECK_SUCCESS(setAnalogControlSettings(), logger_);

    //--- initialize services
    CHECK_SUCCESS(initializeServices(), logger_);

    //--- load dynamic parameters
    setupDynamicParameters();

    //--- load diagnostics
    setupCameraDiagnosticPublisher();

    //--- check ptp
    if (p_gv_tl_control_ && p_gv_tl_control_->is_ptp_enable)
        checkPtpState();

    //--- print currently applied camera configuration
    printCameraConfiguration();

    //--- spawn camera stream in thread, so that initialization is not blocked
    is_spawning_         = true;
    spawn_stream_thread_ = std::thread(&CameraDriverGv::spawnCameraStreams, this);
}

//==================================================================================================
CameraDriverGv::~CameraDriverGv()
{
    // Guarded error object
    GuardedGError err;

    //--- stop acquisition
    if (p_device_)
    {
        arv_device_execute_command(p_device_, "AcquisitionStop", err.ref());
        CHECK_GERROR_MSG(err, logger_, "In executing 'AcquisitionStop'.");
    }

    //--- stop emitting signals for streams
    for (uint i = 0; i < streams_.size(); i++)
        if (streams_[i].p_arv_stream)
            arv_stream_set_emit_signals(streams_[i].p_arv_stream, FALSE);

    //--- join spawning thread
    is_spawning_ = false;
    if (spawn_stream_thread_.joinable())
        spawn_stream_thread_.join();

    //--- join diagnostic thread
    is_diagnostics_published_ = false;
    if (diagnostic_thread_.joinable())
    {
        diagnostic_thread_.join();
    }

    //--- join buffer threads
    for (uint i = 0; i < streams_.size(); i++)
    {
        Stream& stream = streams_[i];

        stream.is_buffer_processed = false;

        //--- push empty object into queue to do final loop
        stream.buffer_queue.push(std::make_pair(nullptr, nullptr));

        if (stream.buffer_processing_thread.joinable())
            stream.buffer_processing_thread.join();
    }

    //--- print stream statistics
    printStreamStatistics();

    //--- unref pointers
    for (uint i = 0; i < streams_.size(); i++)
    {
        if (streams_[i].p_arv_stream)
            g_object_unref(streams_[i].p_arv_stream);
    }

    //--- camera / device object is unreferences in parent class
}

//==================================================================================================
[[nodiscard]] bool CameraDriverGv::setTechSpecificTlControlSettings()
{
    GuardedGError err;

    std::string tmp_feature_name;
    rclcpp::ParameterValue tmp_param_value;
    std::vector<std::pair<std::string, rclcpp::ParameterValue>> tmp_param_values;

    bool is_parameter_set;

    //--- check if transport layer control is correctly initialized
    if (!p_gv_tl_control_)
    {
        RCLCPP_FATAL(logger_, "%s:  Pointer to GevTransportLayerControl is not initialized. ",
                     __PRETTY_FUNCTION__);

        return false;
    }

    //--- for specific parameters, proceed as follows:
    //---   1. Check and read parameter from launch parameters
    //---   2. If provided in launch parameter, set value on device
    //---   3. Get the set value from the device and store it in corresponding struct
    //---   4. If specific parameter was given in the launch parameters, check against the actual
    //---       value which has been read from the camera and issue a warning message if applicable

    //--- GevSCPSPacketSize
    tmp_feature_name = "GevSCPSPacketSize";
    RCLCPP_DEBUG(logger_, "Evaluating 'TransportLayerControl.%s'", tmp_feature_name.c_str());
    is_parameter_set = getTransportLayerControlParameter(tmp_feature_name, tmp_param_value);
    if (is_parameter_set)
        setFeatureValueFromParameter<int64_t>(tmp_feature_name, tmp_param_value);
    getFeatureValue<int64_t>(tmp_feature_name, p_gv_tl_control_->packet_size);
    if (is_parameter_set &&
        !isParameterValueEqualTo<int64_t>(tmp_param_value, p_gv_tl_control_->packet_size))
        config_warn_msgs_.push_back("'" + tmp_feature_name + "' is not as specified.");

    //--- GevSCPD
    tmp_feature_name = "GevSCPD";
    RCLCPP_DEBUG(logger_, "Evaluating 'TransportLayerControl.%s'", tmp_feature_name.c_str());
    is_parameter_set = getTransportLayerControlParameter(tmp_feature_name, tmp_param_value);
    if (is_parameter_set)
        setFeatureValueFromParameter<int64_t>(tmp_feature_name, tmp_param_value);
    getFeatureValue<int64_t>(tmp_feature_name, p_gv_tl_control_->inter_packet_delay);
    if (is_parameter_set &&
        !isParameterValueEqualTo<int64_t>(tmp_param_value, p_gv_tl_control_->inter_packet_delay))
        config_warn_msgs_.push_back("'" + tmp_feature_name + "' is not as specified.");

    //--- set PTP Enable
    tmp_feature_name = "PtpEnable";
    RCLCPP_DEBUG(logger_, "Evaluating 'TransportLayerControl.%s'", tmp_feature_name.c_str());
    is_parameter_set = getTransportLayerControlParameter(tmp_feature_name, tmp_param_value);
    if (is_parameter_set)
        setFeatureValueFromParameter<bool>(tmp_feature_name, tmp_param_value);
    getFeatureValue<bool>(tmp_feature_name, p_gv_tl_control_->is_ptp_enable);
    if (is_parameter_set &&
        !isParameterValueEqualTo<bool>(tmp_param_value, p_gv_tl_control_->is_ptp_enable))
        config_warn_msgs_.push_back("'" + tmp_feature_name + "' is not as specified.");

    return true;
}

//==================================================================================================
int CameraDriverGv::discoverNumberOfStreams()
{
    int num_streams = 0;

    if (p_device_)
    {
        num_streams = arv_device_get_integer_feature_value(p_device_, "DeviceStreamChannelCount",
                                                           nullptr);

        if (num_streams == 0 && arv_camera_is_gv_device(p_camera_))
        {
            num_streams = arv_device_get_integer_feature_value(p_device_, "GevStreamChannelCount",
                                                               nullptr);
        }
    }

    if (num_streams == 0 || !p_device_)
    {
        num_streams = 1;
        RCLCPP_INFO(logger_, "Unable to automatically detect number of supported stream channels. "
                             "Setting num_streams = %i.",
                    num_streams);
    }
    else
    {
        RCLCPP_INFO(logger_, "Number of supported stream channels: %i", num_streams);
    }

    return num_streams;
}

//==================================================================================================
void CameraDriverGv::tuneArvStream(ArvStream* p_stream) const
{
    if (!p_stream)
        return;

    if (!ARV_IS_GV_STREAM(p_stream))
    {
        RCLCPP_ERROR(logger_, "Stream is not a GV_STREAM");
        return;
    }

    gboolean b_auto_buffer               = FALSE;
    gboolean b_packet_resend             = TRUE;
    unsigned int timeout_packet          = 40;  // milliseconds
    unsigned int timeout_frame_retention = 200;

    if (b_auto_buffer)
        g_object_set(p_stream, "socket-buffer",
                     ARV_GV_STREAM_SOCKET_BUFFER_AUTO,
                     "socket-buffer-size", 0,
                     NULL);
    if (!b_packet_resend)
        g_object_set(p_stream, "packet-resend",
                     b_packet_resend
                       ? ARV_GV_STREAM_PACKET_RESEND_ALWAYS
                       : ARV_GV_STREAM_PACKET_RESEND_NEVER,
                     NULL);
    g_object_set(p_stream, "packet-timeout",
                 timeout_packet * 1000,
                 "frame-retention", timeout_frame_retention * 1000,
                 NULL);
}

//==================================================================================================
void CameraDriverGv::postFrameProcessingCallback(const uint stream_id)
{
    RCL_UNUSED(stream_id);

    if (p_gv_tl_control_ && p_gv_tl_control_->is_ptp_enable)
        checkPtpState();
}

//==================================================================================================
void CameraDriverGv::checkPtpState()
{
    //--- get status
    getFeatureValue<std::string>("PtpStatus", p_gv_tl_control_->ptp_status);

    //--- check if clock needs reset
    if (p_gv_tl_control_->ptp_status == "Faulty" ||
        p_gv_tl_control_->ptp_status == "Disabled" ||
        p_gv_tl_control_->ptp_status == "Initializing" ||
        p_gv_tl_control_->ptp_status == "Uncalibrated")
    {
        RCLCPP_INFO_EXPRESSION(logger_, is_verbose_enable_,
                               "PTP Status: %s. Resetting PTP clock.",
                               p_gv_tl_control_->ptp_status.c_str());

        setFeatureValue<bool>("PtpEnable", false);
        setFeatureValue<bool>("PtpEnable", true);

        executeCommand("PtpDataSetLatch");

        getFeatureValue<std::string>("PtpStatus", p_gv_tl_control_->ptp_status);

        RCLCPP_INFO_EXPRESSION(logger_, is_verbose_enable_,
                               "New PTP Status: %s.",
                               p_gv_tl_control_->ptp_status.c_str());
    }
}

}  // end namespace camera_aravis2

#include "rclcpp_components/register_node_macro.hpp"

// Register the component with class_loader.
// This acts as a sort of entry point, allowing the component to be discoverable when its library
// is being loaded into a running process.
RCLCPP_COMPONENTS_REGISTER_NODE(camera_aravis2::CameraDriverGv)
