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

#ifndef GZ_MATH_PYTHON__INTERVAL_HH_
#define GZ_MATH_PYTHON__INTERVAL_HH_

#include <string>
#include <sstream>

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>

#include <gz/math/Interval.hh>

namespace py = pybind11;
using namespace pybind11::literals;

namespace gz
{
namespace math
{
namespace python
{
/// Help define a pybind11 wrapper for a gz::math::Interval
/**
 * \param[in] module a pybind11 module to add the definition to
 * \param[in] typestr name of the type used by Python
 */
template<typename T>
void helpDefineMathInterval(py::module &m, const std::string &typestr)
{
  using Class = gz::math::Interval<T>;
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
    .def(py::init<T, bool, T, bool>())
    .def(py::self != py::self)
    .def(py::self == py::self)
    .def("open",
         &Class::Open,
         "Make an open interval (`_leftValue`, `_rightValue`)")
    .def("left_closed",
         &Class::LeftClosed,
         "Make a left-closed interval [`_leftValue`, `_rightValue`)")
    .def("right_closed",
         &Class::RightClosed,
         "Make a right-closed interval (`_leftValue`, `_rightValue`]")
    .def("closed",
         &Class::Closed,
         "Make a closed interval [`_leftValue`, `_rightValue`]")
    .def("left_value",
          &Class::LeftValue,
         "Get the leftmost interval value")
    .def("is_left_closed",
         &Class::IsLeftClosed,
         "Check if the interval is left-closed")
    .def("right_value",
          &Class::RightValue,
         "Get the rightmost interval value")
    .def("is_right_closed",
         &Class::IsRightClosed,
         "Check if the interval is right-closed")
    .def("empty",
         &Class::Empty,
         "Check if the interval is empty")
    .def("contains",
         py::overload_cast<const T &>(&Class::Contains, py::const_),
         "Check if the interval contains `_value`")
    .def("contains",
         py::overload_cast<const Class &>(&Class::Contains, py::const_),
         "Check if the interval contains `_other` interval")
    .def("intersects",
         &Class::Intersects,
         "Check if the interval intersects `_other` interval")
    .def("__copy__", [](const Class &self) {
      return Class(self);
    })
    .def("__deepcopy__", [](const Class &self, py::dict) {
      return Class(self);
    }, "memo"_a)
    .def("__str__", toString)
    .def("__repr__", toString);
}

/// Define a pybind11 wrapper for a gz::math::Interval
/**
 * \param[in] module a pybind11 module to add the definition to
 * \param[in] typestr name of the type used by Python
 */
void defineMathInterval(py::module &m, const std::string &typestr);

}  // namespace python
}  // namespace math
}  // namespace gz

#endif  // GZ_MATH_PYTHON__INTERVAL_HH_
