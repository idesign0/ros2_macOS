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

#ifndef CAMERA_ARAVIS2__CAMERA_DRIVER_UV_H_
#define CAMERA_ARAVIS2__CAMERA_DRIVER_UV_H_
// Yaml-cpp
#include <yaml-cpp/yaml.h>

// Std
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

// Aravis
extern "C"
{
#include "aravis-0.8/arv.h"
}

// ROS
#include <camera_info_manager/camera_info_manager.hpp>
#include <image_transport/image_transport.hpp>
#include <rclcpp/rclcpp.hpp>

// camera_aravis2
#include "camera_aravis2/camera_driver.h"
#include "camera_aravis2/concurrent_queue.hpp"
#include "camera_aravis2/config_structs.h"
#include "camera_aravis2/conversion_utils.h"
#include "camera_aravis2/image_buffer_pool.h"
#include <camera_aravis2_msgs/msg/camera_diagnostics.hpp>

namespace camera_aravis2
{
class CameraDriverUv : public CameraDriver
{
    //--- METHOD DECLARATION ---//

  public:
    /**
     * @brief Initialization constructor
     *
     * @param[in] options Node options.
     */
    explicit CameraDriverUv(const rclcpp::NodeOptions& options =
                              rclcpp::NodeOptions());

    /**
     * @brief Default destructor.
     *
     */
    virtual ~CameraDriverUv();

  protected:
    /**
     * @brief Method to set UsbVision specific transport layer control settings
     * of the camera.
     *
     * @return True if successful. False, otherwise.
     */
    [[nodiscard]] bool setTechSpecificTlControlSettings() override;

    /**
     * @brief Discover number of available camera streams. In case of USB3 cameras this will
     * return 1.
     *
     */
    int discoverNumberOfStreams() override;

    /**
     * @brief Tune specific parameters for Aravis streams.
     *
     * @param[in] p_stream Pointer to Aravis stream.
     */
    void tuneArvStream(ArvStream* p_stream) const override;

    /**
     * @brief Callback method to inject short processing routines after the
     * publication of each frame.
     *
     * This is called within the processing loop of processStreamBuffer.
     *
     * @note This should not hold heavy processing as it will the delay the processing of the next
     * frame within the stream buffer.
     *
     * @param[in] stream_id ID of stream for which the buffer is to be processed.
     */
    void postFrameProcessingCallback(const uint stream_id) override;

    //--- MEMBER DECLARATION ---//

  protected:
    /// Pointer to Uv-Specific transport layer control settings.
    std::shared_ptr<UvTransportLayerControl> p_uv_tl_control_;
};

}  // namespace camera_aravis2

#endif  // CAMERA_ARAVIS2__CAMERA_DRIVER_UV_H_
