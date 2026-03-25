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

#ifndef CAMERA_ARAVIS2__CONVERSION_UTILS_H_
#define CAMERA_ARAVIS2__CONVERSION_UTILS_H_

// Std
#include <functional>
#include <map>
#include <string>

// ROS
#include <sensor_msgs/image_encodings.hpp>
#include <sensor_msgs/msg/image.hpp>

namespace camera_aravis2
{

// Conversion functions from GenICam to ROS formats
using ConversionFunction = std::function<void(sensor_msgs::msg::Image::SharedPtr& in,
                                              sensor_msgs::msg::Image::SharedPtr& out)>;

[[nodiscard]] bool renameImg(sensor_msgs::msg::Image::SharedPtr& in,
                             sensor_msgs::msg::Image::SharedPtr& out,
                             const std::string out_format);
[[nodiscard]] bool shiftImg(sensor_msgs::msg::Image::SharedPtr& in,
                            sensor_msgs::msg::Image::SharedPtr& out,
                            const size_t n_digits, const std::string out_format);
[[nodiscard]] bool interleaveImg(sensor_msgs::msg::Image::SharedPtr& in,
                                 sensor_msgs::msg::Image::SharedPtr& out,
                                 const size_t n_digits, const std::string out_format);
[[nodiscard]] bool unpack10p32Img(sensor_msgs::msg::Image::SharedPtr& in,
                                  sensor_msgs::msg::Image::SharedPtr& out,
                                  const std::string out_format);
[[nodiscard]] bool unpack10PackedImg(sensor_msgs::msg::Image::SharedPtr& in,
                                     sensor_msgs::msg::Image::SharedPtr& out,
                                     const std::string out_format);
[[nodiscard]] bool unpack10pMonoImg(sensor_msgs::msg::Image::SharedPtr& in,
                                    sensor_msgs::msg::Image::SharedPtr& out,
                                    const std::string out_format);
[[nodiscard]] bool unpack10PackedMonoImg(sensor_msgs::msg::Image::SharedPtr& in,
                                         sensor_msgs::msg::Image::SharedPtr& out,
                                         const std::string out_format);
[[nodiscard]] bool unpack12pImg(sensor_msgs::msg::Image::SharedPtr& in,
                                sensor_msgs::msg::Image::SharedPtr& out,
                                const std::string out_format);
[[nodiscard]] bool unpack12PackedImg(sensor_msgs::msg::Image::SharedPtr& in,
                                     sensor_msgs::msg::Image::SharedPtr& out,
                                     const std::string out_format);
[[nodiscard]] bool unpack565pImg(sensor_msgs::msg::Image::SharedPtr& in,
                                 sensor_msgs::msg::Image::SharedPtr& out,
                                 const std::string out_format);

const std::map<std::string, ConversionFunction> CONVERSIONS_DICTIONARY =
  {
    // equivalent to official ROS color encodings
    {"RGB8", std::bind(&renameImg, std::placeholders::_1, std::placeholders::_2,
                       sensor_msgs::image_encodings::RGB8)},
    {"RGBa8", std::bind(&renameImg, std::placeholders::_1, std::placeholders::_2,
                        sensor_msgs::image_encodings::RGBA8)},
    {"RGB16", std::bind(&renameImg, std::placeholders::_1, std::placeholders::_2,
                        sensor_msgs::image_encodings::RGB16)},
    {"RGBa16", std::bind(&renameImg, std::placeholders::_1, std::placeholders::_2,
                         sensor_msgs::image_encodings::RGBA16)},
    {"BGR8", std::bind(&renameImg, std::placeholders::_1, std::placeholders::_2,
                       sensor_msgs::image_encodings::BGR8)},
    {"BGRa8", std::bind(&renameImg, std::placeholders::_1, std::placeholders::_2,
                        sensor_msgs::image_encodings::BGRA8)},
    {"BGR16", std::bind(&renameImg, std::placeholders::_1, std::placeholders::_2,
                        sensor_msgs::image_encodings::BGR16)},
    {"BGRa16", std::bind(&renameImg, std::placeholders::_1, std::placeholders::_2,
                         sensor_msgs::image_encodings::BGRA16)},
    {"Mono8", std::bind(&renameImg, std::placeholders::_1, std::placeholders::_2,
                        sensor_msgs::image_encodings::MONO8)},
    {"Raw8", std::bind(&renameImg, std::placeholders::_1, std::placeholders::_2,
                       sensor_msgs::image_encodings::MONO8)},
    {"R8", std::bind(&renameImg, std::placeholders::_1, std::placeholders::_2,
                     sensor_msgs::image_encodings::MONO8)},
    {"G8", std::bind(&renameImg, std::placeholders::_1, std::placeholders::_2,
                     sensor_msgs::image_encodings::MONO8)},
    {"B8", std::bind(&renameImg, std::placeholders::_1, std::placeholders::_2,
                     sensor_msgs::image_encodings::MONO8)},
    {"Mono16", std::bind(&renameImg, std::placeholders::_1, std::placeholders::_2,
                         sensor_msgs::image_encodings::MONO16)},
    {"Raw16", std::bind(&renameImg, std::placeholders::_1, std::placeholders::_2,
                        sensor_msgs::image_encodings::MONO16)},
    {"R16", std::bind(&renameImg, std::placeholders::_1, std::placeholders::_2,
                      sensor_msgs::image_encodings::MONO16)},
    {"G16", std::bind(&renameImg, std::placeholders::_1, std::placeholders::_2,
                      sensor_msgs::image_encodings::MONO16)},
    {"B16", std::bind(&renameImg, std::placeholders::_1, std::placeholders::_2,
                      sensor_msgs::image_encodings::MONO16)},
    {"BayerRG8", std::bind(&renameImg, std::placeholders::_1, std::placeholders::_2,
                           sensor_msgs::image_encodings::BAYER_RGGB8)},
    {"BayerBG8", std::bind(&renameImg, std::placeholders::_1, std::placeholders::_2,
                           sensor_msgs::image_encodings::BAYER_BGGR8)},
    {"BayerGB8", std::bind(&renameImg, std::placeholders::_1, std::placeholders::_2,
                           sensor_msgs::image_encodings::BAYER_GBRG8)},
    {"BayerGR8", std::bind(&renameImg, std::placeholders::_1, std::placeholders::_2,
                           sensor_msgs::image_encodings::BAYER_GRBG8)},
    {"BayerRG16", std::bind(&renameImg, std::placeholders::_1, std::placeholders::_2,
                            sensor_msgs::image_encodings::BAYER_RGGB16)},
    {"BayerBG16", std::bind(&renameImg, std::placeholders::_1, std::placeholders::_2,
                            sensor_msgs::image_encodings::BAYER_BGGR16)},
    {"BayerGB16", std::bind(&renameImg, std::placeholders::_1, std::placeholders::_2,
                            sensor_msgs::image_encodings::BAYER_GBRG16)},
    {"BayerGR16", std::bind(&renameImg, std::placeholders::_1, std::placeholders::_2,
                            sensor_msgs::image_encodings::BAYER_GRBG16)},
    {"YUV422_8_UYVY", std::bind(&renameImg, std::placeholders::_1, std::placeholders::_2,
                                sensor_msgs::image_encodings::YUV422)},
    {"YUV422_8", std::bind(&renameImg, std::placeholders::_1, std::placeholders::_2,
                           sensor_msgs::image_encodings::YUV422)},

    // non-color contents
    {"Data8", std::bind(&renameImg, std::placeholders::_1, std::placeholders::_2,
                        sensor_msgs::image_encodings::TYPE_8UC1)},
    {"Confidence8", std::bind(&renameImg, std::placeholders::_1, std::placeholders::_2,
                              sensor_msgs::image_encodings::TYPE_8UC1)},
    {"Data8s", std::bind(&renameImg, std::placeholders::_1, std::placeholders::_2,
                         sensor_msgs::image_encodings::TYPE_8SC1)},
    {"Data16", std::bind(&renameImg, std::placeholders::_1, std::placeholders::_2,
                         sensor_msgs::image_encodings::TYPE_16UC1)},
    {"Confidence16", std::bind(&renameImg, std::placeholders::_1, std::placeholders::_2,
                               sensor_msgs::image_encodings::TYPE_16UC1)},
    {"Data16s", std::bind(&renameImg, std::placeholders::_1, std::placeholders::_2,
                          sensor_msgs::image_encodings::TYPE_16SC1)},
    {"Data32s", std::bind(&renameImg, std::placeholders::_1, std::placeholders::_2,
                          sensor_msgs::image_encodings::TYPE_32SC1)},
    {"Data32f", std::bind(&renameImg, std::placeholders::_1, std::placeholders::_2,
                          sensor_msgs::image_encodings::TYPE_32FC1)},
    {"Confidence32f", std::bind(&renameImg, std::placeholders::_1, std::placeholders::_2,
                                sensor_msgs::image_encodings::TYPE_32FC1)},
    {"Data64f", std::bind(&renameImg, std::placeholders::_1, std::placeholders::_2,
                          sensor_msgs::image_encodings::TYPE_64FC1)},

    // unthrifty formats. Shift away padding Bits for use with ROS.
    {"Mono10", std::bind(&shiftImg, std::placeholders::_1, std::placeholders::_2, 6,
                         sensor_msgs::image_encodings::MONO16)},
    {"Mono12", std::bind(&shiftImg, std::placeholders::_1, std::placeholders::_2, 4,
                         sensor_msgs::image_encodings::MONO16)},
    {"Mono14", std::bind(&shiftImg, std::placeholders::_1, std::placeholders::_2, 2,
                         sensor_msgs::image_encodings::MONO16)},
    {"RGB10", std::bind(&shiftImg, std::placeholders::_1, std::placeholders::_2, 6,
                        sensor_msgs::image_encodings::RGB16)},
    {"RGB12", std::bind(&shiftImg, std::placeholders::_1, std::placeholders::_2, 4,
                        sensor_msgs::image_encodings::RGB16)},
    {"BGR10", std::bind(&shiftImg, std::placeholders::_1, std::placeholders::_2, 6,
                        sensor_msgs::image_encodings::BGR16)},
    {"BGR12", std::bind(&shiftImg, std::placeholders::_1, std::placeholders::_2, 4,
                        sensor_msgs::image_encodings::BGR16)},
    {"BayerRG10", std::bind(&shiftImg, std::placeholders::_1, std::placeholders::_2, 6,
                            sensor_msgs::image_encodings::BAYER_RGGB16)},
    {"BayerBG10", std::bind(&shiftImg, std::placeholders::_1, std::placeholders::_2, 6,
                            sensor_msgs::image_encodings::BAYER_BGGR16)},
    {"BayerGB10", std::bind(&shiftImg, std::placeholders::_1, std::placeholders::_2, 6,
                            sensor_msgs::image_encodings::BAYER_GBRG16)},
    {"BayerGR10", std::bind(&shiftImg, std::placeholders::_1, std::placeholders::_2, 6,
                            sensor_msgs::image_encodings::BAYER_GRBG16)},
    {"BayerRG12", std::bind(&shiftImg, std::placeholders::_1, std::placeholders::_2, 4,
                            sensor_msgs::image_encodings::BAYER_RGGB16)},
    {"BayerBG12", std::bind(&shiftImg, std::placeholders::_1, std::placeholders::_2, 4,
                            sensor_msgs::image_encodings::BAYER_BGGR16)},
    {"BayerGB12", std::bind(&shiftImg, std::placeholders::_1, std::placeholders::_2, 4,
                            sensor_msgs::image_encodings::BAYER_GBRG16)},
    {"BayerGR12", std::bind(&shiftImg, std::placeholders::_1, std::placeholders::_2, 4,
                            sensor_msgs::image_encodings::BAYER_GRBG16)},

    // planar instead pixel-by-pixel encodings
    {"RGB8_Planar", std::bind(&interleaveImg, std::placeholders::_1, std::placeholders::_2, 0,
                              sensor_msgs::image_encodings::RGB8)},
    {"RGB10_Planar", std::bind(&interleaveImg, std::placeholders::_1, std::placeholders::_2, 6,
                               sensor_msgs::image_encodings::RGB16)},
    {"RGB12_Planar", std::bind(&interleaveImg, std::placeholders::_1, std::placeholders::_2, 4,
                               sensor_msgs::image_encodings::RGB16)},
    {"RGB16_Planar", std::bind(&interleaveImg, std::placeholders::_1, std::placeholders::_2, 0,
                               sensor_msgs::image_encodings::RGB16)},
    // packed, non-Byte aligned formats
    {"Mono10p", std::bind(&unpack10pMonoImg, std::placeholders::_1, std::placeholders::_2,
                          sensor_msgs::image_encodings::MONO16)},
    {"RGB10p", std::bind(&unpack10p32Img, std::placeholders::_1, std::placeholders::_2,
                         sensor_msgs::image_encodings::RGB16)},
    {"RGB10p32", std::bind(&unpack10p32Img, std::placeholders::_1, std::placeholders::_2,
                           sensor_msgs::image_encodings::RGB16)},
    {"RGBa10p", std::bind(&unpack10p32Img, std::placeholders::_1, std::placeholders::_2,
                          sensor_msgs::image_encodings::RGBA16)},
    {"BGR10p", std::bind(&unpack10p32Img, std::placeholders::_1, std::placeholders::_2,
                         sensor_msgs::image_encodings::BGR16)},
    {"BGRa10p", std::bind(&unpack10p32Img, std::placeholders::_1, std::placeholders::_2,
                          sensor_msgs::image_encodings::BGRA16)},
    {"BayerRG10p", std::bind(&unpack10pMonoImg, std::placeholders::_1, std::placeholders::_2,
                             sensor_msgs::image_encodings::BAYER_RGGB16)},
    {"BayerBG10p", std::bind(&unpack10pMonoImg, std::placeholders::_1, std::placeholders::_2,
                             sensor_msgs::image_encodings::BAYER_BGGR16)},
    {"BayerGB10p", std::bind(&unpack10pMonoImg, std::placeholders::_1, std::placeholders::_2,
                             sensor_msgs::image_encodings::BAYER_GBRG16)},
    {"BayerGR10p", std::bind(&unpack10pMonoImg, std::placeholders::_1, std::placeholders::_2,
                             sensor_msgs::image_encodings::BAYER_GRBG16)},
    {"Mono12p", std::bind(&unpack12pImg, std::placeholders::_1, std::placeholders::_2,
                          sensor_msgs::image_encodings::MONO16)},
    {"RGB12p", std::bind(&unpack12pImg, std::placeholders::_1, std::placeholders::_2,
                         sensor_msgs::image_encodings::RGB16)},
    {"RGBa12p", std::bind(&unpack12pImg, std::placeholders::_1, std::placeholders::_2,
                          sensor_msgs::image_encodings::RGBA16)},
    {"BGR12p", std::bind(&unpack12pImg, std::placeholders::_1, std::placeholders::_2,
                         sensor_msgs::image_encodings::BGR16)},
    {"BGRa12p", std::bind(&unpack12pImg, std::placeholders::_1, std::placeholders::_2,
                          sensor_msgs::image_encodings::BGRA16)},
    {"BayerRG12p", std::bind(&unpack12pImg, std::placeholders::_1, std::placeholders::_2,
                             sensor_msgs::image_encodings::BAYER_RGGB16)},
    {"BayerBG12p", std::bind(&unpack12pImg, std::placeholders::_1, std::placeholders::_2,
                             sensor_msgs::image_encodings::BAYER_BGGR16)},
    {"BayerGB12p", std::bind(&unpack12pImg, std::placeholders::_1, std::placeholders::_2,
                             sensor_msgs::image_encodings::BAYER_GBRG16)},
    {"BayerGR12p", std::bind(&unpack12pImg, std::placeholders::_1, std::placeholders::_2,
                             sensor_msgs::image_encodings::BAYER_GRBG16)},
    {"RGB565p", std::bind(&unpack565pImg, std::placeholders::_1, std::placeholders::_2,
                          sensor_msgs::image_encodings::RGB8)},
    {"BGR565p", std::bind(&unpack565pImg, std::placeholders::_1, std::placeholders::_2,
                          sensor_msgs::image_encodings::BGR8)},

    // GigE-Vision specific format naming
    {"RGB10V1Packed", std::bind(&unpack10PackedImg, std::placeholders::_1, std::placeholders::_2,
                                sensor_msgs::image_encodings::RGB16)},
    {"RGB10V2Packed", std::bind(&unpack10p32Img, std::placeholders::_1, std::placeholders::_2,
                                sensor_msgs::image_encodings::RGB16)},
    {"RGB12V1Packed", std::bind(&unpack12PackedImg, std::placeholders::_1, std::placeholders::_2,
                                sensor_msgs::image_encodings::RGB16)},
    {"Mono10Packed", std::bind(&unpack10PackedMonoImg, std::placeholders::_1, std::placeholders::_2,
                               sensor_msgs::image_encodings::MONO16)},
    {"Mono12Packed", std::bind(&unpack12PackedImg, std::placeholders::_1, std::placeholders::_2,
                               sensor_msgs::image_encodings::MONO16)},
    {"BayerRG10Packed", std::bind(&unpack10PackedMonoImg,
                                  std::placeholders::_1, std::placeholders::_2,
                                  sensor_msgs::image_encodings::BAYER_RGGB16)},
    {"BayerBG10Packed", std::bind(&unpack10PackedMonoImg,
                                  std::placeholders::_1, std::placeholders::_2,
                                  sensor_msgs::image_encodings::BAYER_BGGR16)},
    {"BayerGB10Packed", std::bind(&unpack10PackedMonoImg,
                                  std::placeholders::_1, std::placeholders::_2,
                                  sensor_msgs::image_encodings::BAYER_GBRG16)},
    {"BayerGR10Packed", std::bind(&unpack10PackedMonoImg,
                                  std::placeholders::_1, std::placeholders::_2,
                                  sensor_msgs::image_encodings::BAYER_GRBG16)},
    {"BayerRG12Packed", std::bind(&unpack12PackedImg, std::placeholders::_1, std::placeholders::_2,
                                  sensor_msgs::image_encodings::BAYER_RGGB16)},
    {"BayerBG12Packed", std::bind(&unpack12PackedImg, std::placeholders::_1, std::placeholders::_2,
                                  sensor_msgs::image_encodings::BAYER_BGGR16)},
    {"BayerGB12Packed", std::bind(&unpack12PackedImg, std::placeholders::_1, std::placeholders::_2,
                                  sensor_msgs::image_encodings::BAYER_GBRG16)},
    {"BayerGR12Packed", std::bind(&unpack12PackedImg, std::placeholders::_1, std::placeholders::_2,
                                  sensor_msgs::image_encodings::BAYER_GRBG16)},
    {"YUV422Packed", std::bind(&renameImg, std::placeholders::_1, std::placeholders::_2,
                               sensor_msgs::image_encodings::YUV422)}};

}  // end namespace camera_aravis2

#endif  // CAMERA_ARAVIS2__CONVERSION_UTILS_H_
