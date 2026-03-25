/*
 * Copyright (C) 2023 Open Source Robotics Foundation
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
#ifndef GZ_MSGS_CONVERT_FUELMETADATA_HH_
#define GZ_MSGS_CONVERT_FUELMETADATA_HH_

#include <gz/msgs/config.hh>

// Message Headers
#include "gz/msgs/fuel_metadata.pb.h"

// Data Headers
#include <sstream>
#include <string>
#include <tinyxml2.h>

#include <gz/math/SemanticVersion.hh>

namespace gz::msgs {
// Inline bracket to help doxygen filtering.
inline namespace GZ_MSGS_VERSION_NAMESPACE {

/////////////////////////////////////////////////
inline bool ConvertFuelMetadata(const std::string &_modelConfigStr,
                                msgs::FuelMetadata &_meta)
{
  gz::msgs::FuelMetadata meta;

  auto trimmed = [](std::string _s) -> std::string {
    // Left trim
    _s.erase(_s.begin(), std::find_if(_s.begin(), _s.end(),
          [](int c) {return !std::isspace(c);}));

    // Right trim
    _s.erase(std::find_if(_s.rbegin(), _s.rend(),
          [](int c) {return !std::isspace(c);}).base(), _s.end());

    return _s;
  };

  // Load the model config into tinyxml
  tinyxml2::XMLDocument modelConfigDoc;
  if (modelConfigDoc.Parse(_modelConfigStr.c_str()) !=
      tinyxml2::XML_SUCCESS)
  {
    std::cerr << "Unable to parse model config XML string.\n";
    return false;
  }

  // Get the top level <model> or <world> element.
  tinyxml2::XMLElement *topElement = modelConfigDoc.FirstChildElement(
      "model");
  bool isModel = true;
  if (!topElement)
  {
    topElement = modelConfigDoc.FirstChildElement("world");
    if (!topElement)
    {
      std::cerr << "Model config string does not contain a "
                << "<model> or <world> element\n";
      return false;
    }
    isModel = false;
  }

  // Read the name, which is a mandatory element.
  tinyxml2::XMLElement *elem = topElement->FirstChildElement("name");
  if (!elem || !elem->GetText())
  {
    std::cerr << "Model config string does not contain a <name> element\n";
    return false;
  }
  meta.set_name(trimmed(elem->GetText()));

  // Read the version, if present.
  elem = topElement->FirstChildElement("version");
  if (elem && elem->GetText())
  {
    auto version = std::stoi(trimmed(elem->GetText()));
    meta.set_version(version);
  }

  // Read the description, if present.
  elem = topElement->FirstChildElement("description");
  if (elem && elem->GetText())
    meta.set_description(trimmed(elem->GetText()));

  // Read the dependencies, if any.
  elem = topElement->FirstChildElement("depend");
  while (elem)
  {
    auto modelElem = elem->FirstChildElement("model");
    if (modelElem)
    {
      auto uriElem = modelElem->FirstChildElement("uri");
      if (uriElem)
      {
        auto dependency = meta.add_dependencies();
        dependency->set_uri(uriElem->GetText());
      }
    }
    elem = elem->NextSiblingElement("depend");
  }

  // Read the authors, if any.
  elem = topElement->FirstChildElement("author");
  while (elem)
  {
    gz::msgs::FuelMetadata::Contact *author = meta.add_authors();
    // Get the author name and email
    if (elem->FirstChildElement("name") &&
        elem->FirstChildElement("name")->GetText())
    {
      author->set_name(trimmed(elem->FirstChildElement("name")->GetText()));
    }
    if (elem->FirstChildElement("email") &&
        elem->FirstChildElement("email")->GetText())
    {
      author->set_email(
          trimmed(elem->FirstChildElement("email")->GetText()));
    }

    elem = elem->NextSiblingElement("author");
  }

  // Get the most recent SDF file
  elem = topElement->FirstChildElement("sdf");
  math::SemanticVersion maxVer;
  while (elem)
  {
    if (elem->GetText() && elem->Attribute("version"))
    {
      std::string verStr = elem->Attribute("version");
      math::SemanticVersion ver(trimmed(verStr));
      if (ver > maxVer)
      {
        gz::msgs::Version *verMsg;

        if (isModel)
        {
          meta.mutable_model()->mutable_file_format()->set_name("sdf");
          verMsg =
            meta.mutable_model()->mutable_file_format()->mutable_version();
          meta.mutable_model()->set_file(trimmed(elem->GetText()));
        }
        else
        {
          meta.mutable_world()->mutable_file_format()->set_name("sdf");
          verMsg =
            meta.mutable_world()->mutable_file_format()->mutable_version();
          meta.mutable_world()->set_file(trimmed(elem->GetText()));
        }

        verMsg->set_major(ver.Major());
        verMsg->set_minor(ver.Minor());
        verMsg->set_patch(ver.Patch());
        verMsg->set_prerelease(ver.Prerelease());
        verMsg->set_build(ver.Build());
      }
    }

    elem = elem->NextSiblingElement("sdf");
  }
  if (meta.model().file().empty() && meta.world().file().empty())
  {
    std::cerr << "Model config string does not contain an <sdf> element\n";
    return false;
  }

  _meta.CopyFrom(meta);
  return true;
}

/////////////////////////////////////////////////
inline bool ConvertFuelMetadata(const msgs::FuelMetadata &_meta,
                                std::string &_modelConfigStr)
{
  std::ostringstream out;

  // Output opening tag.
  if (_meta.has_model())
  {
    if (_meta.model().file_format().name() != "sdf")
    {
      std::cerr << "Model _metadata does not contain an SDF file.\n";
      return false;
    }

    out << "<?xml version='1.0'?>\n"
        << "  <model>\n"
        << "    <sdf version='"
        << _meta.model().file_format().version().major()
        << "." << _meta.model().file_format().version().minor() << "'>"
        << _meta.model().file() << "</sdf>\n";
  }
  else
  {
    if (_meta.world().file_format().name() != "sdf")
    {
      std::cerr << "World _metadata does not contain an SDF file.\n";
      return false;
    }

    out << "<?xml version='1.0'?>\n"
        << "  <world>\n"
        << "    <sdf version='"
        << _meta.world().file_format().version().major()
        << "." << _meta.world().file_format().version().minor() << "'>"
        << _meta.world().file() << "</sdf>\n";
  }

  out << "    <name>" << _meta.name() << "</name>\n"
      << "    <version>" << _meta.version() << "</version>\n"
      << "    <description>" << _meta.description() << "</description>\n";

  // Output author information.
  for (int i = 0; i < _meta.authors_size(); ++i)
  {
    out << "    <author>\n"
    << "      <name>" << _meta.authors(i).name() << "</name>\n"
    << "      <email>" << _meta.authors(i).email() << "</email>\n"
    << "    </author>\n";
  }

  // Output dependency information.
  for (int i = 0; i < _meta.dependencies_size(); ++i)
  {
    out << "    <depend>\n"
    << "      <model>\n"
    << "        <uri>" << _meta.dependencies(i).uri() << "</uri>\n"
    << "      </model>\n"
    << "    </depend>\n";
  }

  // Output closing tag.
  if (_meta.has_model())
    out << "  </model>\n";
  else
    out << "  </world>\n";

  _modelConfigStr = out.str();
  return true;
}
}  // namespace
}  // namespace gz::msgs

#endif  // GZ_MSGS_CONVERT_FUELMETADATA_HH_
