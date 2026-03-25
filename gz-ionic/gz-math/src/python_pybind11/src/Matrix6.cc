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

#include <string>

#include "Matrix6.hh"

namespace gz
{
namespace math
{
namespace python
{
void defineMathMatrix6(py::module &_m, const std::string &_typestr)
{
  helpDefineMathMatrix6<double>(_m, _typestr + "d");
  py::enum_<Matrix6d::Matrix6Corner>(_m, (_typestr + "dCorner").c_str())
    .value("TOP_LEFT", Matrix6d::Matrix6Corner::TOP_LEFT)
    .value("TOP_RIGHT", Matrix6d::Matrix6Corner::TOP_RIGHT)
    .value("BOTTOM_LEFT", Matrix6d::Matrix6Corner::BOTTOM_LEFT)
    .value("BOTTOM_RIGHT", Matrix6d::Matrix6Corner::BOTTOM_RIGHT)
    .export_values();

  helpDefineMathMatrix6<float>(_m, _typestr + "f");
  py::enum_<Matrix6f::Matrix6Corner>(_m, (_typestr + "fCorner").c_str())
    .value("TOP_LEFT", Matrix6f::Matrix6Corner::TOP_LEFT)
    .value("TOP_RIGHT", Matrix6f::Matrix6Corner::TOP_RIGHT)
    .value("BOTTOM_LEFT", Matrix6f::Matrix6Corner::BOTTOM_LEFT)
    .value("BOTTOM_RIGHT", Matrix6f::Matrix6Corner::BOTTOM_RIGHT)
    .export_values();

  helpDefineMathMatrix6<int>(_m, _typestr + "i");
  py::enum_<Matrix6i::Matrix6Corner>(_m, (_typestr + "iCorner").c_str())
    .value("TOP_LEFT", Matrix6i::Matrix6Corner::TOP_LEFT)
    .value("TOP_RIGHT", Matrix6i::Matrix6Corner::TOP_RIGHT)
    .value("BOTTOM_LEFT", Matrix6i::Matrix6Corner::BOTTOM_LEFT)
    .value("BOTTOM_RIGHT", Matrix6i::Matrix6Corner::BOTTOM_RIGHT)
    .export_values();
}

}  // namespace python
}  // namespace math
}  // namespace gz
