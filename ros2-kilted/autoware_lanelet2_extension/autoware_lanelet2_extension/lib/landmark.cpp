// Copyright 2023 Autoware Foundation
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

#include "autoware_lanelet2_extension/localization/landmark.hpp"

#include <Eigen/Core>

#include <boost/archive/binary_iarchive.hpp>

#include <lanelet2_core/LaneletMap.h>
#include <lanelet2_io/io_handlers/Serialize.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace impl
{
static void fromBinMsg(
  const autoware_map_msgs::msg::LaneletMapBin & msg, lanelet::LaneletMapPtr map)
{
  if (!map) {
    std::cerr << __FUNCTION__ << ": map is null pointer!";
    return;
  }

  std::string data_str;
  data_str.assign(msg.data.begin(), msg.data.end());

  std::stringstream ss;
  ss << data_str;
  boost::archive::binary_iarchive oa(ss);
  oa >> *map;
  lanelet::Id id_counter = 0;
  oa >> id_counter;
  lanelet::utils::registerId(id_counter);
}
}  // namespace impl

namespace lanelet::localization
{

std::vector<lanelet::Polygon3d> parseLandmarkPolygons(
  const autoware_map_msgs::msg::LaneletMapBin::ConstSharedPtr & msg,
  const std::string & target_subtype)
{
  lanelet::LaneletMapPtr lanelet_map_ptr{std::make_shared<lanelet::LaneletMap>()};
  ::impl::fromBinMsg(*msg, lanelet_map_ptr);

  std::vector<lanelet::Polygon3d> landmarks;

  for (const auto & poly : lanelet_map_ptr->polygonLayer) {
    const std::string type{poly.attributeOr(lanelet::AttributeName::Type, "none")};
    if (type != "pose_marker") {
      continue;
    }
    const std::string subtype{poly.attributeOr(lanelet::AttributeName::Subtype, "none")};
    if (subtype != target_subtype) {
      continue;
    }
    landmarks.push_back(poly);
  }

  return landmarks;
}

}  // namespace lanelet::localization
