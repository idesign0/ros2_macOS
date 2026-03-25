/*
 * Copyright (C) 2023 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#ifndef GZ_MSGS_CONVERT_PIXELFORMATTYPE_HH_
#define GZ_MSGS_CONVERT_PIXELFORMATTYPE_HH_

#include <gz/msgs/config.hh>

// Message Headers
#include "gz/msgs/image.pb.h"

// Data Headers
#include <string>

namespace gz::msgs {
// Inline bracket to help doxygen filtering.
inline namespace GZ_MSGS_VERSION_NAMESPACE {

/////////////////////////////////////////////
inline msgs::PixelFormatType ConvertPixelFormatType(const std::string &_str)
{
  if (_str == "L_INT8")
  {
    return msgs::PixelFormatType::L_INT8;
  }
  else if (_str == "L_INT16")
  {
    return msgs::PixelFormatType::L_INT16;
  }
  else if (_str == "RGB_INT8")
  {
    return msgs::PixelFormatType::RGB_INT8;
  }
  else if (_str == "RGBA_INT8")
  {
    return msgs::PixelFormatType::RGBA_INT8;
  }
  else if (_str == "BGRA_INT8")
  {
    return msgs::PixelFormatType::BGRA_INT8;
  }
  else if (_str == "RGB_INT16")
  {
    return msgs::PixelFormatType::RGB_INT16;
  }
  else if (_str == "RGB_INT32")
  {
    return msgs::PixelFormatType::RGB_INT32;
  }
  else if (_str == "BGR_INT8")
  {
    return msgs::PixelFormatType::BGR_INT8;
  }
  else if (_str == "BGR_INT16")
  {
    return msgs::PixelFormatType::BGR_INT16;
  }
  else if (_str == "BGR_INT32")
  {
    return msgs::PixelFormatType::BGR_INT32;
  }
  else if (_str == "R_FLOAT16")
  {
    return msgs::PixelFormatType::R_FLOAT16;
  }
  else if (_str == "RGB_FLOAT16")
  {
    return msgs::PixelFormatType::RGB_FLOAT16;
  }
  else if (_str == "R_FLOAT32")
  {
    return msgs::PixelFormatType::R_FLOAT32;
  }
  else if (_str == "RGB_FLOAT32")
  {
    return msgs::PixelFormatType::RGB_FLOAT32;
  }
  else if (_str == "BAYER_RGGB8")
  {
    return msgs::PixelFormatType::BAYER_RGGB8;
  }
  else if (_str == "BAYER_BGGR8")
  {
    return msgs::PixelFormatType::BAYER_BGGR8;
  }
  else if (_str == "BAYER_GBRG8")
  {
    return msgs::PixelFormatType::BAYER_GBRG8;
  }
  else if (_str == "BAYER_GRBG8")
  {
    return msgs::PixelFormatType::BAYER_GRBG8;
  }

  return msgs::PixelFormatType::UNKNOWN_PIXEL_FORMAT;
}

/////////////////////////////////////////////
inline std::string ConvertPixelFormatType(const msgs::PixelFormatType &_t)
{
  switch (_t)
  {
    default:
    case msgs::PixelFormatType::UNKNOWN_PIXEL_FORMAT:
      return "UNKNOWN_PIXEL_FORMAT";
    case msgs::PixelFormatType::L_INT8:
      return "L_INT8";
    case msgs::PixelFormatType::L_INT16:
      return "L_INT16";
    case msgs::PixelFormatType::RGB_INT8:
      return "RGB_INT8";
    case msgs::PixelFormatType::RGBA_INT8:
      return "RGBA_INT8";
    case msgs::PixelFormatType::BGRA_INT8:
      return "BGRA_INT8";
    case msgs::PixelFormatType::RGB_INT16:
      return "RGB_INT16";
    case msgs::PixelFormatType::RGB_INT32:
      return "RGB_INT32";
    case msgs::PixelFormatType::BGR_INT8:
      return "BGR_INT8";
    case msgs::PixelFormatType::BGR_INT16:
      return "BGR_INT16";
    case msgs::PixelFormatType::BGR_INT32:
      return "BGR_INT32";
    case msgs::PixelFormatType::R_FLOAT16:
      return "R_FLOAT16";
    case msgs::PixelFormatType::RGB_FLOAT16:
      return "RGB_FLOAT16";
    case msgs::PixelFormatType::R_FLOAT32:
      return "R_FLOAT32";
    case msgs::PixelFormatType::RGB_FLOAT32:
      return "RGB_FLOAT32";
    case msgs::PixelFormatType::BAYER_RGGB8:
      return "BAYER_RGGB8";
    case msgs::PixelFormatType::BAYER_BGGR8:
      return "BAYER_BGGR8";
    case msgs::PixelFormatType::BAYER_GBRG8:
      return "BAYER_GBRG8";
    case msgs::PixelFormatType::BAYER_GRBG8:
      return "BAYER_GRBG8";
  };
}


}  // namespace
}  // namespace gz::msgs

#endif  // GZ_MSGS_CONVERT_PIXELFORMATTYPE_HH_
