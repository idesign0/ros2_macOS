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

#include <curl/curl.h>
#include <curl/easy.h>
#include <string.h>
#include <tinyxml2.h>

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4251)  // foo needs to have dll-interface
#endif
#include <google/protobuf/text_format.h>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#include <csignal>
#include <exception>

#include <gz/msgs/fuel_metadata.pb.h>
#include <gz/msgs/Utility.hh>

#include <algorithm>
#include <chrono>
#include <deque>
#include <future>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <gz/common/Console.hh>
#include <gz/common/Filesystem.hh>
#include <gz/common/SignalHandler.hh>
#include <gz/common/URI.hh>

#include "gz/fuel_tools/ClientConfig.hh"
#include "gz/fuel_tools/CollectionIdentifier.hh"
#include "gz/fuel_tools/config.hh"
#include "gz/fuel_tools/FuelClient.hh"
#include "gz/fuel_tools/Helpers.hh"
#include "gz/fuel_tools/Result.hh"
#include "gz.hh"
#include "gz/fuel_tools/WorldIdentifier.hh"

//////////////////////////////////////////////////
/// \brief Print resources in a human readable manner
/// \param[in] _serverConfig Server configuration
/// \param[in] _resourceMap Key is owner name, value is vector of resources
/// \param[in] _resourceType Type, such as "models"
extern "C" void prettyPrint(
    const gz::fuel_tools::ServerConfig &_serverConfig,
    const std::map<std::string, std::vector<std::string>> &_resourceMap,
    const std::string &_resourceType)
{
  unsigned int ownerCount{0};
  unsigned int resourceCount{0};
  std::cout << "\033[92m\033[1m" << _serverConfig.Url().Str()
            << "\033[39m\033[0m" << std::endl;
  for (auto owner = _resourceMap.begin(); owner != _resourceMap.end(); ++owner)
  {
    ownerCount++;

    // Owner indentation
    bool lastOwner = owner == std::prev(_resourceMap.end());
    if (lastOwner)
      std::cout << "└──";
    else
      std::cout << "├──";

    // Owner name
    std::cout << " \033[93m\033[1m" << owner->first << "\033[39m\033[0m"
              << std::endl;

    // Resources
    for (auto resource = owner->second.begin(); resource != owner->second.end();
        ++resource)
    {
      resourceCount++;

      // Resource indentation
      if (lastOwner)
        std::cout << "    ";
      else
        std::cout << "│   ";

      if (resource == std::prev(owner->second.end()))
        std::cout << "└── ";
      else
        std::cout << "├── ";

      // Resource name
      std::cout << *resource << std::endl;
    }
  }

  std::cout << "\033[36m" << ownerCount << " owners, " << resourceCount
            << " " << _resourceType << "\033[39m" << std::endl;
}

//////////////////////////////////////////////////
/// \brief Print resources in a machine readable manner
/// \param[in] _serverConfig Server configuration
/// \param[in] _resourceMap Key is owner name, value is vector of resources
/// \param[in] _resourceType Type, such as "models"
extern "C" void uglyPrint(
    const gz::fuel_tools::ServerConfig &_serverConfig,
    const std::map<std::string, std::vector<std::string>> &_resourceMap,
    const std::string &_resourceType)
{
  CURL *curl = curl_easy_init();
  for (auto owner = _resourceMap.begin(); owner != _resourceMap.end(); ++owner)
  {
    for (auto resource : owner->second)
    {
      auto encodedRes =
          curl_easy_escape(curl, resource.c_str(), resource.size());

      std::cout << _serverConfig.Url().Str() << "/" << _serverConfig.Version()
                << "/" << owner->first << "/" << _resourceType << "/"
                << std::string(encodedRes) << std::endl;
      curl_free(encodedRes);
    }
  }
  curl_easy_cleanup(curl);
}

//////////////////////////////////////////////////
/// \brief Fill a map with all models from a server
/// \param[in] _client Fuel client
/// \param[in] _server Server configuration
/// \param[out] _resourceMap Key is owner name, value is vector of resources
/// \return True if successful, will fail if there's a server error or if the
/// server has no models yet.
extern "C" bool getAllModels(
    const gz::fuel_tools::FuelClient &_client,
    const gz::fuel_tools::ServerConfig &_server,
    std::map<std::string, std::vector<std::string>> &_resourceMap)
{
  auto iter = _client.Models(_server);

  if (!iter)
  {
    std::cout <<
        "Either failed to fetch model list, or server has no models yet."
        << std::endl;
    return false;
  }

  // Rearrange by user
  // key: user name
  // value: vector of model names
  for (; iter; ++iter)
  {
    _resourceMap[iter->Identification().Owner()].push_back(
        iter->Identification().Name());
  }

  return true;
}

//////////////////////////////////////////////////
/// \brief Fill a map with all worlds from a server
/// \param[in] _client Fuel client
/// \param[in] _server Server configuration
/// \param[out] _resourceMap Key is owner name, value is vector of resources
/// \return True if successful, will fail if there's a server error or if the
/// server has no worlds yet.
extern "C" bool getAllWorlds(
    const gz::fuel_tools::FuelClient &_client,
    const gz::fuel_tools::ServerConfig &_server,
    std::map<std::string, std::vector<std::string>> &_resourceMap)
{
  auto iter = _client.Worlds(_server);

  if (!iter)
  {
    std::cout <<
        "Either failed to fetch world list, or server has no worlds yet."
        << std::endl;
    return false;
  }

  // Rearrange by user
  // key: user name
  // value: vector of world names
  for (; iter; ++iter)
  {
    _resourceMap[iter->Owner()].push_back(iter->Name());
  }

  return true;
}

//////////////////////////////////////////////////
/// \brief Fill a map with all models from an owner
/// \param[in] _client Fuel client
/// \param[in] _modelId Identifier for models to be returned
/// \param[out] _resourceMap Key is owner name, value is vector of resources
/// \return True if successful, will fail if there's a server error or if the
/// server has no models yet.
extern "C" bool getOwnerModels(
    const gz::fuel_tools::FuelClient &_client,
    const gz::fuel_tools::ModelIdentifier &_modelId,
    std::map<std::string, std::vector<std::string>> &_resourceMap)
{
  gz::fuel_tools::ModelIter iter = _client.Models(_modelId);

  if (!iter)
  {
    std::cout <<
        "Either failed to fetch model list, or server has no models yet."
        << std::endl;
    return false;
  }

  // Rearrange by user
  // key: user name
  // value: vector of model names
  for (; iter; ++iter)
  {
    _resourceMap[iter->Identification().Owner()].push_back(
        iter->Identification().Name());
  }

  return true;
}

//////////////////////////////////////////////////
/// \brief Fill a map with all worlds from an owner
/// \param[in] _client Fuel client
/// \param[in] _worldId Identifier for worlds to be returned
/// \param[out] _resourceMap Key is owner name, value is vector of resources
/// \return True if successful, will fail if there's a server error or if the
/// server has no worlds yet.
extern "C" bool getOwnerWorlds(
    const gz::fuel_tools::FuelClient &_client,
    const gz::fuel_tools::WorldIdentifier &_worldId,
    std::map<std::string, std::vector<std::string>> &_resourceMap)
{
  auto iter = _client.Worlds(_worldId);

  if (!iter)
  {
    std::cout <<
        "Either failed to fetch world list, or server has no worlds yet."
        << std::endl;
    return false;
  }

  // Rearrange by user
  // key: user name
  // value: vector of world names
  for (; iter; ++iter)
  {
    _resourceMap[iter->Owner()].push_back(iter->Name());
  }

  return true;
}

//////////////////////////////////////////////////
extern "C" GZ_FUEL_TOOLS_VISIBLE char *gzVersion()
{
  return strdup(GZ_FUEL_TOOLS_VERSION_FULL);
}

//////////////////////////////////////////////////
extern "C" GZ_FUEL_TOOLS_VISIBLE int listModels(const char *_url,
    const char *_owner, const char *_raw, const char *_configFile)
{
  std::string urlStr{_url};
  if (!urlStr.empty() && !gz::common::URI::Valid(_url))
  {
    std::cout << "Invalid URL [" << _url << "]" << std::endl;
    return 0;
  }

  gz::common::URI url(urlStr);
  std::string owner{_owner};
  std::string rawStr{_raw};
  std::transform(rawStr.begin(), rawStr.end(),
                 rawStr.begin(), ::tolower);
  bool pretty = rawStr != "true";

  // Client
  gz::fuel_tools::ClientConfig conf;
  if (_configFile && strlen(_configFile) > 0)
  {
    conf.Clear();
    conf.LoadConfig(_configFile);
  }

  if (url.Valid())
  {
    conf.Clear();
    gz::fuel_tools::ServerConfig serverConf;
    serverConf.SetUrl(gz::common::URI(url));
    conf.AddServer(serverConf);
  }

  conf.SetUserAgent("FuelTools " GZ_FUEL_TOOLS_VERSION_FULL);

  // Filter
  gz::fuel_tools::ModelIdentifier modelId;
  if (!owner.empty())
    modelId.SetOwner(owner);

  gz::fuel_tools::FuelClient client(conf);

  // Get models
  for (auto server : conf.Servers())
  {
    modelId.SetServer(server);

    if (pretty)
    {
      std::cout << "Fetching model list from " << server.Url().Str() << "..."
                << std::endl;
    }

    auto startTime = std::chrono::high_resolution_clock::now();

    std::map<std::string, std::vector<std::string>> modelsMap;

    // All models
    if (owner.empty())
    {
      if (!getAllModels(client, server, modelsMap))
        continue;
    }
    else
    {
      if (!getOwnerModels(client, modelId, modelsMap))
        continue;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime);

    if (pretty)
    {
      std::cout << "Received model list (took " << duration.count() << "ms)."
                << std::endl;
    }

    // Print all models
    if (pretty)
      prettyPrint(server, modelsMap, "models");
    else
      uglyPrint(server, modelsMap, "models");
  }

  return true;
}

//////////////////////////////////////////////////
extern "C" GZ_FUEL_TOOLS_VISIBLE int listWorlds(const char *_url,
    const char *_owner, const char *_raw, const char *_configFile)
{
  std::string urlStr{_url};
  if (!urlStr.empty() && !gz::common::URI::Valid(_url))
  {
    std::cout << "Invalid URL [" << _url << "]" << std::endl;
    return 0;
  }

  gz::common::URI url(urlStr);
  std::string owner{_owner};
  std::string rawStr{_raw};
  std::transform(rawStr.begin(), rawStr.end(),
                 rawStr.begin(), ::tolower);
  bool pretty = rawStr != "true";

  // Client
  gz::fuel_tools::ClientConfig conf;
  if (_configFile && strlen(_configFile) > 0)
  {
    conf.Clear();
    conf.LoadConfig(_configFile);
  }

  if (url.Valid())
  {
    conf.Clear();
    gz::fuel_tools::ServerConfig serverConf;
    serverConf.SetUrl(url);
    conf.AddServer(serverConf);
  }

  conf.SetUserAgent("FuelTools " GZ_FUEL_TOOLS_VERSION_FULL);

  // Filter
  gz::fuel_tools::WorldIdentifier worldId;
  if (!owner.empty())
    worldId.SetOwner(owner);

  gz::fuel_tools::FuelClient client(conf);

  // Get worlds
  for (auto server : conf.Servers())
  {
    worldId.SetServer(server);

    if (pretty)
    {
      std::cout << "Fetching world list from " << server.Url().Str() << "..."
                << std::endl;
    }

    auto startTime = std::chrono::high_resolution_clock::now();

    std::map<std::string, std::vector<std::string>> worldsMap;

    // All worlds
    if (owner.empty())
    {
      if (!getAllWorlds(client, server, worldsMap))
        continue;
    }
    else
    {
      if (!getOwnerWorlds(client, worldId, worldsMap))
        continue;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime);

    if (pretty)
    {
      std::cout << "Received world list (took " << duration.count() << "ms)."
                << std::endl;
    }

    // Print all worlds
    if (pretty)
      prettyPrint(server, worldsMap, "worlds");
    else
      uglyPrint(server, worldsMap, "worlds");
  }

  return true;
}

//////////////////////////////////////////////////
extern "C" GZ_FUEL_TOOLS_VISIBLE int downloadUrl(const char *_url,
    const char *_configFile, const char *_header, const char *_type, int _jobs)
{
  // Add signal handler for SIGTERM and SIGINT. Ctrl-C doesn't work without this
  // handler.
  gz::common::SignalHandler sigHandler;
  sigHandler.AddCallback([&](int _sig) {
      if (SIGTERM == _sig || SIGINT == _sig)
      {
        std::exit(1);
      }
  });
  std::string urlStr{_url};
  gz::common::URI url(urlStr);
  if (!url.Valid())
  {
    std::cout << "Download failed: Malformed URL" << std::endl;
    return false;
  }

  // Client
  gz::fuel_tools::ClientConfig conf;
  if (_configFile && strlen(_configFile) > 0)
  {
    conf.Clear();
    conf.LoadConfig(_configFile);
  }

  conf.SetUserAgent("FuelTools " GZ_FUEL_TOOLS_VERSION_FULL);

  gz::fuel_tools::FuelClient client(conf);
  gz::fuel_tools::ModelIdentifier model;
  gz::fuel_tools::WorldIdentifier world;
  gz::fuel_tools::CollectionIdentifier collection;

  // Model?
  if (client.ParseModelUrl(url, model))
  {
    // Download
    if (gz::common::Console::Verbosity() >= 3)
    {
      std::cout << "Downloading model: " << "\033[36m" << std::endl
                << model.AsPrettyString("  ") << "\033[39m" << std::endl;
    }

    if (model.Version() != 0)
    {
      gzwarn << "Requested version [" << model.VersionStr()  << "], but "
              << "currently only the model's latest (tip) version is supported."
              << std::endl;
    }

    gz::fuel_tools::Result result(gz::fuel_tools::ResultType::UNKNOWN);
    if (_header && strlen(_header) > 0)
    {
      std::vector<std::string> headers;
      headers.push_back(_header);
      result = client.DownloadModel(model, headers);
    }
    else
    {
      result = client.DownloadModel(model);
    }

    if (result.Type() != gz::fuel_tools::ResultType::FETCH)
    {
      std::cout << "Download failed because " << result.ReadableResult()
        << std::endl;
      return false;
    }
  }
  // World?
  else if (client.ParseWorldUrl(url, world))
  {
    // Download
    if (gz::common::Console::Verbosity() >= 3)
    {
      std::cout << "Downloading world: " << "\033[36m" << std::endl
                << world.AsPrettyString("  ") << "\033[39m" << std::endl;
    }

    if (world.Version() != 0)
    {
      gzwarn << "Requested version [" << world.VersionStr()  << "], but "
              << "currently only the world's latest (tip) version is supported."
              << std::endl;
    }

    gz::fuel_tools::Result result = client.DownloadWorld(world);

    if (result.Type() != gz::fuel_tools::ResultType::FETCH)
    {
      std::cout << "Download failed because " << result.ReadableResult()
        << std::endl;
      return false;
    }
  }
  // Collection?
  else if (client.ParseCollectionUrl(url, collection))
  {
    if (gz::common::Console::Verbosity() >= 3)
    {
      std::cout << "Downloading collection: " << "\033[36m" << std::endl
                << collection.AsPrettyString("  ") << "\033[39m" << std::endl;
    }

    bool downloadModels = true;
    bool downloadWorlds = true;
    if (nullptr !=_type)
    {
      if (strcmp(_type, "model") == 0)
      {
        downloadWorlds = false;
      }
      else if (strcmp(_type, "world") == 0)
      {
        downloadModels = false;
      }
      else
      {
        gzerr << "Unknown resource type [" << _type << "] specified.\n";
        return false;
      }
    }

    std::vector<gz::fuel_tools::ModelIdentifier> modelIds;
    std::vector<gz::fuel_tools::WorldIdentifier> worldIds;

    if (downloadModels)
    {
      // Get list of model identifiers in collection
      auto modelsIter = client.Models(collection);
      for (; modelsIter; ++modelsIter)
      {
        modelIds.push_back(modelsIter->Identification());
      }
      gzmsg << "Found " << modelIds.size() << " models in collection ["
        << collection.Name() << "]" << std::endl;
    }

    if (downloadWorlds)
    {
      // Get list of world identifiers in collection
      auto worldIter = client.Worlds(collection);
      for (; worldIter; ++worldIter)
      {
        worldIds.push_back(worldIter);
      }
      gzmsg << "Found " << worldIds.size() << " worlds in collection ["
        << collection.Name() << "]" << std::endl;
    }

    const std::size_t totalItemCount = modelIds.size() + worldIds.size();
    if (totalItemCount == 0)
    {
      std::cout << "There are no items in collection [" << collection.Name()
        << "]" << std::endl;
      return false;
    }

    if (downloadModels)
    {
      auto result = client.DownloadModels(modelIds, _jobs);
    }

    if (downloadWorlds)
    {
      auto result = client.DownloadWorlds(worldIds, _jobs);
    }
  }
  else
  {
    std::cout << "Invalid URL: only models and worlds or collections can be "
              << "downloaded so far." << std::endl;
    return false;
  }

  if (gz::common::Console::Verbosity() >= 3)
  {
    std::cout << "Download succeeded." << std::endl;
  }
  return true;
}

//////////////////////////////////////////////////
extern "C" GZ_FUEL_TOOLS_VISIBLE void cmdVerbosity(const char *_verbosity)
{
  gz::common::Console::SetVerbosity(std::atoi(_verbosity));
}

//////////////////////////////////////////////////
extern "C" GZ_FUEL_TOOLS_VISIBLE int upload(const char *_path,
    const char *_url, const char *_header, const char *_private,
    const char *_owner)
{
  gz::common::SignalHandler handler;
  bool sigKilled{false};
  handler.AddCallback([&sigKilled](const int)
  {
    sigKilled = true;
  });

  gz::fuel_tools::ClientConfig conf;
  conf.SetUserAgent("FuelTools " GZ_FUEL_TOOLS_VERSION_FULL);
  gz::fuel_tools::FuelClient client(conf);
  gz::fuel_tools::ModelIdentifier model;

  // Set the server URL, if present.
  if (_url && std::strlen(_url) != 0)
    model.Server().SetUrl(gz::common::URI(_url));

  // Store header information
  std::vector<std::string> headers;
  if (_header && strlen(_header) > 0)
    headers.push_back(_header);

  // Determine if the resource should be private.
  bool privateBool = false;
  if (_private && std::strlen(_private) != 0)
  {
    std::string privateStr = gz::common::lowercase(_private);
    privateBool = privateStr == "1" || privateStr == "true";
  }

  if (!gz::common::exists(_path))
  {
    gzerr << "The model path[" << _path << "] doesn't exist.\n";
    return 0;
  }

  // Get the set of licenses from the server. This license information will
  // be used during the upload process.
  client.PopulateLicenses(model.Server());

  if (gz::common::exists(
        gz::common::joinPaths(_path, "metadata.pbtxt")) ||
      gz::common::exists(
        gz::common::joinPaths(_path, "model.config")))
  {
    std::cout << "Uploading a model[" << _path << "]\n";
    // Upload the model
    return client.UploadModel(_path, model, headers, privateBool, _owner);
  }

  // If a model.config or metadata.pbtxt file does not exist, then assume
  // that the given path is a directory containing multiple models.
  gz::common::DirIter dirIter(_path);
  gz::common::DirIter end;
  while (!sigKilled && dirIter != end)
  {
    if (gz::common::isDirectory(*dirIter) &&
        (gz::common::exists(
           gz::common::joinPaths(*dirIter, "metadata.pbtxt")) ||
         gz::common::exists(
           gz::common::joinPaths(*dirIter, "model.config"))))
    {
      if (!client.UploadModel(*dirIter, model, headers, privateBool, _owner))
      {
        gzerr << "Failed to upload model[" << *dirIter << "]\n";
      }
    }
    ++dirIter;
  }
  return 1;
}

//////////////////////////////////////////////////
extern "C" GZ_FUEL_TOOLS_VISIBLE int deleteUrl(
    const char *_url, const char *_header)
{
  gz::fuel_tools::ClientConfig conf;
  conf.SetUserAgent("FuelTools " GZ_FUEL_TOOLS_VERSION_FULL);
  gz::fuel_tools::FuelClient client(conf);

  // Store header information
  std::vector<std::string> headers;
  if (_header && strlen(_header) > 0)
    headers.push_back(_header);

  gz::common::URI url(_url);

  if (gz::common::Console::Verbosity() >= 3)
  {
    gz::fuel_tools::ModelIdentifier model;
    gz::fuel_tools::WorldIdentifier world;

    if (client.ParseModelUrl(url, model))
    {
      std::cout << "Deleting model: " << "\033[36m" << std::endl
        << model.AsPrettyString("  ") << "\033[39m" << std::endl;
    }
    else if (client.ParseWorldUrl(url, world))
    {
      std::cout << "Deleting world: " << "\033[36m" << std::endl
                << world.AsPrettyString("  ") << "\033[39m" << std::endl;
    }
    else
    {
      std::cout << "Invalid URL: only models and worlds can be delete so far."
        << std::endl;
    }
  }

  client.DeleteUrl(url, headers);

  return 1;
}

//////////////////////////////////////////////////
extern "C" GZ_FUEL_TOOLS_VISIBLE int config2Pbtxt(const char *_path)
{
  gz::msgs::FuelMetadata meta;

  std::ifstream inputFile(_path);
  std::string inputStr((std::istreambuf_iterator<char>(inputFile)),
      std::istreambuf_iterator<char>());

  if (!gz::msgs::ConvertFuelMetadata(inputStr, meta))
  {
    gzerr << "Unable to convert model config[" << _path << "].\n";
    return 0;
  }

  // Output the result.
  std::cout << meta.DebugString() << std::endl;
  return 1;
}

//////////////////////////////////////////////////
extern "C" GZ_FUEL_TOOLS_VISIBLE int pbtxt2Config(const char *_path)
{
  gz::msgs::FuelMetadata meta;

  // Read the pbtxt file.
  std::ifstream inputFile(_path);
  std::string inputStr((std::istreambuf_iterator<char>(inputFile)),
      std::istreambuf_iterator<char>());

  // Parse the file into the fuel metadata message
  if (!google::protobuf::TextFormat::ParseFromString(inputStr, &meta))
  {
    std::cerr << "Unable to parse Fuel metadata from [" << _path << "]\n";
    return 0;
  }

  std::string modelConfig;
  if (!gz::msgs::ConvertFuelMetadata(meta, modelConfig))
  {
    std::cerr << "Unable to convert Fuel metadata to model.config\n";
    return 0;
  }

  std::cout << modelConfig << std::endl;
  return 1;
}

//////////////////////////////////////////////////
extern "C" GZ_FUEL_TOOLS_VISIBLE int editUrl(
    const char *_url, const char *_header, const char *_private,
    const char *_path)
{
  gz::fuel_tools::ClientConfig conf;
  conf.SetUserAgent("FuelTools " GZ_FUEL_TOOLS_VERSION_FULL);
  gz::fuel_tools::FuelClient client(conf);

  // Store header information
  std::vector<std::string> headers;
  if (_header && strlen(_header) > 0)
    headers.push_back(_header);

  gz::common::URI url(_url);

  // Get a privacy change.
  std::optional<bool> privateBool;
  if (_private && std::strlen(_private) != 0)
  {
    std::string privateStr = gz::common::lowercase(_private);
    privateBool = privateStr == "1" || privateStr == "true";
  }

  gz::fuel_tools::ModelIdentifier model;

  std::string modelPath;
  if (_path && std::strlen(_path) != 0)
  {
    if (!gz::common::exists(_path))
    {
      gzerr << "The model path[" << _path << "] doesn't exist.\n";
      return 0;
    }
    modelPath = _path;
  }

  // Check to see if a model has been specified in the the URI.
  if (client.ParseModelUrl(url, model))
  {
    if (gz::common::Console::Verbosity() >= 3)
    {
      std::cout << "Editing model: " << "\033[36m" << std::endl
        << model.AsPrettyString("  ") << "\033[39m" << std::endl;
    }

    // Get the model details from the server
    gz::fuel_tools::ModelIdentifier details;
    if (!client.ModelDetails(model, details, headers))
    {
      gzerr << "Failed to fetch model details for model["
        << model.Name() << "]\n";
      return 0;
    }

    // Change the privacy setting, if a change is present.
    if (privateBool.has_value() || !modelPath.empty())
    {
      details.SetPrivate(*privateBool);

      if (!client.PatchModel(details, headers, modelPath))
      {
        gzerr << "Failed to patch model[" << model.Name() << "].\n";
        return 0;
      }
    }
  }
  else
  {
    std::cout << "Invalid URL: only models can be edited right now."
      << std::endl;
    return 0;
  }

  return 1;
}

//////////////////////////////////////////////////
extern "C" GZ_FUEL_TOOLS_VISIBLE int update(
    const char *_onlyModels, const char *_onlyWorlds, const char *_header)
{
  // Add signal handler for SIGTERM and SIGINT. Ctrl-C doesn't work without this
  // handler.
  gz::common::SignalHandler sigHandler;
  sigHandler.AddCallback([&](int _sig) {
      if (SIGTERM == _sig || SIGINT == _sig)
      {
        std::exit(1);
      }
  });

  bool onlyModelsBool = false;
  if (_onlyModels && std::strlen(_onlyModels) != 0)
  {
    std::string str = gz::common::lowercase(_onlyModels);
    onlyModelsBool = str == "1" || str == "true";
  }
  bool onlyWorldsBool = false;
  if (_onlyWorlds && std::strlen(_onlyWorlds) != 0)
  {
    std::string str = gz::common::lowercase(_onlyWorlds);
    onlyWorldsBool = str == "1" || str == "true";
  }
  // Client
  gz::fuel_tools::ClientConfig conf;

  conf.SetUserAgent("FuelTools " GZ_FUEL_TOOLS_VERSION_FULL);

  gz::fuel_tools::FuelClient client(conf);

  // Headers
  std::vector<std::string> headers;
  if (_header && strlen(_header) > 0)
    headers.push_back(_header);

  if (!onlyWorldsBool && !client.UpdateModels(headers)) {
    return 0;
  }
  if (!onlyModelsBool && !client.UpdateWorlds(headers)) {
    return 0;
  }
  return 1;
}
