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

#ifndef GZ_MATH_PYTHON__POLYNOMIAL3_HH_
#define GZ_MATH_PYTHON__POLYNOMIAL3_HH_

#include <string>
#include <sstream>

#include <pybind11/pybind11.h>

#include <gz/math/Polynomial3.hh>
#include <gz/math/Vector4.hh>

namespace py = pybind11;
using namespace pybind11::literals;

namespace gz
{
namespace math
{
namespace python
{
/// Help define a pybind11 wrapper for an gz::math::Polynomial3
/**
 * \param[in] module a pybind11 module to add the definition to
 * \param[in] typestr name of the type used by Python
 */
template<typename T>
void helpDefineMathPolynomial3(py::module &m, const std::string &typestr)
{
  using Class = gz::math::Polynomial3<T>;
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
    .def(py::init<Vector4<T>>())
    .def("constant",
         &Class::Constant,
         "Make a constant polynomial")
    .def("coeffs",
         &Class::Coeffs,
         "Get the polynomial coefficients")
    .def("evaluate",
         &Class::Evaluate,
         "Evaluate the polynomial at `_x`. For non-finite `_x`, this function "
         "computes p(z) as z tends to `_x`.")
    .def("minimum",
         [](const Class &self, const Interval<T> &_interval, T &_xMin)
         {
           T result = self.Minimum(_interval, _xMin);
           return std::make_tuple(result, _xMin);
         },
         "Compute polynomial minimum in an `_interval`")
    .def("minimum",
         py::overload_cast<const Interval<T> &>(&Class::Minimum, py::const_),
         "Compute polynomial minimum in an `_interval`")
    .def("minimum",
         [](const Class &self, T &_xMin)
         {
           T result = self.Minimum(_xMin);
           return std::make_tuple(result, _xMin);
         },
         "Compute polynomial minimum in an `_interval`")
    .def("minimum",
         py::overload_cast<>(&Class::Minimum, py::const_),
         "Compute polynomial minimum in an `_interval`")
    .def("__call__", &Class::operator())
    .def("__copy__", [](const Class &self) {
      return Class(self);
    })
    .def("__deepcopy__", [](const Class &self, py::dict) {
      return Class(self);
    }, "memo"_a)
    .def("__str__", toString)
    .def("__repr__", toString);
}

/// Define a pybind11 wrapper for an gz::math::Polynomial3
/**
 * \param[in] module a pybind11 module to add the definition to
 * \param[in] typestr name of the type used by Python
 */
void defineMathPolynomial3(py::module &m, const std::string &typestr);

}  // namespace python
}  // namespace math
}  // namespace gz

#endif  // GZ_MATH_PYTHON__POLYNOMIAL3_HH_
