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

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4251)  // foo needs to have dll-interface
#endif
#include <google/protobuf/text_format.h>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#include <gz/msgs/fuel_metadata.pb.h>

#include <gz/msgs/Utility.hh>
#include "gz/common/Console.hh"
#include "gz/fuel_tools/Interface.hh"
#include "gz/fuel_tools/WorldIdentifier.hh"

namespace gz::fuel_tools
{
  //////////////////////////////////////////////
  std::string fetchResource(const std::string &_uri)
  {
    gz::fuel_tools::FuelClient client;
    return fetchResourceWithClient(_uri, client);
  }

  //////////////////////////////////////////////
  std::string fetchResourceWithClient(const std::string &_uri,
      gz::fuel_tools::FuelClient &_client)
  {
    std::string result;

    gz::fuel_tools::ModelIdentifier model;
    gz::fuel_tools::WorldIdentifier world;
    std::string fileUrl;
    gz::common::URI uri(_uri);
    // Download the model, if it is a model URI
    if (_client.ParseModelUrl(uri, model) &&
        !_client.CachedModel(uri, result))
    {
      _client.DownloadModel(uri, result);
    }
    // Download the model, if it's a model file URI
    else if (_client.ParseModelFileUrl(uri, model, fileUrl) &&
        !_client.CachedModelFile(uri, result))
    {
      auto modelUri = _uri.substr(0,
          _uri.find("files", model.UniqueName().size())-1);
      _client.DownloadModel(common::URI(modelUri), result);
      result = common::joinPaths(result, fileUrl);
    }
    // Download the world, if it is a world URI
    else if (_client.ParseWorldUrl(uri, world) &&
             !_client.CachedWorld(uri, result))
    {
      _client.DownloadWorld(uri, result);
    }
    // Download the world, if it's a world file URI
    else if (_client.ParseWorldFileUrl(uri, world, fileUrl) &&
        !_client.CachedWorldFile(uri, result))
    {
      auto worldUri = _uri.substr(0,
          _uri.find("files", world.UniqueName().size())-1);
      _client.DownloadWorld(common::URI(worldUri), result);
      result = common::joinPaths(result, fileUrl);

    }

    return result;
  }

  //////////////////////////////////////////////
  std::string sdfFromPath(const std::string &_path)
  {
    gz::msgs::FuelMetadata meta;
    std::string metadataPath =
      gz::common::joinPaths(_path, "metadata.pbtxt");
    std::string modelConfigPath =
      gz::common::joinPaths(_path, "model.config");

    bool foundMetadataPath = gz::common::exists(metadataPath);
    bool foundModelConfigPath = gz::common::exists(modelConfigPath);

    // Use the metadata.pbtxt or model.config first.
    if (foundMetadataPath || foundModelConfigPath)
    {
      std::string modelPath =
        (foundMetadataPath) ? metadataPath : modelConfigPath;

      // Read the pbtxt or config file.
      std::ifstream inputFile(modelPath);
      std::string inputStr((std::istreambuf_iterator<char>(inputFile)),
          std::istreambuf_iterator<char>());

      if (foundMetadataPath)
      {
        // Parse the file into the fuel metadata message
        if (!google::protobuf::TextFormat::ParseFromString(inputStr, &meta))
          return "";
      }
      else
      {
        if (!gz::msgs::ConvertFuelMetadata(inputStr, meta))
          return "";
      }

      if (meta.has_model())
        return gz::common::joinPaths(_path, meta.model().file());
      else if (meta.has_world())
        return gz::common::joinPaths(_path, meta.world().file());
      return "";
    }

    // Otherwise, use the first ".sdf" file found in the given path.
    common::DirIter dirIter(_path);
    common::DirIter end;
    while (dirIter != end)
    {
      if (common::isFile(*dirIter))
      {
        std::string basename = gz::common::basename(*dirIter);
        // Just some safety checks.
        if (!basename.empty() && basename.find(".sdf") != std::string::npos)
        {
          std::string extension = basename.substr(
              basename.find_last_of('.') + 1);
          if (extension == "sdf")
            return *dirIter;
        }
      }
      ++dirIter;
    }

    return "";
  }
}  // namespace gz::fuel_tools
