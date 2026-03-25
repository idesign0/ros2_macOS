#!/usr/bin/env python3

data = {
    "Color": [["gz/msgs/color.pb.h", "gz/math/Color.hh", "gz::msgs::Color", "gz::math::Color"]],
    "AxisAlignedBox": [["gz/msgs/axis_aligned_box.pb.h", "gz/math/AxisAlignedBox.hh", "gz::msgs::AxisAlignedBox", "gz::math::AxisAlignedBox"]],
    "SphericalCoordinates": [["gz/msgs/spherical_coordinates.pb.h", "gz/math/SphericalCoordinates.hh", "gz::msgs::SphericalCoordinates", "gz::math::SphericalCoordinates"]],
    "Inertial": [
        ["gz/msgs/inertial.pb.h", "gz/math/Inertial.hh", "gz::msgs::Inertial", "gz::math::Inertiald"],
        ["", "gz/math/MassMatrix3.hh", "gz::msgs::Inertial", "gz::math::MassMatrix3d"]
    ],
    "Pose": [["gz/msgs/pose.pb.h", "gz/math/Pose3.hh", "gz::msgs::Pose", "gz::math::Pose3d"]],
    "Plane": [["gz/msgs/planegeom.pb.h", "gz/math/Plane.hh", "gz::msgs::PlaneGeom", "gz::math::Planed"]],
    "Quaternion": [["gz/msgs/quaternion.pb.h", "gz/math/Quaternion.hh", "gz::msgs::Quaternion", "gz::math::Quaterniond"]],
    "Vector2": [["gz/msgs/vector2d.pb.h", "gz/math/Vector2.hh", "gz::msgs::Vector2d", "gz::math::Vector2d"]],
    "Vector3": [["gz/msgs/vector3d.pb.h", "gz/math/Vector3.hh", "gz::msgs::Vector3d", "gz::math::Vector3d"]],
    "StdTypes": [
        ["gz/msgs/time.pb.h", "chrono", "gz::msgs::Time", "std::chrono::steady_clock::duration"],
        ["gz/msgs/stringmsg.pb.h", "string", "gz::msgs::StringMsg", "std::string"],
        ["gz/msgs/boolean.pb.h", "", "gz::msgs::Boolean", "bool"],
        ["gz/msgs/int32.pb.h", "", "gz::msgs::Int32", "int32_t"],
        ["gz/msgs/uint32.pb.h", "", "gz::msgs::UInt32", "uint32_t"],
        ["gz/msgs/int64.pb.h", "", "gz::msgs::Int64", "int64_t"],
        ["gz/msgs/uint64.pb.h", "", "gz::msgs::UInt64", "uint64_t"],
        ["gz/msgs/float.pb.h", "", "gz::msgs::Float", "float"],
        ["gz/msgs/double.pb.h", "", "gz::msgs::Double", "double"],
    ],
}

template = """/*
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
#ifndef GZ_MSGS_CONVERT_{header}_HH_
#define GZ_MSGS_CONVERT_{header}_HH_

#include <gz/msgs/Converter.hh>
#include <gz/msgs/config.hh>

// Message Headers
{msg_headers}

// Data Headers
{data_headers}

namespace gz::msgs {{
// Inline bracket to help doxygen filtering.
inline namespace GZ_MSGS_VERSION_NAMESPACE {{

{conversions}

}}  // namespace
}}  // namespace gz::msgs

#endif  // GZ_MSGS_CONVERT_VECTOR3_HH_
"""

convert_template = """/////////////////////////////////
template<>
inline void Converter<{msg_type}, {data_type}>::Set({msg_type} *_msg, const {data_type} &_data)
{{
}}

template<>
inline void Converter<{msg_type}, {data_type}>::Set({data_type} *_data, const {msg_type} &_msg)
{{
}}

inline {msg_type} Convert(const {data_type} &_data)
{{
  return Converter<{msg_type}, {data_type}>::Convert(_data);
}}

inline {data_type} Convert(const {msg_type} &_msg)
{{
  return Converter<{msg_type}, {data_type}>::Convert(_msg);
}}
"""

headers = []

for header, v in data.items():
    msg_headers = []
    data_headers = []
    conversions = []

    headers.append(f'#include <gz/msgs/convert/{header}.hh>')

    for entry in v:
        msg_header = entry[0]
        data_header = entry[1]
        msg_type = entry[2]
        data_type = entry[3]

        if len(msg_header):
            msg_headers.append(f'#include "{msg_header}"')
        if len(data_header):
            data_headers.append(f'#include <{data_header}>')
        conversions.append(convert_template.format(**locals()))

    msg_headers = '\n'.join(sorted(msg_headers))
    data_headers = '\n'.join(sorted(data_headers))
    conversions = '\n'.join(conversions)

    with open(header + '.hh', 'w') as f:
        header = header.upper()
        f.write(template.format(**locals()))

print('\n'.join(sorted(headers)))
