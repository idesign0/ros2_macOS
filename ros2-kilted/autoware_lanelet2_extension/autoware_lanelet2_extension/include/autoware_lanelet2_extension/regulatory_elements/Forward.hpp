// Copyright 2023 Tier IV, Inc.
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

#ifndef AUTOWARE_LANELET2_EXTENSION__REGULATORY_ELEMENTS__FORWARD_HPP_
#define AUTOWARE_LANELET2_EXTENSION__REGULATORY_ELEMENTS__FORWARD_HPP_

// NOLINTBEGIN(readability-identifier-naming)

#include <lanelet2_core/Attribute.h>
#include <lanelet2_core/primitives/BasicRegulatoryElements.h>

#include <memory>

namespace lanelet::autoware
{

inline namespace format_v2
{
class AutowareTrafficLight;
class Crosswalk;
class DetectionArea;
class NoParkingArea;
class NoStoppingArea;
class RoadMarking;
class SpeedBump;
class VirtualTrafficLight;
class BusStopArea;
}  // namespace format_v2

}  // namespace lanelet::autoware

namespace lanelet
{

inline namespace format_v2
{
using TrafficSignConstPtr = std::shared_ptr<const lanelet::TrafficSign>;
using TrafficLightConstPtr = std::shared_ptr<const lanelet::TrafficLight>;
using AutowareTrafficLightConstPtr = std::shared_ptr<const lanelet::autoware::AutowareTrafficLight>;
using DetectionAreaConstPtr = std::shared_ptr<const lanelet::autoware::DetectionArea>;
using NoParkingAreaConstPtr = std::shared_ptr<const lanelet::autoware::NoParkingArea>;
using NoStoppingAreaConstPtr = std::shared_ptr<const lanelet::autoware::NoStoppingArea>;
using NoParkingAreaConstPtr = std::shared_ptr<const lanelet::autoware::NoParkingArea>;
using SpeedBumpConstPtr = std::shared_ptr<const lanelet::autoware::SpeedBump>;
using CrosswalkConstPtr = std::shared_ptr<const lanelet::autoware::Crosswalk>;
using BusStopAreaConstPtr = std::shared_ptr<const lanelet::autoware::BusStopArea>;
}  // namespace format_v2

}  // namespace lanelet

// NOLINTEND(readability-identifier-naming)

#endif  // AUTOWARE_LANELET2_EXTENSION__REGULATORY_ELEMENTS__FORWARD_HPP_
