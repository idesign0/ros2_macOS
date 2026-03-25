/*
 * Copyright (C) 2022 Open Source Robotics Foundation
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

#ifndef GZ_MATH_PYTHON__REGION3_HH_
#define GZ_MATH_PYTHON__REGION3_HH_

#include <string>
#include <sstream>

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>

#include <gz/math/Region3.hh>
#include <gz/math/Vector3.hh>

namespace py = pybind11;
using namespace pybind11::literals;

namespace gz
{
namespace math
{
namespace python
{
/// Help define a pybind11 wrapper for an gz::math::Region3
/**
 * \param[in] module a pybind11 module to add the definition to
 * \param[in] typestr name of the type used by Python
 */
template<typename T>
void helpDefineMathRegion3(py::module &m, const std::string &typestr)
{
  using Class = gz::math::Region3<T>;
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
    .def(py::init<Interval<T>, Interval<T>, Interval<T>>())
    .def(py::self != py::self)
    .def(py::self == py::self)
    .def("open",
         &Class::Open,
         "Make an open region")
    .def("closed",
         &Class::Closed,
         "Make a closed region")
    .def("ix",
          &Class::Ix,
         "Get the x-axis interval for the region")
    .def("iy",
         &Class::Iy,
         "Get the y-axis interval for the region")
    .def("iz",
          &Class::Iz,
         "Get the z-axis interval for the region")
    .def("empty",
         &Class::Empty,
         "Check if the interval is empty")
    .def("contains",
         py::overload_cast<const Vector3<T> &>(&Class::Contains, py::const_),
         "Check if the region contains `_point`")
    .def("contains",
         py::overload_cast<const Class &>(&Class::Contains, py::const_),
         "Check if the region contains `_other` region")
    .def("intersects",
         &Class::Intersects,
         "Check if the region intersects `_other` region")
    .def("__copy__", [](const Class &self) {
      return Class(self);
    })
    .def("__deepcopy__", [](const Class &self, py::dict) {
      return Class(self);
    }, "memo"_a)
    .def("__str__", toString)
    .def("__repr__", toString);
}

/// Define a pybind11 wrapper for an gz::math::Region3
/**
 * \param[in] module a pybind11 module to add the definition to
 * \param[in] typestr name of the type used by Python
 */
void defineMathRegion3(py::module &m, const std::string &typestr);

}  // namespace python
}  // namespace math
}  // namespace gz

#endif  // GZ_MATH_PYTHON__REGION3_HH_
