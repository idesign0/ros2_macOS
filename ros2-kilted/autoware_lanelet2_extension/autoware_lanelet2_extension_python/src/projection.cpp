// Copyright 2023 Autoware Foundation. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Authors: Mamoru Sobue

// NOLINTBEGIN(readability-identifier-naming)

#include <autoware_lanelet2_extension/projection/mgrs_projector.hpp>
#include <autoware_lanelet2_extension/projection/transverse_mercator_projector.hpp>

#include <boost/python.hpp>

#include <lanelet2_io/Projection.h>

#include <memory>
#include <string>

BOOST_PYTHON_MODULE(_autoware_lanelet2_extension_python_boost_python_projection)
{
  namespace bp = boost::python;

  bp::class_<
    lanelet::projection::MGRSProjector, std::shared_ptr<lanelet::projection::MGRSProjector>,
    bp::bases<lanelet::Projector>>(
    "MGRSProjector", bp::init<lanelet::Origin>((bp::arg("origin") = lanelet::Origin({0.0, 0.0}))))
    .def(
      "setMGRSCode", (void (lanelet::projection::MGRSProjector::*)(const std::string &))(
                       &lanelet::projection::MGRSProjector::setMGRSCode))
    .def("isMGRSCodeSet", &lanelet::projection::MGRSProjector::isMGRSCodeSet)
    .def("getProjectedMGRSGrid", &lanelet::projection::MGRSProjector::getProjectedMGRSGrid);

  bp::class_<
    lanelet::projection::TransverseMercatorProjector,
    std::shared_ptr<lanelet::projection::TransverseMercatorProjector>,
    bp::bases<lanelet::Projector>>
    transverse_mercator_projector(
      "TransverseMercatorProjector", bp::init<lanelet::Origin>("origin"));
}

// NOLINTEND(readability-identifier-naming)
