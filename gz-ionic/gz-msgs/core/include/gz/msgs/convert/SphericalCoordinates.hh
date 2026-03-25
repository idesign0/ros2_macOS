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
#ifndef GZ_MSGS_CONVERT_SPHERICALCOORDINATES_HH_
#define GZ_MSGS_CONVERT_SPHERICALCOORDINATES_HH_

#include <gz/msgs/config.hh>

// Message Headers
#include "gz/msgs/spherical_coordinates.pb.h"

// Data Headers
#include <gz/math/SphericalCoordinates.hh>

namespace gz::msgs {
// Inline bracket to help doxygen filtering.
inline namespace GZ_MSGS_VERSION_NAMESPACE {


/////////////////////////////////////////////
inline msgs::SphericalCoordinatesType ConvertCoord(
  const math::SphericalCoordinates::CoordinateType &_sc)
{
  GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION
  auto result = msgs::SphericalCoordinatesType::LOCAL2;
  GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION
  switch (_sc)
  {
    case math::SphericalCoordinates::CoordinateType::ECEF:
      result = msgs::SphericalCoordinatesType::ECEF;
      break;
    case math::SphericalCoordinates::CoordinateType::GLOBAL:
      result = msgs::SphericalCoordinatesType::GLOBAL;
      break;
    case math::SphericalCoordinates::CoordinateType::SPHERICAL:
      result = msgs::SphericalCoordinatesType::SPHERICAL;
      break;
    case math::SphericalCoordinates::CoordinateType::LOCAL:
      result = msgs::SphericalCoordinatesType::LOCAL;
      break;
    GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION
    case math::SphericalCoordinates::CoordinateType::LOCAL2:
      result = msgs::SphericalCoordinatesType::LOCAL2;
      break;
    GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION
    default:
      std::cerr << "Invalid coordinate type passed" << std::endl;
  }
  return result;
}

/////////////////////////////////////////////
inline math::SphericalCoordinates::CoordinateType Convert(
  const msgs::SphericalCoordinatesType &_sc)
{
  switch (_sc)
  {
    case msgs::SphericalCoordinatesType::ECEF:
      return math::SphericalCoordinates::CoordinateType::ECEF;
    case msgs::SphericalCoordinatesType::GLOBAL:
      return math::SphericalCoordinates::CoordinateType::GLOBAL;
    case msgs::SphericalCoordinatesType::SPHERICAL:
      return math::SphericalCoordinates::CoordinateType::SPHERICAL;
    case msgs::SphericalCoordinatesType::LOCAL:
      return math::SphericalCoordinates::CoordinateType::LOCAL;
    GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION
    case msgs::SphericalCoordinatesType::LOCAL2:
      return math::SphericalCoordinates::CoordinateType::LOCAL2;
    GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION
    default:
      std::cerr << "Invalid coordinate type passed" << std::endl;
  }
  GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION
  return math::SphericalCoordinates::CoordinateType::LOCAL2;
  GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION
}

/////////////////////////////////
inline void Set(gz::msgs::SphericalCoordinates *_msg,
                const gz::math::SphericalCoordinates &_data)
{
  if (_data.Surface() == math::SphericalCoordinates::EARTH_WGS84)
  {
    _msg->set_surface_model(msgs::SphericalCoordinates::EARTH_WGS84);
  }
  else if (_data.Surface() == math::SphericalCoordinates::MOON_SCS)
  {
    _msg->set_surface_model(msgs::SphericalCoordinates::MOON_SCS);
  }
  else if (_data.Surface() ==
      math::SphericalCoordinates::CUSTOM_SURFACE)
  {
    _msg->set_surface_model(
        msgs::SphericalCoordinates::CUSTOM_SURFACE);
    _msg->set_surface_axis_equatorial(_data.SurfaceAxisEquatorial());
    _msg->set_surface_axis_polar(_data.SurfaceAxisPolar());
  }
  else
  {
    std::cerr << "Unrecognized spherical surface type ["
              << _data.Surface()
              << "]. Not populating message field." << std::endl;
  }
  _msg->set_latitude_deg(_data.LatitudeReference().Degree());
  _msg->set_longitude_deg(_data.LongitudeReference().Degree());
  _msg->set_elevation(_data.ElevationReference());
  _msg->set_heading_deg(_data.HeadingOffset().Degree());
}

inline void Set(gz::math::SphericalCoordinates *_data,
                const gz::msgs::SphericalCoordinates &_msg)
{
  if (_msg.surface_model() == msgs::SphericalCoordinates::EARTH_WGS84)
  {
    _data->SetSurface(math::SphericalCoordinates::EARTH_WGS84);
  }
  else if (_msg.surface_model() == msgs::SphericalCoordinates::MOON_SCS)
  {
    _data->SetSurface(math::SphericalCoordinates::MOON_SCS);
  }
  else if (_msg.surface_model() == msgs::SphericalCoordinates::CUSTOM_SURFACE)
  {
    _data->SetSurface(math::SphericalCoordinates::CUSTOM_SURFACE,
                      _msg.surface_axis_equatorial(),
                      _msg.surface_axis_polar());
  }
  else
  {
    std::cerr << "Unrecognized spherical surface type ["
              << _msg.surface_model()
              << "]. Not populating data field." << std::endl;
  }

  auto lat = gz::math::Angle();
  lat.SetDegree(_msg.latitude_deg());
  _data->SetLatitudeReference(lat);

  auto lon = gz::math::Angle();
  lon.SetDegree(_msg.longitude_deg());
  _data->SetLongitudeReference(lon);

  auto head = gz::math::Angle();
  head.SetDegree(_msg.heading_deg());
  _data->SetHeadingOffset(head);

  _data->SetElevationReference(_msg.elevation());
}

inline gz::msgs::SphericalCoordinates
Convert(const gz::math::SphericalCoordinates &_data)
{
  gz::msgs::SphericalCoordinates ret;
  Set(&ret, _data);
  return ret;
}

inline gz::math::SphericalCoordinates
  Convert(const gz::msgs::SphericalCoordinates &_msg)
{
  gz::math::SphericalCoordinates ret;
  Set(&ret, _msg);
  return ret;
}
}  // namespace
}  // namespace gz::msgs
#endif  // GZ_MSGS_CONVERT_VECTOR3_HH_
