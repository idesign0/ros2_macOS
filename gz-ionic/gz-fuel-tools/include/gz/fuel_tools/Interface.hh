/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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

#ifndef GZ_FUEL_TOOLS_INTERFACE_HH_
#define GZ_FUEL_TOOLS_INTERFACE_HH_

#include <string>
#include "gz/fuel_tools/Export.hh"
#include "gz/fuel_tools/FuelClient.hh"

namespace gz::fuel_tools
{
  /// \brief Download the specified resource into the default configuration of
  /// fuel tools. This will place the asset in ~/.gz/fuel.
  /// \param[in] _uri URI to the asset.
  /// \return Path to the downloaded asset. Empty on error.
  GZ_FUEL_TOOLS_VISIBLE std::string fetchResource(
      const std::string &_uri);

  /// \brief Download the specified resource using the ClientConfig
  /// contained in the FuelClient parameter.
  /// \param[in] _uri URI to the asset.
  /// \param[in] _client Custom FuelClient configuration.
  /// \return Path to the downloaded asset. Empty on error.
  GZ_FUEL_TOOLS_VISIBLE std::string fetchResourceWithClient(
      const std::string &_uri, gz::fuel_tools::FuelClient &_client);

  /// \brief Get the SDF file path for a model or world based on a directory
  /// containing a Fuel model or world. Here is a typical use case:
  ///
  ///   1. Fetch a Fuel resource:
  ///   `std::string resourcePath = fetchResource("https://...")`
  ///   2. Get the SDF file for the resource:
  ///   `std::string resourceSdf = sdfFromPath(resourcePath)`
  ///   3. Parse the SDF file using libsdformat.
  ///
  /// This function will determine the SDF file according to the following:
  ///   1. Check for a metadata.pbtxt file, and return the SDF file specified
  /// within the metadata.pbtxt file.
  ///   2. Check for a model.config file, and return the SDF file specified
  /// within the model.config file.
  ///   3. Return the first file with an `.sdf` extension.
  /// \param[in] _path Filesystem path to a Fuel model or world directory.
  /// \return Full path to the model or world SDF file, or empty string if
  /// the SDF file could not be determined.
  GZ_FUEL_TOOLS_VISIBLE std::string sdfFromPath(
      const std::string &_path);
}  // namespace gz::fuel_tools

#endif  // GZ_FUEL_TOOLS_INTERFACE_HH_
