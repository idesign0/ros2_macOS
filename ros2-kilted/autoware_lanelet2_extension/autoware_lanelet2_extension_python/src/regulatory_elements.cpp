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

#include <autoware_lanelet2_extension/regulatory_elements/autoware_traffic_light.hpp>
#include <autoware_lanelet2_extension/regulatory_elements/crosswalk.hpp>
#include <autoware_lanelet2_extension/regulatory_elements/detection_area.hpp>
#include <autoware_lanelet2_extension/regulatory_elements/no_parking_area.hpp>
#include <autoware_lanelet2_extension/regulatory_elements/no_stopping_area.hpp>
#include <autoware_lanelet2_extension/regulatory_elements/road_marking.hpp>
#include <autoware_lanelet2_extension/regulatory_elements/speed_bump.hpp>
#include <autoware_lanelet2_extension/regulatory_elements/virtual_traffic_light.hpp>

#include <boost/python.hpp>

#include <memory>
#include <string>

namespace bp = boost::python;

// from lanelet2_python/src/core.cpp

void format_helper([[maybe_unused]] std::ostream & os)
{
}

template <typename... Args>
void format_helper(std::ostream & os, const std::string & s, const Args &... Args_)
{
  if (!s.empty()) {
    os << ", ";
  }
  os << s;
  format_helper(os, Args_...);
}

template <typename T, typename... Args>
void format_helper(std::ostream & os, const T & next, const Args &... Args_)
{
  os << ", ";
  os << next;
  format_helper(os, Args_...);
}

template <typename T, typename... Args>
void format(std::ostream & os, const T & first, const Args &... Args_)
{
  os << first;
  format_helper(os, Args_...);
}

template <typename... Args>
std::string make_repr(const char * name, const Args &... Args_)
{
  std::ostringstream os;
  os << name << '(';
  format(os, Args_...);
  os << ')';
  return os.str();
}

std::string repr(const bp::object & o)
{
  bp::object repr = bp::import("builtins").attr("repr");
  return bp::call<std::string>(repr.ptr(), o);
}

std::string repr(const lanelet::AttributeMap & a)
{
  if (a.empty()) {
    return {};
  }
  return repr(bp::object(a));
}

std::string repr(const lanelet::RegulatoryElementConstPtrs & regulatory_elements)
{
  if (regulatory_elements.empty()) {
    return {};
  }
  return repr(bp::list(regulatory_elements));
}

BOOST_PYTHON_MODULE(_autoware_lanelet2_extension_python_boost_python_regulatory_elements)
{
  // autoware_traffic_light
  bp::class_<
    lanelet::autoware::AutowareTrafficLight, boost::noncopyable,
    std::shared_ptr<lanelet::autoware::AutowareTrafficLight>, bp::bases<lanelet::TrafficLight>>(
    "AutowareTrafficLight", "autoware traffic light regulatory element", bp::no_init)
    .def(
      "__init__", bp::make_constructor(
                    &lanelet::autoware::AutowareTrafficLight::make, bp::default_call_policies(),
                    (bp::arg("Id"), bp::arg("attributes"), bp::arg("trafficLights"),
                     bp::arg("stopline") = lanelet::Optional<lanelet::LineString3d>(),
                     bp::arg("lightBulbs") = lanelet::LineString3d({}))))
    .def(
      "lightBulbs",
      +[](lanelet::autoware::AutowareTrafficLight & self) { return self.lightBulbs(); })
    .def("addLightBulbs", &lanelet::autoware::AutowareTrafficLight::addLightBulbs)
    .def("removeLightBulbs", &lanelet::autoware::AutowareTrafficLight::removeLightBulbs)
    .def(
      "__repr__", +[](lanelet::autoware::AutowareTrafficLight & r) {
        return make_repr(
          "AutowareTrafficLight", r.id(), repr(bp::dict(r.constData()->parameters)),
          repr(r.attributes()));
      });
  bp::implicitly_convertible<
    std::shared_ptr<lanelet::autoware::AutowareTrafficLight>, lanelet::RegulatoryElementPtr>();

  // crosswalk
  bp::class_<
    lanelet::autoware::Crosswalk, boost::noncopyable, std::shared_ptr<lanelet::autoware::Crosswalk>,
    bp::bases<lanelet::RegulatoryElement>>(
    "Crosswalk", "autoware crosswalk regulatory element", bp::no_init)
    .def(
      "__init__", bp::make_constructor(
                    &lanelet::autoware::Crosswalk::make, bp::default_call_policies(),
                    (bp::arg("Id"), bp::arg("attributes"), bp::arg("crosswalk_lanelet"),
                     bp::arg("crosswalk_area"), bp::arg("stop_line"))))
    .def(
      "crosswalkAreas", +[](lanelet::autoware::Crosswalk & self) { return self.crosswalkAreas(); })
    .def(
      "stopLines", +[](lanelet::autoware::Crosswalk & self) { return self.stopLines(); })
    .def(
      "crosswalkLanelet",
      +[](lanelet::autoware::Crosswalk & self) { return self.crosswalkLanelet(); })
    .def("addCrosswalkArea", &lanelet::autoware::Crosswalk::addCrosswalkArea)
    .def("removeCrosswalkArea", &lanelet::autoware::Crosswalk::removeCrosswalkArea)
    .def(
      "__repr__", +[](lanelet::autoware::Crosswalk & r) {
        return make_repr(
          "Crosswalk", r.id(), repr(bp::dict(r.constData()->parameters)), repr(r.attributes()));
      });
  bp::implicitly_convertible<
    std::shared_ptr<lanelet::autoware::Crosswalk>, lanelet::RegulatoryElementPtr>();

  // detection_area
  bp::class_<
    lanelet::autoware::DetectionArea, boost::noncopyable,
    std::shared_ptr<lanelet::autoware::DetectionArea>, bp::bases<lanelet::RegulatoryElement>>(
    "DetectionArea", "detection_area regulatory element", bp::no_init)
    .def(
      "__init__",
      bp::make_constructor(
        &lanelet::autoware::DetectionArea::make, bp::default_call_policies(),
        (bp::arg("Id"), bp::arg("attributes"), bp::arg("detectionAreas"), bp::arg("stopLine"))))
    .def(
      "detectionAreas",
      +[](lanelet::autoware::DetectionArea & self) { return self.detectionAreas(); })
    .def("addDetectionArea", &lanelet::autoware::DetectionArea::addDetectionArea)
    .def("removeDetectionArea", &lanelet::autoware::DetectionArea::removeDetectionArea)
    .def(
      "stopLine", +[](lanelet::autoware::DetectionArea & self) { return self.stopLine(); })
    .def("setStopLine", &lanelet::autoware::DetectionArea::setStopLine)
    .def("removeStopLine", &lanelet::autoware::DetectionArea::removeStopLine)
    .def(
      "__repr__", +[](lanelet::autoware::DetectionArea & r) {
        return make_repr(
          "DetectionArea", r.id(), repr(bp::dict(r.constData()->parameters)), repr(r.attributes()));
      });
  bp::implicitly_convertible<
    std::shared_ptr<lanelet::autoware::DetectionArea>, lanelet::RegulatoryElementPtr>();

  // no_parking_area
  bp::class_<
    lanelet::autoware::NoParkingArea, boost::noncopyable,
    std::shared_ptr<lanelet::autoware::NoParkingArea>, bp::bases<lanelet::RegulatoryElement>>(
    "NoParkingArea", "no_parking_area regulatory element", bp::no_init)
    .def(
      "__init__", bp::make_constructor(
                    &lanelet::autoware::NoParkingArea::make, bp::default_call_policies(),
                    (bp::arg("Id"), bp::arg("attributes"), bp::arg("no_parking_areas"))))
    .def(
      "noParkingAreas",
      +[](lanelet::autoware::NoParkingArea & self) { return self.noParkingAreas(); })
    .def("addNoParkingArea", &lanelet::autoware::NoParkingArea::addNoParkingArea)
    .def("removeNoParkingArea", &lanelet::autoware::NoParkingArea::removeNoParkingArea)
    .def(
      "__repr__", +[](lanelet::autoware::NoParkingArea & r) {
        return make_repr(
          "NoParkingArea", r.id(), repr(bp::dict(r.constData()->parameters)), repr(r.attributes()));
      });
  bp::implicitly_convertible<
    std::shared_ptr<lanelet::autoware::NoParkingArea>, lanelet::RegulatoryElementPtr>();

  // no_stopping_area
  bp::class_<
    lanelet::autoware::NoStoppingArea, boost::noncopyable,
    std::shared_ptr<lanelet::autoware::NoStoppingArea>, bp::bases<lanelet::RegulatoryElement>>(
    "NoStoppingArea", "no_stopping_area regulatory element", bp::no_init)
    .def(
      "__init__", bp::make_constructor(
                    &lanelet::autoware::NoStoppingArea::make, bp::default_call_policies(),
                    (bp::arg("Id"), bp::arg("attributes"), bp::arg("no_stopping_areas"),
                     bp::arg("stopLine") = lanelet::Optional<lanelet::LineString3d>())))
    .def(
      "noStoppingAreas",
      +[](lanelet::autoware::NoStoppingArea & self) { return self.noStoppingAreas(); })
    .def("addNoStoppingArea", &lanelet::autoware::NoStoppingArea::addNoStoppingArea)
    .def("removeNoStoppingArea", &lanelet::autoware::NoStoppingArea::removeNoStoppingArea)
    .def(
      "stopLine", +[](lanelet::autoware::NoStoppingArea & self) { return self.stopLine(); })
    .def("setStopLine", &lanelet::autoware::NoStoppingArea::setStopLine)
    .def("removeStopLine", &lanelet::autoware::NoStoppingArea::removeStopLine)
    .def(
      "__repr__", +[](lanelet::autoware::NoParkingArea & r) {
        return make_repr(
          "NoParkingArea", r.id(), repr(bp::dict(r.constData()->parameters)), repr(r.attributes()));
      });
  bp::implicitly_convertible<
    std::shared_ptr<lanelet::autoware::NoStoppingArea>, lanelet::RegulatoryElementPtr>();

  // road_marking
  bp::class_<
    lanelet::autoware::RoadMarking, boost::noncopyable,
    std::shared_ptr<lanelet::autoware::RoadMarking>, bp::bases<lanelet::RegulatoryElement>>(
    "RoadMarking", "road_marking regulatory element", bp::no_init)
    .def(
      "__init__", bp::make_constructor(
                    &lanelet::autoware::RoadMarking::make, bp::default_call_policies(),
                    (bp::arg("Id"), bp::arg("attributes"), bp::arg("road_marking"))))
    .def(
      "roadMarking", +[](lanelet::autoware::RoadMarking & self) { return self.roadMarking(); })
    .def("setRoadMarking", &lanelet::autoware::RoadMarking::setRoadMarking)
    .def("removeRoadMarking", &lanelet::autoware::RoadMarking::removeRoadMarking)
    .def(
      "__repr__", +[](lanelet::autoware::RoadMarking & r) {
        return make_repr(
          "RoadMarking", r.id(), repr(bp::dict(r.constData()->parameters)), repr(r.attributes()));
      });
  bp::implicitly_convertible<
    std::shared_ptr<lanelet::autoware::RoadMarking>, lanelet::RegulatoryElementPtr>();

  // speed_bump
  bp::class_<
    lanelet::autoware::SpeedBump, boost::noncopyable, std::shared_ptr<lanelet::autoware::SpeedBump>,
    bp::bases<lanelet::RegulatoryElement>>(
    "SpeedBump", "speed_bump regulatory element", bp::no_init)
    .def(
      "__init__", bp::make_constructor(
                    &lanelet::autoware::SpeedBump::make, bp::default_call_policies(),
                    (bp::arg("Id"), bp::arg("attributes"), bp::arg("speed_bump"))))
    .def(
      "speedBump", +[](lanelet::autoware::SpeedBump & self) { return self.speedBump(); })
    .def("addSpeedBump", &lanelet::autoware::SpeedBump::addSpeedBump)
    .def("removeSpeedBump", &lanelet::autoware::SpeedBump::removeSpeedBump)
    .def(
      "__repr__", +[](lanelet::autoware::SpeedBump & r) {
        return make_repr(
          "SpeedBump", r.id(), repr(bp::dict(r.constData()->parameters)), repr(r.attributes()));
      });
  bp::implicitly_convertible<
    std::shared_ptr<lanelet::autoware::SpeedBump>, lanelet::RegulatoryElementPtr>();

  // virtual_traffic_light
  bp::class_<
    lanelet::autoware::VirtualTrafficLight, boost::noncopyable,
    std::shared_ptr<lanelet::autoware::VirtualTrafficLight>, bp::bases<lanelet::RegulatoryElement>>(
    "VirtualTrafficLight", "virtual_traffic_light regulatory element", bp::no_init)
    .def(
      "__init__", bp::make_constructor(
                    &lanelet::autoware::VirtualTrafficLight::make, bp::default_call_policies(),
                    (bp::arg("Id"), bp::arg("attributes"), bp::arg("virtual_traffic_light"))))
    .def(
      "getVirtualTrafficLight",
      +[](lanelet::autoware::VirtualTrafficLight & self) { return self.getVirtualTrafficLight(); })
    .def(
      "getStopLine",
      +[](lanelet::autoware::VirtualTrafficLight & self) { return self.getStopLine(); })
    .def(
      "getStartLine",
      +[](lanelet::autoware::VirtualTrafficLight & self) { return self.getStartLine(); })
    .def(
      "getEndLines",
      +[](lanelet::autoware::VirtualTrafficLight & self) { return self.getEndLines(); })
    .def(
      "__repr__", +[](lanelet::autoware::VirtualTrafficLight & r) {
        return make_repr(
          "VirtualTrafficLight", r.id(), repr(bp::dict(r.constData()->parameters)),
          repr(r.attributes()));
      });
  bp::implicitly_convertible<
    std::shared_ptr<lanelet::autoware::VirtualTrafficLight>, lanelet::RegulatoryElementPtr>();

  bp::register_ptr_to_python<lanelet::TrafficSignConstPtr>();
  bp::register_ptr_to_python<lanelet::TrafficLightConstPtr>();
  bp::register_ptr_to_python<lanelet::AutowareTrafficLightConstPtr>();
  bp::register_ptr_to_python<lanelet::DetectionAreaConstPtr>();
  bp::register_ptr_to_python<lanelet::NoParkingAreaConstPtr>();
  bp::register_ptr_to_python<lanelet::NoStoppingAreaConstPtr>();
  bp::register_ptr_to_python<lanelet::SpeedBumpConstPtr>();
  bp::register_ptr_to_python<lanelet::CrosswalkConstPtr>();
}

// NOLINTEND(readability-identifier-naming)
