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

#include "camera_aravis2/camera_xml_exporter.h"

// Std
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>

// camera_aravis2
#include "camera_aravis2/error.h"

namespace camera_aravis2
{

//==================================================================================================
CameraXmlExporter::CameraXmlExporter(const rclcpp::NodeOptions& options) :
  CameraAravisNodeBase("camera_xml_exporter", options),
  xml_file_path_("")
{
    //--- setup parameters
    setupParameters();

    //--- open camera device
    CHECK_SUCCESS(discoverAndOpenCameraDevice(), logger_);

    std::string camera_guid_str = CameraAravisNodeBase::constructCameraGuidStr(p_camera_);
    RCLCPP_INFO(logger_, "Successfully Opened: %s", camera_guid_str.c_str());

    is_initialized_ = true;
}

//==================================================================================================
CameraXmlExporter::~CameraXmlExporter()
{
}

//==================================================================================================
[[nodiscard]] bool CameraXmlExporter::exportXmlDataToFile()
{
    if (!is_initialized_)
    {
        RCLCPP_ERROR(logger_, "'%s' is not initialized.", this->get_name());
        return false;
    }

    /// Path string of output xml file.
    std::string xml_file_str = get_parameter("xml_file").as_string();

    if (xml_file_str.empty())
    {
        std::string tmpFileName = (guid_.empty())
                                    ? constructCameraGuidStr(p_camera_)
                                    : guid_;

        // replace whitespaces in url (coming from GUID) with '_'
        std::replace(tmpFileName.begin(), tmpFileName.end(), ' ', '_');

        xml_file_path_ = std::filesystem::path(tmpFileName + ".xml");
    }
    else
    {
        xml_file_path_ = std::filesystem::path(xml_file_str);
    }

    //--- make path absolute
    xml_file_path_ = std::filesystem::absolute(xml_file_path_);

    //--- print warning if file already exists
    if (std::filesystem::exists(xml_file_path_))
        RCLCPP_WARN(logger_,
                    "Output file already exists and will be overwritten. Path: %s",
                    std::filesystem::canonical(xml_file_path_).c_str());

    //--- make parent directory if not existing
    if (!xml_file_path_.parent_path().empty())
        std::filesystem::create_directories(xml_file_path_.parent_path());

    //--- extract and save XML
    size_t xml_size        = 0;
    const char* p_xml_data = arv_device_get_genicam_xml(p_device_, &xml_size);
    std::ofstream fout;
    fout.open(xml_file_path_.c_str(), std::ios::binary | std::ios::out);
    fout.write(p_xml_data, xml_size);
    fout.close();

    RCLCPP_INFO(logger_, "Written GenICam XML to file: %s",
                std::filesystem::canonical(xml_file_path_).c_str());

    return true;
}

//==================================================================================================
void CameraXmlExporter::setupParameters()
{
    //--- call method of parent class
    CameraAravisNodeBase::setupParameters();

    auto xmlFile_desc = rcl_interfaces::msg::ParameterDescriptor{};
    xmlFile_desc.description =
      "Path to XML output file. If omitted, the XML data will be written into file with GUID as "
      "file name.";
    declare_parameter<std::string>("xml_file", "", xmlFile_desc);
}

}  // end namespace camera_aravis2

#include "rclcpp_components/register_node_macro.hpp"
