/*
 * Copyright (C) 2021 Open Source Robotics Foundation
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
#include <pybind11/operators.h>
#include <pybind11/stl.h>

#include "SphericalCoordinates.hh"
#include <gz/math/Angle.hh>
#include <gz/math/SphericalCoordinates.hh>

namespace gz
{
namespace math
{
namespace python
{
void defineMathSphericalCoordinates(py::module &m, const std::string &typestr)
{
  using Class = gz::math::SphericalCoordinates;
  using CoordinateVector3 = gz::math::CoordinateVector3;
  using CoordinateType = gz::math::SphericalCoordinates::CoordinateType;
  std::string pyclass_name = typestr;

  py::class_<Class> sphericalCoordinates(m,
                    pyclass_name.c_str(),
                    py::buffer_protocol(),
                    py::dynamic_attr());
  sphericalCoordinates
    .def(py::init<>())
    .def(py::init<const Class&>())
    .def(py::init<const Class::SurfaceType>())
    .def(py::init<const Class::SurfaceType, const gz::math::Angle &,
                  const gz::math::Angle &, const double,
                  const gz::math::Angle &>())
    .def(py::init<const Class::SurfaceType, const double,
                  const double>())
    .def(py::self != py::self)
    .def(py::self == py::self)
    .def("spherical_from_local_position",
         py::overload_cast<const CoordinateVector3&>(
           &Class::SphericalFromLocalPosition, py::const_),
         "Convert a Cartesian position vector to geodetic coordinates.")
    .def("spherical_from_local_position",
         [](const Class &self, const gz::math::Vector3d &_xyz)
         {
           PyErr_WarnEx(
             PyExc_DeprecationWarning,
             "Passing Vector3d to spherical_from_local_position() is deprecated"
             " and will be removed. Pass CoordinateVector3 instead and arrange "
             "for the different behavior. See Migration.md ",
             1);
           GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION
           return self.SphericalFromLocalPosition(_xyz);
           GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION
         },
         "DEPRECATED: Passing Vector3d is deprecated and will be removed. "
         "Pass CoordinateVector3 instead and arrange for the different "
         "behavior. See Migration.md .")
    .def("global_from_local_velocity",
         py::overload_cast<const CoordinateVector3&>(
           &Class::GlobalFromLocalVelocity, py::const_),
         "Convert a Cartesian velocity vector in the local frame "
         " to a global Cartesian frame with components East, North, Up")
    .def("global_from_local_velocity",
         [](const Class &self, const gz::math::Vector3d &_xyz)
         {
           PyErr_WarnEx(
             PyExc_DeprecationWarning,
             "Passing Vector3d to global_from_local_velocity() is deprecated"
             " and will be removed. Pass CoordinateVector3 instead and arrange "
             "for the different behavior. See Migration.md ",
             1);
           GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION
           return self.GlobalFromLocalVelocity(_xyz);
           GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION
         },
         "DEPRECATED: Passing Vector3d is deprecated and will be removed. "
         "Pass CoordinateVector3 instead and arrange for the different "
         "behavior. See Migration.md .")
    .def("convert",
         py::overload_cast<const std::string &>(&Class::Convert),
         "Convert a string to a SurfaceType.")
    .def("convert",
         py::overload_cast<Class::SurfaceType>(&Class::Convert),
         "Convert a SurfaceType to a string.")
    .def("distance_WGS84",
         &Class::DistanceWGS84,
         "Get the distance between two points expressed in geographic "
         "latitude and longitude. It assumes that both points are at sea level."
         " Example: _latA = 38.0016667 and _lonA = -123.0016667) represents "
         "the point with latitude 38d 0'6.00\"N and longitude 123d 0'6.00\"W."
         " This function assumes the surface is EARTH_WGS84.")
    .def("distance_between_points",
         &Class::DistanceBetweenPoints,
         "Get the distance between two points expressed in geographic "
         "latitude and longitude. It assumes that both points are at sea level."
         " Example: _latA = 38.0016667 and _lonA = -123.0016667) represents "
         "the point with latitude 38d 0'6.00\"N and longitude 123d 0'6.00\"W.")
    .def("surface",
         &Class::Surface,
         "Get SurfaceType currently in use.")
    .def("surface_radius",
         &Class::SurfaceRadius,
         "Get the radius of the surface.")
    .def("surface_axis_equatorial",
         &Class::SurfaceAxisEquatorial,
         "Get the major of the surface.")
    .def("surface_axis_polar",
         &Class::SurfaceAxisPolar,
         "Get the minor axis of the surface.")
    .def("surface_flattening",
         &Class::SurfaceFlattening,
         "Get the flattening parameter of the surface.")
    .def("latitude_reference",
         &Class::LatitudeReference,
         "Get reference geodetic latitude.")
    .def("longitude_reference",
         &Class::LongitudeReference,
         "Get reference longitude.")
    .def("elevation_reference",
         &Class::ElevationReference,
         "Get reference elevation in meters.")
    .def("heading_offset",
         &Class::HeadingOffset,
         "Get heading offset for the reference frame, expressed as "
         "angle from East to x-axis, or equivalently "
         "from North to y-axis.")
    .def("set_surface",
         py::overload_cast<const Class::SurfaceType&>(&Class::SetSurface),
         "Set SurfaceType for planetary surface model.")
    .def("set_surface",
         py::overload_cast<const Class::SurfaceType&,
         const double, const double
         >(&Class::SetSurface),
         "Set SurfaceType for planetary surface model.")
    .def("set_latitude_reference",
         &Class::SetLatitudeReference,
         "Set reference geodetic latitude.")
    .def("set_longitude_reference",
         &Class::SetLongitudeReference,
         "Set reference longitude.")
    .def("set_elevation_reference",
         &Class::SetElevationReference,
         "Set reference elevation above sea level in meters.")
    .def("set_heading_offset",
         &Class::SetHeadingOffset,
         "Set heading angle offset for the frame.")
    .def("local_from_spherical_position",
         py::overload_cast<const CoordinateVector3&>(
           &Class::LocalFromSphericalPosition, py::const_),
         "Convert a geodetic position vector to Cartesian coordinates.")
    .def("local_from_spherical_position",
         [](const Class &self, const gz::math::Vector3d &_xyz)
         {
           PyErr_WarnEx(
             PyExc_DeprecationWarning,
             "Passing Vector3d to local_from_spherical_position() is deprecated"
             " and will be removed. Pass CoordinateVector3 instead.",
             1);
           GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION
           return self.LocalFromSphericalPosition(_xyz);
           GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION
         },
         "DEPRECATED: Passing Vector3d is deprecated and will be removed. "
         "Pass CoordinateVector3 instead.")
    .def("local_from_global_velocity",
         py::overload_cast<const CoordinateVector3&>(
           &Class::LocalFromGlobalVelocity, py::const_),
         "Convert a Cartesian velocity vector with components East, "
         "North, Up to a local cartesian frame vector XYZ.")
    .def("local_from_global_velocity",
         [](const Class &self, const gz::math::Vector3d &_xyz)
         {
           PyErr_WarnEx(
             PyExc_DeprecationWarning,
             "Passing Vector3d to local_from_global_velocity() is deprecated"
             " and will be removed. Pass CoordinateVector3 instead.",
             1);
           GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION
           return self.LocalFromGlobalVelocity(_xyz);
           GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION
         },
         "DEPRECATED: Passing Vector3d is deprecated and will be removed. "
         "Pass CoordinateVector3 instead.")
    .def("update_transformation_matrix",
         &Class::UpdateTransformationMatrix,
         "Update coordinate transformation matrix with reference location")
    .def("position_transform",
         py::overload_cast<
           const CoordinateVector3&,
           const CoordinateType&,
           const CoordinateType&
           >(&Class::PositionTransform, py::const_),
         "Convert between velocity in SPHERICAL/ECEF/LOCAL/GLOBAL frame "
         "Spherical coordinates use radians, while the other frames use "
         "meters.")
    .def("position_transform",
         [](const Class &self,
            const gz::math::Vector3d &_pos,
            const CoordinateType& _in,
            const CoordinateType& _out) -> gz::math::Vector3d
         {
           PyErr_WarnEx(
             PyExc_DeprecationWarning,
             "Passing Vector3d to position_transform() is deprecated and will "
             "be removed. Pass CoordinateVector3 instead and arrange for the "
             "different behavior. See Migration.md .",
             1);
           GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION
           return self.PositionTransform(_pos, _in, _out);
           GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION
         },
         "DEPRECATED: Passing Vector3d is deprecated and will be removed. "
         "Pass CoordinateVector3 instead and arrange for the different "
         "behavior. See Migration.md .")
    .def("velocity_transform",
         py::overload_cast<
           const CoordinateVector3&,
           const CoordinateType&,
           const CoordinateType&
         >(&Class::VelocityTransform, py::const_),
         "Convert between velocity in SPHERICAL/ECEF/LOCAL/GLOBAL frame "
         "Spherical coordinates use radians, while the other frames use "
         "meters.")
    .def("velocity_transform",
         [](const Class &self,
            const gz::math::Vector3d &_vel,
            const CoordinateType& _in,
            const CoordinateType& _out) -> gz::math::Vector3d
         {
           PyErr_WarnEx(
             PyExc_DeprecationWarning,
             "Passing Vector3d to velocity_transform() is deprecated and will "
             "be removed. Pass CoordinateVector3 instead and arrange for the "
             "different behavior. See Migration.md .",
             1);
           GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION
           return self.VelocityTransform(_vel, _in, _out);
           GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION
         },
         "DEPRECATED: Passing Vector3d is deprecated and will be removed. "
         "Pass CoordinateVector3 instead and arrange for the different "
         "behavior. See Migration.md .");

   GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION
   py::enum_<Class::CoordinateType>(sphericalCoordinates, "CoordinateType")
       .value("SPHERICAL", Class::CoordinateType::SPHERICAL)
       .value("ECEF", Class::CoordinateType::ECEF)
       .value("GLOBAL", Class::CoordinateType::GLOBAL)
       .value("LOCAL", Class::CoordinateType::LOCAL)
       .value("LOCAL2", Class::CoordinateType::LOCAL2)
       .export_values();
   GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION

   py::enum_<Class::SurfaceType>(sphericalCoordinates, "SurfaceType")
       .value("EARTH_WGS84", Class::SurfaceType::EARTH_WGS84)
       .value("MOON_SCS", Class::SurfaceType::MOON_SCS)
       .value("CUSTOM_SURFACE", Class::SurfaceType::CUSTOM_SURFACE)
       .export_values();
}
}  // namespace python
}  // namespace math
}  // namespace gz
