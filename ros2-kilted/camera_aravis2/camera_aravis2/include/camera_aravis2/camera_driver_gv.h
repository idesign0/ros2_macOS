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

#ifndef CAMERA_ARAVIS2__CAMERA_DRIVER_GV_H_
#define CAMERA_ARAVIS2__CAMERA_DRIVER_GV_H_

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
#include <rcl_interfaces/msg/set_parameters_result.hpp>
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
class CameraDriverGv : public CameraDriver
{
    //--- METHOD DECLARATION ---//

  public:
    /**
     * @brief Initialization constructor
     *
     * @param[in] options Node options.
     */
    explicit CameraDriverGv(const rclcpp::NodeOptions& options =
                              rclcpp::NodeOptions());

    /**
     * @brief Default destructor.
     *
     */
    virtual ~CameraDriverGv();

  protected:
    /**
     * @brief Method to set GigEVision specific transport layer control settings
     * of the camera.
     *
     * For example: PtpEnable, GevSCPSPacketSize...
     *
     * @return True if successful. False, otherwise.
     */
    [[nodiscard]] bool setTechSpecificTlControlSettings() override;

    /**
     * @brief Discover number of available camera streams.
     */
    int discoverNumberOfStreams() override;

    /**
     * @brief Tune specific parameters for GigEVision streams.
     *
     * @param[in] p_stream Pointer to Aravis Gv stream.
     */
    void tuneArvStream(ArvStream* p_stream) const override;

    /**
     * @brief Callback method to inject short processing routines after the
     * publication of each frame, e.g. check state of PTP.
     *
     * This is called within the processing loop of processStreamBuffer.
     *
     * @note This should not hold heavy processing as it will the delay the processing of the next
     * frame within the stream buffer.
     *
     * @param[in] stream_id ID of stream for which the buffer is to be processed.
     */
    void postFrameProcessingCallback(const uint stream_id) override;

    /**
     * @brief Check the status of the Precision Time Protocol and reset its clock if applicable.     *
     * The clock will only be reset if the status of PTP is "Faulty", "Disabled", or
     * "Initializing".
     */
    void checkPtpState();

    //--- MEMBER DECLARATION ---//

  protected:
    /// Pointer to Gev-Specific transport layer control settings.
    std::shared_ptr<GvTransportLayerControl> p_gv_tl_control_;
};

}  // namespace camera_aravis2

#endif  // CAMERA_ARAVIS2__CAMERA_DRIVER_GV_H_
