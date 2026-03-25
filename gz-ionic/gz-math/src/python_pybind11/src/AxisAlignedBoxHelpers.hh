/*
 * Copyright (C) 2025 Open Source Robotics Foundation
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

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>

#include <string>
#include <gz/math/Box.hh>
#include <gz/math/Sphere.hh>
#include <gz/math/Capsule.hh>
#include <gz/math/Cylinder.hh>
#include <gz/math/AxisAlignedBox.hh>
#include <gz/math/AxisAlignedBoxHelpers.hh>

namespace py = pybind11;
using namespace gz::math;

namespace gz
{
namespace math
{
namespace python
{
/// Define a pybind11 wrapper for a gz::math::AxisAlignedBoxHelpers
/**
 * \param[in] module a pybind11 module to add the definition to
 * \param[in] typestr name of the type used by Python
 */
template<typename T>
void defineMathAxisAlignedBoxHelpers(
    py::module &m,
    const std::string &typestr)
{
    using Class = gz::math::AxisAlignedBoxHelpers<T>;
    std::string pyclass_name = typestr;
    py::class_<Class>(m,
                pyclass_name.c_str(),
                py::buffer_protocol(),
                py::dynamic_attr())
    .def("ConvertToAxisAlignedBox",
        [](const py::object &shape) -> AxisAlignedBox {
            if (py::isinstance<Box<T>>(shape)) {
                return AxisAlignedBoxHelpers<T>::ConvertToAxisAlignedBox(
                    shape.cast<Box<T>>());
            } else if (py::isinstance<Sphere<T>>(shape)) {
                return AxisAlignedBoxHelpers<T>::ConvertToAxisAlignedBox(
                    shape.cast<Sphere<T>>());
            } else if (py::isinstance<Capsule<T>>(shape)) {
                return AxisAlignedBoxHelpers<T>::ConvertToAxisAlignedBox(
                    shape.cast<Capsule<T>>());
            } else if (py::isinstance<Cylinder<T>>(shape)) {
                return AxisAlignedBoxHelpers<T>::ConvertToAxisAlignedBox(
                    shape.cast<Cylinder<T>>());
            } else {
                throw std::runtime_error("Unsupported shape type");
            }
        },
        "Convert a shape to an AxisAlignedBox");
}
}  // namespace python
}  // namespace math
}  // namespace gz
