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

#include "camera_aravis2/camera_driver_uv.h"

// Std
#include <chrono>
#include <iostream>
#include <thread>

// ROS
#include <rclcpp/time.hpp>

// camera_aravis2
#include "camera_aravis2/conversion_utils.h"
#include "camera_aravis2/error.h"

namespace camera_aravis2
{

//==================================================================================================
CameraDriverUv::CameraDriverUv(const rclcpp::NodeOptions& options) :
  CameraDriver("camera_driver_uv", options)
{
    //--- setup parameters
    setupParameters();

    is_verbose_enable_ = get_parameter("verbose").as_bool();

    //--- get parameter overrides, i.e. all parameters, including those that are not declared
    parameter_overrides_ = this->get_node_parameters_interface()->get_parameter_overrides();

    //--- open camera device
    CHECK_SUCCESS(discoverAndOpenCameraDevice(), logger_);

    //--- check if UV Device.
    if (!arv_camera_is_uv_device(p_camera_))
    {
        RCLCPP_FATAL(logger_, "Camera is no USB3-Vision Device.");
        return;
    }

    //--- set up structs holding relevant information of camera streams
    CHECK_SUCCESS(setupCameraStreamStructs(), logger_);

    //--- set device control settings
    CHECK_SUCCESS(setDeviceControlSettings(), logger_);

    //--- set transport layer control settings
    p_uv_tl_control_ = std::make_shared<UvTransportLayerControl>();
    p_tl_control_    = std::dynamic_pointer_cast<GenTransportLayerControl>(p_uv_tl_control_);

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

    //--- print currently applied camera configuration
    printCameraConfiguration();

    //--- spawn camera stream in thread, so that initialization is not blocked
    is_spawning_         = true;
    spawn_stream_thread_ = std::thread(&CameraDriverUv::spawnCameraStreams, this);
}

//==================================================================================================
CameraDriverUv::~CameraDriverUv()
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
[[nodiscard]] bool CameraDriverUv::setTechSpecificTlControlSettings()
{
    std::shared_ptr<UvTransportLayerControl> p_uv_tl_control =
      std::dynamic_pointer_cast<UvTransportLayerControl>(p_tl_control_);

    //--- check if transport layer control is correctly initialized
    if (!p_uv_tl_control_)
    {
        RCLCPP_FATAL(logger_, "%s:  Pointer to UvTransportLayerControl is not initialized. ",
                     __PRETTY_FUNCTION__);

        return false;
    }

    return true;
}

//==================================================================================================
int CameraDriverUv::discoverNumberOfStreams()
{
    return 1;
}

//==================================================================================================
void CameraDriverUv::tuneArvStream(ArvStream* p_stream) const
{
    if (!p_stream)
        return;

    if (!ARV_IS_UV_STREAM(p_stream))
    {
        RCLCPP_ERROR(logger_, "Stream is not a UV_STREAM");
        return;
    }
}

//==================================================================================================
void CameraDriverUv::postFrameProcessingCallback(const uint stream_id)
{
    RCL_UNUSED(stream_id);
}

}  // end namespace camera_aravis2

#include "rclcpp_components/register_node_macro.hpp"

// Register the component with class_loader.
// This acts as a sort of entry point, allowing the component to be discoverable when its library
// is being loaded into a running process.
RCLCPP_COMPONENTS_REGISTER_NODE(camera_aravis2::CameraDriverUv)
