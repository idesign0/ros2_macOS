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

#ifndef GZ_MATH_PYTHON__MATRIX6_HH_
#define GZ_MATH_PYTHON__MATRIX6_HH_

#include <sstream>
#include <string>

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>

#include <gz/math/Matrix6.hh>

namespace py = pybind11;
using namespace pybind11::literals;

namespace gz
{
namespace math
{
namespace python
{
/// Define a pybind11 wrapper for an Matrix6
/**
 * \param[in] _m a pybind11 module to add the definition to
 * \param[in] _typestr name of the type used by Python
 */
void defineMathMatrix6(py::module &_m, const std::string &_typestr);

/// Define a pybind11 wrapper for an Matrix6
/**
 * \param[in] _m a pybind11 module to add the definition to
 * \param[in] _typestr name of the type used by Python
 */
template<typename T>
void helpDefineMathMatrix6(py::module &_m, const std::string &_typestr)
{
  using Class = Matrix6<T>;
  auto toString = [](const Class &_si) {
    std::stringstream stream;
    stream << _si;
    return stream.str();
  };
  py::class_<Class>(_m,
                    _typestr.c_str(),
                    py::buffer_protocol(),
                    py::dynamic_attr())
    .def(py::init<>())
    .def(py::init<const Class&>())
    .def(py::init<T, T, T, T, T, T,
                  T, T, T, T, T, T,
                  T, T, T, T, T, T,
                  T, T, T, T, T, T,
                  T, T, T, T, T, T,
                  T, T, T, T, T, T>())
    .def(py::self * py::self)
    .def(py::self + py::self)
    .def(py::self == py::self)
    .def(py::self != py::self)
    .def("__call__",
         py::overload_cast<size_t, size_t>(&Class::operator()),
         py::return_value_policy::reference_internal)
    .def("set",
         &Class::Set,
         "Set values")
    .def("set_value",
         &Class::SetValue,
         "Set value in a specific row and col.")
    .def("equal",
         &Class::Equal,
         "Equality test operator")
    .def("submatrix",
         &Class::Submatrix,
         "One of the four 3x3 submatrices that compose this matrix.")
    .def("set_submatrix",
         &Class::SetSubmatrix,
         "Set one of the four 3x3 submatrices that compose this matrix.")
    .def("__copy__", [](const Class &_self) {
      return Class(_self);
    })
    .def("__deepcopy__", [](const Class &_self, py::dict) {
      return Class(_self);
    }, "memo"_a)
    .def_readonly_static("IDENTITY", &Class::Identity, "Identity matrix")
    .def_readonly_static("ZERO", &Class::Zero, "Zero matrix")
    .def("__str__", toString)
    .def("__repr__", toString);
}
}  // namespace python
}  // namespace math
}  // namespace gz

#endif  // GZ_MATH_PYTHON__MATRIX6_HH_
