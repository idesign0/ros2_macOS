/*
 * Copyright 2020 Martin Pecka
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
*/
#ifndef GZ_COMMON_HWVIDEO_HH
#define GZ_COMMON_HWVIDEO_HH

#include <gz/common/EnumIface.hh>

namespace gz::common
{
  enum class GZ_COMMON_AV_VISIBLE HWEncoderType
  {
    NONE,
    NVENC,  // Linux device is /dev/nvidia*
    // Windows is the same (even though such file doesn't exist)
    VAAPI,  // Linux device /dev/dri/renderD* or display number (e.g. :0)
    VDPAU,  // Not supported (probably only for decoding?)
    QSV,  // Win device is 0-based GPU index,
    // Linux uses /dev/dri/renderD* or display number (e.g. :0)
    VIDEOTOOLBOX,  // Not yet suported
    AMF,  // Not yet suported
    OMX,  // Not yet suported
    V4L2M2M,  // Not yet suported
    DXVA2,  // Not yet suported
    _  // For FlagSet to work.
  };

  GZ_ENUM(HWEncoderTypeParser, HWEncoderType,
    HWEncoderType::NONE, HWEncoderType::_,
    "NONE",
    "NVENC",
    "VAAPI",
    "VDPAU",
    "QSV",
    "VIDEOTOOLBOX",
    "AMF",
    "OMX",
    "V4L2M2M",
    "DXVA2",
    "INVALID"
  )
}

#endif
