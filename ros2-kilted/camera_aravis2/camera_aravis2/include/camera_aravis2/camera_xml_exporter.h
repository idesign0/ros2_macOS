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

#ifndef CAMERA_ARAVIS2__CAMERA_XML_EXPORTER_H_
#define CAMERA_ARAVIS2__CAMERA_XML_EXPORTER_H_

// Std
#include <filesystem>
#include <string>

// ROS
#include <rclcpp/rclcpp.hpp>

// camera_aravis2
#include "camera_aravis2/camera_aravis_node_base.h"

namespace camera_aravis2
{

class CameraXmlExporter : public CameraAravisNodeBase
{
    //--- METHOD DECLARATION ---//

  public:
    /**
     * @brief Initialization constructor
     *
     * @param[in] options Node options.
     */
    explicit CameraXmlExporter(const rclcpp::NodeOptions& options = rclcpp::NodeOptions());

    /**
     * @brief Default destructor.
     *
     */
    virtual ~CameraXmlExporter();

    /**
     * @brief Export XML data from camera and write to file.
     *
     * @return True if successful. False, otherwise.
     */
    [[nodiscard]] bool exportXmlDataToFile();

  protected:
    /**
     * @overload
     * @brief Set the up launch parameters.
     */
    void setupParameters() override;

    //--- MEMBER DECLARATION ---//

  protected:
    /// Path of output xml file.
    std::filesystem::path xml_file_path_;
};

}  // namespace camera_aravis2

#endif  // CAMERA_ARAVIS2__CAMERA_XML_EXPORTER_H_
