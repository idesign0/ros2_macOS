// Copyright 2015-2019 Autoware Foundation. All rights reserved.
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
// Authors: Ryohsuke Mitsudome

// NOLINTBEGIN(readability-identifier-naming)

#include "autoware_lanelet2_extension/io/autoware_osm_parser.hpp"

#include <lanelet2_core/geometry/LineString.h>
#include <lanelet2_io/io_handlers/Factory.h>
#include <lanelet2_io/io_handlers/OsmFile.h>
#include <lanelet2_io/io_handlers/OsmHandler.h>

#include <iostream>
#include <memory>
#include <regex>
#include <string>

namespace lanelet::io_handlers
{
std::unique_ptr<LaneletMap> AutowareOsmParser::parse(
  const std::string & filename, ErrorMessages & errors) const
{
  auto map = OsmParser::parse(filename, errors);

  // rerun align function in just in case
  for (Lanelet & lanelet : map->laneletLayer) {
    LineString3d new_left;
    LineString3d new_right;
    std::tie(new_left, new_right) = geometry::align(lanelet.leftBound(), lanelet.rightBound());
    lanelet.setLeftBound(new_left);
    lanelet.setRightBound(new_right);
  }

  return map;
}

namespace
{
RegisterParser<AutowareOsmParser> regParser;
}  // namespace

void AutowareOsmParser::parseVersions(
  const std::string & filename, std::string * format_version, std::string * map_version)
{
  if (format_version == nullptr || map_version == nullptr) {
    std::cerr << __FUNCTION__ << ": either format_version or map_version is null pointer!";
    return;
  }

  pugi::xml_document doc;
  auto result = doc.load_file(filename.c_str());
  if (!result) {
    throw lanelet::ParseError(
      std::string("Errors occurred while parsing osm file: ") + result.description());
  }

  auto osmNode = doc.child("osm");
  auto metainfo = osmNode.child("MetaInfo");
  if (metainfo.attribute("format_version")) {
    *format_version = metainfo.attribute("format_version").value();
  }
  if (metainfo.attribute("map_version")) {
    *map_version = metainfo.attribute("map_version").value();
  }
}

std::optional<uint64_t> parseMajorVersion(const std::string & format_version)
{
  std::regex re(R"(^(\d+\.)?(\d+\.)?(\d+)$)");
  // NOTE(Mamoru Sobue): matches `1`, `1.10`, `1.10.100`
  // `1` ==> [``, ``, `1`]
  // `1.10` ==> [`1.`, ``, `10`]
  // `1.10.100` ==> [`1.`, `10.`, `100`]
  std::smatch match;
  if (!std::regex_match(format_version, match, re)) {
    return std::nullopt;
  }
  if (match[3].str() == "") {
    return std::nullopt;
  }

  std::string major{};
  if (match[1].str() == "") {
    major = match[3].str();
  } else {
    major = match[1].str();
  }
  major.erase(std::remove(major.begin(), major.end(), '.'), major.end());
  return std::stoi(major);
}

}  // namespace lanelet::io_handlers

// NOLINTEND(readability-identifier-naming)
