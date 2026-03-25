/*
 * Copyright (C) 2024 Open Source Robotics Foundation
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

#include <sstream>
#include <string>

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>

#include "CoordinateVector3.hh"

#include <gz/math/Angle.hh>
#include <gz/math/CoordinateVector3.hh>
#include <gz/math/Vector3.hh>

namespace py = pybind11;
using namespace pybind11::literals;

namespace gz
{
namespace math
{
namespace python
{
void defineMathCoordinateVector3(py::module &m, const std::string &typestr)
{
  using Class = gz::math::CoordinateVector3;
  auto toString = [](const Class &si) {
    std::stringstream stream;
    stream << si;
    return stream.str();
  };
  std::string pyclass_name = typestr;
  py::class_<Class>(m,
                    pyclass_name.c_str(),
                    py::buffer_protocol(),
                    py::dynamic_attr())
    .def(py::init<>())
    .def(py::init<const Class&>())
    .def("metric",
         py::overload_cast<const gz::math::Vector3d &>(&Class::Metric),
        "Constructor for metric values.")
    .def("metric",
         py::overload_cast<double, double, double>(&Class::Metric),
        "Constructor for metric values.")
    .def("spherical", &Class::Spherical,
         "Constructor for spherical values.")
    .def("set_metric",
         py::overload_cast<const gz::math::Vector3d &>(&Class::SetMetric),
        "Set the metric contents of the vector")
    .def("set_metric",
         py::overload_cast<double, double, double>(&Class::SetMetric),
        "Set the metric contents of the vector")
    .def("set_spherical", &Class::SetSpherical,
         "Set the spherical contents of the vector")
    .def("is_metric", &Class::IsMetric, "Whether this vector is metric.")
    .def("is_spherical", &Class::IsSpherical,
         "Whether this vector is spherical.")
    .def("as_metric_vector", &Class::AsMetricVector,
         "Return this vector as a metric Vector3d (only valid for metric).")
    .def(py::self + py::self)
    .def(py::self += py::self)
    .def(py::self - py::self)
    .def(py::self -= py::self)
    .def(py::self != py::self)
    .def(py::self == py::self)
    .def(-py::self)
    .def("equal",
         py::overload_cast<
           const CoordinateVector3&,
           const double&,
           const Angle&
         >(&Class::Equal, py::const_), "Equality test with tolerance.")
    .def("equal",
         py::overload_cast<const Class&>(&Class::Equal, py::const_),
         "Equal to operator")
    .def("is_finite", &Class::IsFinite,
         "See if all vector components are finite (e.g., not nan)")
    .def("x", py::overload_cast<>(&Class::X, py::const_),
        "Get the x value of a metric vector.")
    .def("lat", py::overload_cast<>(&Class::Lat, py::const_),
        "Get the latitude of a spherical vector.")
    .def("y", py::overload_cast<>(&Class::Y, py::const_),
        "Get the y value of a metric vector.")
    .def("lon", py::overload_cast<>(&Class::Lon, py::const_),
        "Get the longitude of a spherical vector.")
    .def("z", py::overload_cast<>(&Class::Z, py::const_),
        "Get the z value of a metric vector.")
    .def("__copy__", [](const Class &self) {
      return Class(self);
    })
    .def("__deepcopy__", [](const Class &self, py::dict) {
      return Class(self);
    }, "memo"_a)
    .def("__str__", toString)
    .def("__repr__", toString);
}

}  // namespace python
}  // namespace math
}  // namespace gz
