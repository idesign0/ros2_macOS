/*
 * Copyright (C) 2017 Open Source Robotics Foundation
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

#include <algorithm>
#include <deque>
#include <iomanip>
#include <iostream>
#include <memory>
#include <regex>
#include <string>

#include <gz/common/Console.hh>
#include <gz/common/Filesystem.hh>
#include <gz/common/URI.hh>
#include <gz/common/Util.hh>

#include <gz/msgs/fuel_metadata.pb.h>
#include <gz/msgs/Utility.hh>

#include "gz/fuel_tools/ClientConfig.hh"
#include "gz/fuel_tools/CollectionIdentifier.hh"
#include "gz/fuel_tools/FuelClient.hh"
#include "gz/fuel_tools/Helpers.hh"
#include "gz/fuel_tools/JSONParser.hh"
#include "gz/fuel_tools/ModelIdentifier.hh"
#include "gz/fuel_tools/RestClient.hh"
#include "gz/fuel_tools/WorldIdentifier.hh"
#include "gz/fuel_tools/WorldIter.hh"

#include "LocalCache.hh"
#include "ModelIterPrivate.hh"
#include "WorldIterPrivate.hh"

namespace std
{
  template<> struct hash<gz::fuel_tools::ModelIdentifier>
  {
    std::size_t operator()(
      const gz::fuel_tools::ModelIdentifier &_id) const noexcept
    {
      return std::hash<std::string>{}(_id.AsString());
    }
  };
}  // namespace std

namespace gz::fuel_tools
{
/// \brief Private Implementation
class FuelClientPrivate
{
  /// \brief A model URL,
  /// E.g.: https://fuel.gazebosim.org/1.0/caguero/models/Beer/2
  /// Where the API version and the model version are optional.
  public: const std::string kModelUrlRegexStr{
    // Method
    "^([[:alnum:]\\.\\+\\-]+):\\/\\/"
    // Server
    "([^\\/\\s]+)\\/+"
    // API Version
    "([0-9]+[.][0-9]+)?\\/*"
    // Owner
    "([^\\/\\s]+)\\/+"
    // "models"
    "models\\/+"
    // Name
    "([^\\/]+)\\/*"
    // Version
    "([0-9]*|tip)"
    // Trailing /
    "/?"};

  /// \brief A world URL,
  /// E.g.: https://fuel.gazebosim.org/1.0/OpenRobotics/worlds/Empty/1
  /// Where the API version and the world version are optional.
  public: const std::string kWorldUrlRegexStr{
    // Method
    "^([[:alnum:]\\.\\+\\-]+):\\/\\/"
    // Server
    "([^\\/\\s]+)\\/+"
    // API Version
    "([0-9]+[.][0-9]+)?\\/*"
    // Owner
    "([^\\/\\s]+)\\/+"
    // "worlds"
    "worlds\\/+"
    // Name
    "([^\\/]+)\\/*"
    // Version
    "([0-9]*|tip)"
    // Trailing /
    "/?"};

  /// \brief A model file URL,
  /// E.g.: https://server.org/1.0/owner/models/modelname/files/meshes/mesh.dae
  /// Where the API version is optional, but the model version is required.
  public: const std::string kModelFileUrlRegexStr{
    // Method
    "^([[:alnum:]\\.\\+\\-]+):\\/\\/"
    // Server
    "([^\\/\\s]+)\\/+"
    // API Version
    "([0-9]+[.][0-9]+)?\\/*"
    // Owner
    "([^\\/\\s]+)\\/+"
    // "models"
    "models\\/+"
    // Model
    "([^\\/]+)\\/+"
    // Version
    "([0-9]*|tip)\\/+"
    // "files"
    "files\\/+"
    // File path
    "(.*)"
    // Trailing /
    "/?"};

  /// \brief A world file URL,
  /// E.g.: https://server.org/1.0/owner/worlds/worldname/files/meshes/mesh.dae
  /// Where the API version is optional, but the world version is required.
  public: const std::string kWorldFileUrlRegexStr{
    // Method
    "^([[:alnum:]\\.\\+\\-]+):\\/\\/"
    // Server
    "([^\\/\\s]+)\\/+"
    // API Version
    "([0-9]+[.][0-9]+)?\\/*"
    // Owner
    "([^\\/\\s]+)\\/+"
    // "worlds"
    "worlds\\/+"
    // World
    "([^\\/]+)\\/+"
    // Version
    "([0-9]*|tip)\\/+"
    // "files"
    "files\\/+"
    // File path
    "(.*)"
    // Trailing /
    "/?"};

  /// \brief A collection URL,
  /// E.g.:
  /// https://fuel.gazebosim.org/1.0/OpenRobotics/collections/TestColl
  /// Where the API version is optional
  public: const std::string kCollectionUrlRegexStr{
    // Method
    "^([[:alnum:]\\.\\+\\-]+):\\/\\/"
    // Server
    "([^\\/\\s]+)\\/+"
    // API Version
    "([0-9]+[.][0-9]+)?\\/*"
    // Owner
    "([^\\/\\s]+)\\/+"
    // "collections"
    "collections\\/+"
    // Name
    "([^\\/]+)\\/*"};

  /// \brief Recursively get all the files in the given path.
  /// \param[in] _path Path to process.
  /// \param[out] _files All the files in the given _path.
  public: void AllFiles(const std::string &_path,
              std::vector<std::string> &_files) const;

  /// \brief Populate a model form such that it can be used during
  /// model creation and editing REST calls.
  /// \param[in] _pathToModelDir Path to the model directory.
  /// \param[in] _id Model identifier information.
  /// \param[in] _private True if this model should be private.
  /// \param[in] _owner Model owner name.
  /// \param[out] _form Form to fill.
  /// \return True if the operation completed successfully.
  public: bool FillModelForm(const std::string &_pathToModelDir,
              const ModelIdentifier &_id, bool _private,
              const std::string &_owner,
              std::multimap<std::string, std::string> &_form);

  /// \brief This function requests the available licenses from the
  ///  Fuel server and stores this information locally.
  ///
  /// The UploadModel function can use this information to set
  /// appropriate license information based on a model's metadata.pbtxt
  /// file. If license information is not available, then the
  /// UploadModel function will default to the
  /// "Creative Commons - Public Domain" license.
  /// \param[in] _server Information about the server that provides
  /// license information.
  public: void PopulateLicenses(const ServerConfig &_server);

  /// \brief Get zip data from a REST response. This is used by world and
  /// model download.
  public: void ZipFromResponse(const RestResponse &_resp,
              std::string &_zip);

  /// \brief Client configuration
  public: ClientConfig config;

  /// \brief RESTful client
  public: Rest rest;

  /// \brief Local Cache
  public: std::shared_ptr<LocalCache> cache;

  /// \brief Regex to parse Gazebo Fuel model URLs.
  public: std::unique_ptr<std::regex> urlModelRegex;

  /// \brief Regex to parse Gazebo Fuel world URLs.
  public: std::unique_ptr<std::regex> urlWorldRegex;

  /// \brief Regex to parse Gazebo Fuel model file URLs.
  public: std::unique_ptr<std::regex> urlModelFileRegex;

  /// \brief Regex to parse Gazebo Fuel world file URLs.
  public: std::unique_ptr<std::regex> urlWorldFileRegex;

  /// \brief Regex to parse Gazebo Fuel Collection URLs.
  public: std::unique_ptr<std::regex> urlCollectionRegex;

  /// \brief The set of licenses where the key is the name of the license
  /// and the value is the license ID on a Fuel server. See the
  /// PopulateLicenses function.
  public: std::map<std::string, unsigned int> licenses;
};

//////////////////////////////////////////////////
FuelClient::FuelClient()
  : FuelClient(ClientConfig(), Rest())
{
}

//////////////////////////////////////////////////
FuelClient::FuelClient(const ClientConfig &_config, const Rest &_rest)
  : dataPtr(std::make_unique<FuelClientPrivate>())
{
  this->dataPtr->config = _config;
  this->dataPtr->rest = _rest;
  this->dataPtr->rest.SetUserAgent(this->dataPtr->config.UserAgent());

  this->dataPtr->cache = std::make_unique<LocalCache>(&(this->dataPtr->config));

  this->dataPtr->urlModelRegex.reset(new std::regex(
    this->dataPtr->kModelUrlRegexStr));
  this->dataPtr->urlWorldRegex.reset(new std::regex(
    this->dataPtr->kWorldUrlRegexStr));
  this->dataPtr->urlModelFileRegex.reset(new std::regex(
    this->dataPtr->kModelFileUrlRegexStr));
  this->dataPtr->urlWorldFileRegex.reset(new std::regex(
    this->dataPtr->kWorldFileUrlRegexStr));
  this->dataPtr->urlCollectionRegex.reset(new std::regex(
    this->dataPtr->kCollectionUrlRegexStr));
}

//////////////////////////////////////////////////
FuelClient::~FuelClient()
{
}

//////////////////////////////////////////////////
ClientConfig &FuelClient::Config()
{
  return this->dataPtr->config;
}

//////////////////////////////////////////////////
Result FuelClient::ModelDetails(const ModelIdentifier &_id,
    ModelIdentifier &_model) const
{
  return this->ModelDetails(_id, _model, {});
}

//////////////////////////////////////////////////
Result FuelClient::ModelDetails(const ModelIdentifier &_id,
    ModelIdentifier &_model, const std::vector<std::string> &_headers) const
{
  gz::fuel_tools::Rest rest;
  RestResponse resp;

  auto serverUrl = _id.Server().Url().Str();
  auto version = _id.Server().Version();
  common::URIPath path;
  path = path / _id.Owner() / "models" / _id.Name();

  std::vector<std::string> headersIncludingServerConfig = _headers;
  AddServerConfigParametersToHeaders(
    _id.Server(), headersIncludingServerConfig);
  resp = rest.Request(HttpMethod::GET, serverUrl, version,
      path.Str(), {}, headersIncludingServerConfig, "");
  if (resp.statusCode != 200)
    return Result(ResultType::FETCH_ERROR);

  _model = JSONParser::ParseModel(resp.data, _id.Server());

  return Result(ResultType::FETCH);
}

//////////////////////////////////////////////////
ModelIter FuelClient::Models(const ServerConfig &_server)
{
  return const_cast<const FuelClient*>(this)->Models(_server);
}

//////////////////////////////////////////////////
ModelIter FuelClient::Models(const ServerConfig &_server) const
{
  ModelIter iter = ModelIterFactory::Create(this->dataPtr->rest,
      _server, "models");

  if (!iter)
  {
    // Return just the cached models
    gzwarn << "Failed to fetch models from server, returning cached models."
            << std::endl << _server.AsString() << std::endl;

    ModelIdentifier id;
    id.SetServer(_server);

    return this->dataPtr->cache->MatchingModels(id);
  }
  return iter;
}

//////////////////////////////////////////////////
Result FuelClient::WorldDetails(const WorldIdentifier &_id,
    WorldIdentifier &_world) const
{

  return this->WorldDetails(_id, _world, {});
}

//////////////////////////////////////////////////
Result FuelClient::WorldDetails(const WorldIdentifier &_id,
    WorldIdentifier &_world, const std::vector<std::string> &_headers) const
{
  auto serverUrl = _id.Server().Url().Str();

  if (serverUrl.empty() || _id.Owner().empty() || _id.Name().empty())
    return Result(ResultType::FETCH_ERROR);

  gz::fuel_tools::Rest rest;
  RestResponse resp;

  auto version = _id.Server().Version();
  common::URIPath path;
  path = path / _id.Owner() / "worlds" / _id.Name();

  std::vector<std::string> headersIncludingServerConfig = _headers;
  AddServerConfigParametersToHeaders(
    _id.Server(), headersIncludingServerConfig);

  resp = rest.Request(HttpMethod::GET, serverUrl, version,
      path.Str(), {}, {}, "");
  if (resp.statusCode != 200)
    return Result(ResultType::FETCH_ERROR);

  _world = JSONParser::ParseWorld(resp.data, _id.Server());

  return Result(ResultType::FETCH);
}

//////////////////////////////////////////////////
WorldIter FuelClient::Worlds(const ServerConfig &_server) const
{
  Rest rest(this->dataPtr->rest);
  WorldIter iter = WorldIterFactory::Create(rest, _server, "worlds");

  if (!iter)
  {
    // Return just the cached worlds
    gzwarn << "Failed to fetch worlds from server, returning cached worlds."
            << std::endl << _server.AsString() << std::endl;

    WorldIdentifier id;
    id.SetServer(_server);

    return this->dataPtr->cache->MatchingWorlds(id);
  }
  return iter;
}

//////////////////////////////////////////////////
ModelIter FuelClient::Models(const ModelIdentifier &_id)
{
  return this->Models(_id, true);
}

//////////////////////////////////////////////////
ModelIter FuelClient::Models(const ModelIdentifier &_id) const
{
  return this->Models(_id, true);
}

//////////////////////////////////////////////////
ModelIter FuelClient::Models(const ModelIdentifier &_id, bool _checkCache)
{
  return const_cast<const FuelClient*>(this)->Models(_id, _checkCache);
}

//////////////////////////////////////////////////
ModelIter FuelClient::Models(const ModelIdentifier &_id, bool _checkCache) const
{
  if (_checkCache)
  {
    // Check local cache first
    ModelIter localIter = this->dataPtr->cache->MatchingModels(_id);
    if (localIter)
      return localIter;
  }

  // TODO(nkoenig) try to fetch model directly from a server
  // Note: gz-fuel-server doesn't like URLs ending in /
  common::URIPath path;
  if (!_id.Name().empty() && !_id.Owner().empty())
    path = path / _id.Owner() / "models" / _id.Name();
  else if (!_id.Owner().empty())
    path = path / _id.Owner() / "models";

  if (path.Str().empty())
    return ModelIterFactory::Create();

  gzmsg << _id.UniqueName() << " not found in cache, attempting download\n";

  return ModelIterFactory::Create(this->dataPtr->rest, _id.Server(),
      path.Str());
}

//////////////////////////////////////////////////
ModelIter FuelClient::Models(const CollectionIdentifier &_id) const
{
  return ModelIterFactory::Create(
      this->dataPtr->rest, _id.Server(),
      common::joinPaths(_id.Owner(), "collections", _id.Name(), "models"));
}

//////////////////////////////////////////////////
WorldIter FuelClient::Worlds(const WorldIdentifier &_id) const
{
  // Check local cache first
  WorldIter localIter = this->dataPtr->cache->MatchingWorlds(_id);
  if (localIter)
    return localIter;

  gzmsg << _id.UniqueName() << " not found in cache, attempting download\n";

  // Note: gz-fuel-server doesn't like URLs ending in /
  common::URIPath path;
  if (!_id.Name().empty())
    path = path / _id.Owner() / "worlds" / _id.Name();
  else
    path = path / _id.Owner() / "worlds";

  Rest rest(this->dataPtr->rest);
  return WorldIterFactory::Create(rest, _id.Server(), path.Str());
}

//////////////////////////////////////////////////
WorldIter FuelClient::Worlds(const CollectionIdentifier &_id) const
{
  return WorldIterFactory::Create(
      this->dataPtr->rest, _id.Server(),
      common::joinPaths(_id.Owner(), "collections", _id.Name(), "worlds"));
}

//////////////////////////////////////////////////
Result FuelClient::UploadModel(const std::string &_pathToModelDir,
    const ModelIdentifier &_id, const std::vector<std::string> &_headers,
    bool _private)
{
  return this->UploadModel(_pathToModelDir, _id, _headers, _private, "");
}

//////////////////////////////////////////////////
Result FuelClient::UploadModel(const std::string &_pathToModelDir,
    const ModelIdentifier &_id, const std::vector<std::string> &_headers,
    bool _private, const std::string &_owner)
{
  gz::fuel_tools::Rest rest;
  RestResponse resp;

  std::multimap<std::string, std::string> form;
  if (!this->dataPtr->FillModelForm(_pathToModelDir, _id, _private, _owner,
        form))
  {
    return Result(ResultType::UPLOAD_ERROR);
  }

  std::vector<std::string> headersIncludingServerConfig = _headers;
  AddServerConfigParametersToHeaders(
    _id.Server(), headersIncludingServerConfig);
  // Send the request.
  resp = rest.Request(HttpMethod::POST_FORM, _id.Server().Url().Str(),
      _id.Server().Version(), "models", {},
      headersIncludingServerConfig, "", form);

  if (resp.statusCode != 200)
  {
    std::string categories;
    if (form.find("categories") != form.end())
    {
      categories = form.find("categories")->second;
    }

    gzerr << "Failed to upload model." << std::endl
           << "  Server: " << _id.Server().Url().Str() << std::endl
           << "  Server API Version: " <<  _id.Server().Version() << std::endl
           << "  Route: /models\n"
           << "  Categories: " << categories << std::endl
           << "  REST response code: " << resp.statusCode
           << std::endl << std::endl
           << "Suggestions" << std::endl
           << "  1. Is the Server URL correct? Try entering it on a browser.\n"
           << "  2. Do the categories exist? If you are using the Fuel server,"
           << "     then you can get the complete list at"
           << "     https://fuel.gazebosim.org/1.0/categories.\n"
           << "  3. If the owner is specified, make sure you have correct\n"
           << "     permissions." << std::endl;
    return Result(ResultType::FETCH_ERROR);
  }

  return Result(ResultType::UPLOAD);
}

//////////////////////////////////////////////////
void FuelClient::AddServerConfigParametersToHeaders(
  const gz::fuel_tools::ServerConfig &_serverConfig,
  std::vector<std::string> &_headers) const
{
  bool privateTokenDefined = false;
  for (const auto &header : _headers)
  {
    if (header.find("Private-token:") != std::string::npos)
    {
      privateTokenDefined = true;
    }
  }
  if (!privateTokenDefined)
  {
    if (!_serverConfig.ApiKey().empty())
    {
      _headers.push_back("Private-token: " + _serverConfig.ApiKey());
    }
  }
}

//////////////////////////////////////////////////
Result FuelClient::DeleteUrl(const gz::common::URI &_uri,
    const std::vector<std::string> &_headers)
{
  gz::fuel_tools::Rest rest;

  RestResponse resp;

  std::string server;
  std::string version;
  common::URIPath path;
  std::string type;
  std::string name;

  ModelIdentifier modelId;
  WorldIdentifier worldId;
  std::vector<std::string> headersIncludingServerConfig = _headers;
  if (this->ParseModelUrl(_uri, modelId))
  {
    type = "model";
    name = modelId.UniqueName();
    server = modelId.Server().Url().Str();
    version = modelId.Server().Version();
    path = path / modelId.Owner() / "models" / modelId.Name();
    AddServerConfigParametersToHeaders(
      modelId.Server(), headersIncludingServerConfig);
  }
  else if (this->ParseWorldUrl(_uri, worldId))
  {
    type = "world";
    name = worldId.UniqueName();
    server = worldId.Server().Url().Str();
    version = worldId.Server().Version();
    path = path / worldId.Owner() / "worlds" / worldId.Name();
    AddServerConfigParametersToHeaders(
      worldId.Server(), headersIncludingServerConfig);
  }
  else
  {
    gzerr << "Unable to parse URI[" << _uri.Str() << "]\n";
    return Result(ResultType::DELETE_ERROR);
  }

  // Send the request.
  resp = rest.Request(HttpMethod::DELETE, server, version, path.Str(), {},
      headersIncludingServerConfig, "", {});

  if (resp.statusCode != 200)
  {
    gzerr << "Failed to delete resource." << std::endl
           << "  Server: " << server << std::endl
           << "  API Version: " << version << std::endl
           << "  Route: " << path.Str() << std::endl
           << "  REST response code: " << resp.statusCode << std::endl;
    return Result(ResultType::DELETE_ERROR);
  }
  else
  {
    gzmsg << "Deleted " << type << " [" << name << "]" << std::endl;
  }

  return Result(ResultType::DELETE);
}

//////////////////////////////////////////////////
Result FuelClient::DownloadModel(const ModelIdentifier &_id)
{
  return this->DownloadModel(_id, {});
}

//////////////////////////////////////////////////
Result FuelClient::DownloadModel(const ModelIdentifier &_id,
    const std::vector<std::string> &_headers)
{
  std::vector<ModelIdentifier> dependencies;
  auto res = this->DownloadModel(_id, _headers, dependencies);

  if(!res)
    return res;

  for (const ModelIdentifier &dep : dependencies)
  {
    // Download dependency if not in the local cache
    if (!this->dataPtr->cache->MatchingModel(dep))
    {
      auto depRes = this->DownloadModel(dep, _headers);

      if(!depRes)
        return depRes;
    }
  }

  return res;
}

//////////////////////////////////////////////////
Result FuelClient::DownloadModel(const ModelIdentifier &_id,
    const std::vector<std::string> &_headers,
    std::vector<ModelIdentifier> &_dependencies)
{
  // Server config
  if (!_id.Server().Url().Valid() || _id.Server().Version().empty())
  {
    gzerr << "Can't download model, server configuration incomplete: "
          << std::endl << _id.Server().AsString() << std::endl;
    return Result(ResultType::FETCH_ERROR);
  }

  // Route
  common::URIPath route;
  route = route / _id.Owner() / "models" / _id.Name() / _id.VersionStr() /
        (_id.Name() + ".zip");

  gzmsg << "Downloading model [" << _id.UniqueName() << "]" << std::endl;

  std::vector<std::string> headersIncludingServerConfig = _headers;
  AddServerConfigParametersToHeaders(
    _id.Server(), headersIncludingServerConfig);
  // Request
  gz::fuel_tools::Rest rest;
  RestResponse resp;
  resp = rest.Request(HttpMethod::GET, _id.Server().Url().Str(),
      _id.Server().Version(), route.Str(), {"link=true"},
      headersIncludingServerConfig, "");
  if (resp.statusCode != 200)
  {
    gzerr << "Failed to download model." << std::endl
           << "  Server: " << _id.Server().Url().Str() << std::endl
           << "  Route: " << route.Str() << std::endl
           << "  REST response code: " << resp.statusCode << std::endl;
    return Result(ResultType::FETCH_ERROR);
  }

  // Get version from header
  ModelIdentifier newId = _id;
  unsigned int version = 1;
  if (resp.headers.find("X-Ign-Resource-Version") != resp.headers.end())
  {
    try
    {
      version = std::stoi(resp.headers["X-Ign-Resource-Version"]);
    }
    catch(std::invalid_argument &)
    {
      gzwarn << "Failed to convert X-Ign-Resource-Version header value ["
              << resp.headers["X-Ign-Resource-Version"]
              << "] to integer. Hardcoding version 1." << std::endl;
    }
  }
  else
  {
    gzwarn << "Missing X-Ign-Resource-Version in REST response headers."
            << " Hardcoding version 1." << std::endl;
  }
  newId.SetVersion(version);

  std::string zipData;
  this->dataPtr->ZipFromResponse(resp, zipData);

  // Save
  // Note that the save function doesn't return the path
  if (zipData.empty() || !this->dataPtr->cache->SaveModel(newId, zipData, true))
    return Result(ResultType::FETCH_ERROR);

  return this->ModelDependencies(_id, _dependencies);
}

//////////////////////////////////////////////////
Result FuelClient::ModelDependencies(const ModelIdentifier &_id,
    std::vector<ModelIdentifier> &_dependencies)
{
  _dependencies.clear();

  // Locate any dependencies from the input model and download them.
  std::string path;
  gz::msgs::FuelMetadata meta;

  if (this->CachedModel(_id, path))
  {
    std::string metadataPath =
      gz::common::joinPaths(path, "metadata.pbtxt");
    std::string modelConfigPath =
      gz::common::joinPaths(path, "model.config");

    bool foundMetadataPath = gz::common::exists(metadataPath);
    bool foundModelConfigPath = gz::common::exists(modelConfigPath);

    if (foundMetadataPath || foundModelConfigPath)
    {
      std::string modelPath =
        (foundMetadataPath) ? metadataPath : modelConfigPath;

      // Read the pbtxt file.
      std::ifstream inputFile(modelPath);
      std::string inputStr((std::istreambuf_iterator<char>(inputFile)),
        std::istreambuf_iterator<char>());

      if (foundMetadataPath)
      {
        // Parse the file into the fuel metadata message
        if (!google::protobuf::TextFormat::ParseFromString(inputStr, &meta))
        {
          gzerr << "Unable to parse fuel metadata message ["
                << modelPath << "]" << std::endl;
          return Result(ResultType::FETCH_ERROR);
        }
      }
      else
      {
        if (!gz::msgs::ConvertFuelMetadata(inputStr, meta))
        {
          return Result(ResultType::UPLOAD_ERROR);
        }
      }

      for (int i = 0; i < meta.dependencies_size(); ++i)
      {
        gz::common::URI dependencyURI(meta.dependencies(i).uri());

        ModelIdentifier dependencyID;
        if(!this->ParseModelUrl(dependencyURI, dependencyID))
        {
          // There is a potential that dependencies are specified via
          // [model://model_name], which is valid, but not something that we
          // can fetch from Fuel. In that case, warn the user so they have
          // a chance to update their specified dependencies.
          gzwarn << "Error resolving URL for dependency [" <<
            meta.dependencies(i).uri() << "] of model [" <<
            _id.UniqueName() <<"]: Skipping" << std::endl;
        } else {
          _dependencies.push_back(dependencyID);
        }
      }
    }
  }

  return Result(ResultType::FETCH);
}

//////////////////////////////////////////////////
Result FuelClient::ModelDependencies(
    const std::vector<ModelIdentifier> &_ids,
    std::vector<ModelIdentifier> &_dependencies)
{
  std::vector<ModelIdentifier> newDeps;
  for (const auto &modelId : _ids)
  {
    std::vector<ModelIdentifier> modelDeps;
    auto result = this->ModelDependencies(modelId, modelDeps);

    if (!modelDeps.empty())
    {
      std::vector<ModelIdentifier> recursiveDeps;
      this->ModelDependencies(modelDeps, recursiveDeps);

      std::copy(modelDeps.begin(), modelDeps.end(),
                std::back_inserter(newDeps));
      std::copy(recursiveDeps.begin(), recursiveDeps.end(),
                std::back_inserter(newDeps));
    }
  }

  _dependencies = std::vector<ModelIdentifier>(newDeps.begin(), newDeps.end());
  return Result(ResultType::FETCH);
}


//////////////////////////////////////////////////
Result FuelClient::DownloadWorld(WorldIdentifier &_id)
{
  return this->DownloadWorld(_id, {});
}

//////////////////////////////////////////////////
Result FuelClient::DownloadWorld(WorldIdentifier &_id,
    const std::vector<std::string> &_headers)
{
  // Server config
  if (!_id.Server().Url().Valid() || _id.Server().Version().empty())
  {
    gzerr << "Can't download world, server configuration incomplete: "
          << std::endl << _id.Server().AsString() << std::endl;
    return Result(ResultType::FETCH_ERROR);
  }

  // Route
  common::URIPath route;
  route = route / _id.Owner() / "worlds" / _id.Name() / _id.VersionStr() /
        (_id.Name() + ".zip");

  gzmsg << "Downloading world [" << _id.UniqueName() << "]" << std::endl;

  std::vector<std::string> headersIncludingServerConfig = _headers;
  AddServerConfigParametersToHeaders(
    _id.Server(), headersIncludingServerConfig);

  // Request
  gz::fuel_tools::Rest rest;
  RestResponse resp;
  resp = rest.Request(HttpMethod::GET, _id.Server().Url().Str(),
      _id.Server().Version(), route.Str(), {"link=true"},
      headersIncludingServerConfig, "");
  if (resp.statusCode != 200)
  {
    gzerr << "Failed to download world." << std::endl
           << "  Server: " << _id.Server().Url().Str() << std::endl
           << "  Route: " << route.Str() << std::endl
           << "  REST response code: " << resp.statusCode << std::endl;
    return Result(ResultType::FETCH_ERROR);
  }

  // Get version from header
  unsigned int version = 1;

  if (resp.headers.find("X-Ign-Resource-Version") != resp.headers.end())
  {
    try
    {
      version = std::stoi(resp.headers["X-Ign-Resource-Version"]);
    }
    catch(std::invalid_argument &)
    {
      gzwarn << "Failed to convert X-Ign-Resource-Version header value ["
              << resp.headers["X-Ign-Resource-Version"]
              << "] to integer. Hardcoding version 1." << std::endl;
    }
  }
  else
  {
    gzwarn << "Missing X-Ign-Resource-Version in REST response headers."
            << " Hardcoding version 1." << std::endl;
  }
  _id.SetVersion(version);

  std::string zipData;
  this->dataPtr->ZipFromResponse(resp, zipData);

  // Save
  if (zipData.empty() || !this->dataPtr->cache->SaveWorld(_id, zipData, true))
      return Result(ResultType::FETCH_ERROR);

  return Result(ResultType::FETCH);
}

//////////////////////////////////////////////////
std::vector<FuelClient::ModelResult> FuelClient::DownloadModels(
    const std::vector<ModelIdentifier> &_ids,
    size_t _jobs)
{
  std::mutex resultMutex;
  std::vector<FuelClient::ModelResult> result;

  std::mutex idsMutex;
  std::deque<ModelIdentifier> idsToDownload(_ids.begin(), _ids.end());
  std::unordered_set<ModelIdentifier> uniqueIds(_ids.begin(), _ids.end());

  std::atomic<bool> running = true;

  auto downloadWorker = [&](){
    ModelIdentifier id;

    while(running)
    {
      // Pop the next ID off the queue
      {
        std::lock_guard<std::mutex> lock(idsMutex);

        if (idsToDownload.empty())
        {
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
          continue;
        }

        id = idsToDownload.front();
        idsToDownload.pop_front();
      }

      std::vector<ModelIdentifier> dependencies;
      auto modelResult = this->DownloadModel(id, {}, dependencies);

      {
        std::lock_guard<std::mutex> lock(resultMutex);
        result.push_back(std::make_tuple(id, modelResult));
      }

      if (!dependencies.empty())
      {
        std::lock_guard<std::mutex> lock(idsMutex);
        gzdbg << "Adding " << dependencies.size()
          << " model dependencies to queue from " << id.Name() << "\n";
        for (const auto &dep : dependencies)
        {
          if (uniqueIds.count(dep) == 0)
          {
            idsToDownload.push_back(dep);
            uniqueIds.insert(dep);
          }
        }
      }
    }
  };

  std::vector<std::thread> workers;

  for (size_t ii = 0; ii < _jobs; ++ii)
  {
    workers.push_back(std::thread(downloadWorker));
  }

  gzmsg << "Preparing to download "
    << idsToDownload.size() << " models with "
    << _jobs << " worker threads\n";


  while (running)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    if(idsToDownload.empty())
    {
      running = false;
    }
  }

  for (auto& worker : workers)
  {
    worker.join();
  }

  gzmsg << "Finished, downloaded " << result.size() << " models in total\n";

  return result;
}

//////////////////////////////////////////////////
Result FuelClient::DownloadWorlds(
    const std::vector<WorldIdentifier> &_ids, size_t _jobs)
{
  std::deque<std::future<gz::fuel_tools::Result>> tasks;
  // Check for finished tasks by checking if the status of their futures is
  // "ready". If a task is finished, check if it succeeded and print out an
  // error message if it failed. When a task is finished, it gets erased from
  // the tasks list to make room for other tasks to be added.
  size_t itemCount = 0;
  const size_t totalItemCount = _ids.size();

  gzmsg << "Using " << _jobs << " jobs to download collection of "
         << totalItemCount << " items" << std::endl;

  auto checkForFinishedTasks = [&itemCount, &totalItemCount, &tasks] {
    auto finishedIt =
        std::partition(tasks.begin(), tasks.end(), [](const auto &_task)
            {
              return std::future_status::ready !=
              _task.wait_for(std::chrono::milliseconds(100));
            });

    if (finishedIt != tasks.end())
    {
      for (auto taskIt = finishedIt; taskIt != tasks.end(); ++taskIt)
      {
        gz::fuel_tools::Result result = taskIt->get();
        if (result)
        {
          ++itemCount;
        }
        else
        {
          gzerr << result.ReadableResult() << std::endl;
        }
      }

      tasks.erase(finishedIt, tasks.end());
      gzmsg << "Downloaded: " << itemCount << " / " << totalItemCount
             << std::endl;
    }
  };

  // We need a mutable worldId because DownloadWorld modifies it
  for (auto& id : _ids)
  {
    while (tasks.size() >= _jobs)
    {
      checkForFinishedTasks();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    auto handle = std::async(std::launch::async, [&id, this]
        {
          WorldIdentifier tempId = id;
          return this->DownloadWorld(tempId);
        });
    tasks.push_back(std::move(handle));
  }

  while (!tasks.empty())
  {
    checkForFinishedTasks();
  }

  return Result(ResultType::FETCH);
}

//////////////////////////////////////////////////
bool FuelClient::ParseModelUrl(const common::URI &_modelUrl,
    ModelIdentifier &_id)
{
  if (!_modelUrl.Valid())
    return false;

  auto urlStr = _modelUrl.Str();

  std::smatch match;
  std::string scheme;
  std::string server;
  std::string apiVersion;
  std::string owner;
  std::string modelName;
  std::string modelVersion;

  if (std::regex_match(urlStr, match, *this->dataPtr->urlModelRegex) &&
      match.size() >= 5u)
  {
    unsigned int i{1};

    scheme = match[i++];
    server = match[i++];
    apiVersion = match[i++];
    owner = match[i++];
    modelName = match[i++];
    modelVersion = match[i++];
  }
  else
  {
    return false;
  }

  // Get remaining server information from config
  common::URI serverUri;
  serverUri.SetScheme(scheme);
  serverUri.SetAuthority(gz::common::URIAuthority("//" + server));

  _id.Server().SetUrl(serverUri);
  _id.Server().SetVersion(apiVersion);
  for (const auto &s : this->dataPtr->config.Servers())
  {
    // cppcheck-suppress useStlAlgorithm
    if (s.Url().Str() == _id.Server().Url().Str()) {
      if (!apiVersion.empty() && s.Version() != _id.Server().Version())
      {
        gzwarn << "Requested server API version [" << apiVersion
                << "] for server [" << s.Url().Str() << "], but will use ["
                << s.Version() << "] as given in the config file."
                << std::endl;
      }
      _id.Server() = s;
      break;
    }
  }

  if (_id.Server().Version().empty())
  {
    gzwarn << "Server configuration is incomplete:" << std::endl
            << _id.Server().AsString();
  }

  _id.SetOwner(owner);
  _id.SetName(modelName);
  _id.SetVersionStr(modelVersion);

  return true;
}

//////////////////////////////////////////////////
bool FuelClient::ParseWorldUrl(const common::URI &_worldUrl,
    WorldIdentifier &_id)
{
  if (!_worldUrl.Valid())
    return false;

  auto urlStr = _worldUrl.Str();

  std::smatch match;
  std::string scheme;
  std::string server;
  std::string apiVersion;
  std::string owner;
  std::string worldName;
  std::string worldVersion;

  if (std::regex_match(urlStr, match, *this->dataPtr->urlWorldRegex) &&
      match.size() >= 5u)
  {
    unsigned int i{1};

    scheme = match[i++];
    server = match[i++];
    apiVersion = match[i++];
    owner = match[i++];
    worldName = match[i++];
    worldVersion = match[i++];
  }
  else
  {
    return false;
  }

  // Get remaining server information from config
  common::URI serverUri;
  serverUri.SetScheme(scheme);
  serverUri.SetAuthority(gz::common::URIAuthority("//" + server));

  _id.Server().SetUrl(serverUri);
  _id.Server().SetVersion(apiVersion);
  for (const auto &s : this->dataPtr->config.Servers())
  {
    // cppcheck-suppress useStlAlgorithm
    if (s.Url() == _id.Server().Url()) {
      if (!apiVersion.empty() && s.Version() != _id.Server().Version())
      {
        gzwarn << "Requested server API version [" << apiVersion
                << "] for server [" << s.Url().Str() << "], but will use ["
                << s.Version() << "] as given in the config file."
                << std::endl;
      }
      _id.Server() = s;
      break;
    }
  }

  if (_id.Server().Version().empty())
  {
    gzwarn << "Server configuration is incomplete:" << std::endl
            << _id.Server().AsString();
  }

  _id.SetOwner(owner);
  _id.SetName(worldName);
  _id.SetVersionStr(worldVersion);

  return true;
}

//////////////////////////////////////////////////
bool FuelClient::ParseModelFileUrl(const common::URI &_modelFileUrl,
    ModelIdentifier &_id, std::string &_filePath)
{
  if (!_modelFileUrl.Valid())
    return false;

  auto urlStr = _modelFileUrl.Str();

  std::smatch match;
  std::string scheme;
  std::string server;
  std::string apiVersion;
  std::string owner;
  std::string modelName;
  std::string modelVersion;
  std::string file;

  if (std::regex_match(urlStr, match, *this->dataPtr->urlModelFileRegex) &&
      match.size() == 8u)
  {
    unsigned int i{1};

    scheme = match[i++];
    server = match[i++];
    apiVersion = match[i++];
    owner = match[i++];
    modelName = match[i++];
    modelVersion = match[i++];
    file = match[i++];
  }
  else
  {
    return false;
  }

  // Get remaining server information from config
  common::URI serverUri;
  serverUri.SetScheme(scheme);
  serverUri.SetAuthority(gz::common::URIAuthority("//" + server));

  _id.Server().SetUrl(serverUri);
  _id.Server().SetVersion(apiVersion);
  for (const auto &s : this->dataPtr->config.Servers())
  {
    // cppcheck-suppress useStlAlgorithm
    if (s.Url().Str() == _id.Server().Url().Str()) {
      if (!apiVersion.empty() && s.Version() != _id.Server().Version())
      {
        gzwarn << "Requested server API version [" << apiVersion
                << "] for server [" << s.Url().Str() << "], but will use ["
                << s.Version() << "] as given in the config file."
                << std::endl;
      }
      _id.Server() = s;
      break;
    }
  }

  if (_id.Server().Version().empty())
  {
    gzwarn << "Server configuration is incomplete:" << std::endl
            << _id.Server().AsString();
  }

  _id.SetOwner(owner);
  _id.SetName(modelName);
  _id.SetVersionStr(modelVersion);
  _filePath = file;

  return true;
}

//////////////////////////////////////////////////
bool FuelClient::ParseWorldFileUrl(const common::URI &_worldFileUrl,
    WorldIdentifier &_id, std::string &_filePath)
{
  if (!_worldFileUrl.Valid())
    return false;

  auto urlStr = _worldFileUrl.Str();

  std::smatch match;
  std::string scheme;
  std::string server;
  std::string apiVersion;
  std::string owner;
  std::string worldName;
  std::string worldVersion;
  std::string file;

  if (std::regex_match(urlStr, match, *this->dataPtr->urlWorldFileRegex) &&
      match.size() == 8u)
  {
    unsigned int i{1};

    scheme = match[i++];
    server = match[i++];
    apiVersion = match[i++];
    owner = match[i++];
    worldName = match[i++];
    worldVersion = match[i++];
    file = match[i++];
  }
  else
  {
    return false;
  }

  // Get remaining server information from config
  common::URI serverUri;
  serverUri.SetScheme(scheme);
  serverUri.SetAuthority(gz::common::URIAuthority("//" + server));

  _id.Server().SetUrl(serverUri);
  _id.Server().SetVersion(apiVersion);
  for (const auto &s : this->dataPtr->config.Servers())
  {
    // cppcheck-suppress useStlAlgorithm
    if (s.Url() == _id.Server().Url()) {
      if (!apiVersion.empty() && s.Version() != _id.Server().Version())
      {
        gzwarn << "Requested server API version [" << apiVersion
                << "] for server [" << s.Url().Str() << "], but will use ["
                << s.Version() << "] as given in the config file."
                << std::endl;
      }
      _id.Server() = s;
      break;
    }
  }

  if (_id.Server().Version().empty())
  {
    gzwarn << "Server configuration is incomplete:" << std::endl
            << _id.Server().AsString();
  }

  _id.SetOwner(owner);
  _id.SetName(worldName);
  _id.SetVersionStr(worldVersion);
  _filePath = file;

  return true;
}

//////////////////////////////////////////////////
bool FuelClient::ParseCollectionUrl(const common::URI &_url,
    CollectionIdentifier &_id)
{
  if (!_url.Valid())
    return false;

  auto urlStr = _url.Str();

  std::smatch match;
  std::string scheme;
  std::string server;
  std::string apiVersion;
  std::string owner;
  std::string collectionName;

  bool result =
      std::regex_match(urlStr, match, *this->dataPtr->urlCollectionRegex);

  if (result && match.size() >= 5u)
  {
    unsigned int i{1};

    scheme = match[i++];
    server = match[i++];
    apiVersion = match[i++];
    owner = match[i++];
    collectionName = match[i++];
  }
  else
  {
    return false;
  }

  // Get remaining server information from config
  common::URI serverUri;
  serverUri.SetScheme(scheme);
  serverUri.SetAuthority(gz::common::URIAuthority("//" + server));

  _id.Server().SetUrl(serverUri);
  _id.Server().SetVersion(apiVersion);
  for (const auto &s : this->dataPtr->config.Servers())
  {
    // cppcheck-suppress useStlAlgorithm
    if (s.Url() == _id.Server().Url()) {
      if (!apiVersion.empty() && s.Version() != _id.Server().Version())
      {
        gzwarn << "Requested server API version [" << apiVersion
                << "] for server [" << s.Url().Str() << "], but will use ["
                << s.Version() << "] as given in the config file."
                << std::endl;
      }
      _id.Server() = s;
      break;
    }
  }

  if (_id.Server().Version().empty())
  {
    gzwarn << "Server configuration is incomplete:" << std::endl
            << _id.Server().AsString();
  }

  _id.SetOwner(owner);
  _id.SetName(collectionName);

  return true;
}


//////////////////////////////////////////////////
Result FuelClient::DownloadModel(const common::URI &_modelUrl,
  std::string &_path)
{
  // Get data from URL
  ModelIdentifier id;
  if (!this->ParseModelUrl(_modelUrl, id))
  {
    return Result(ResultType::FETCH_ERROR);
  }

  // Download
  Result result = this->DownloadModel(id);
  if (!result)
    return result;

  // We need to figure out the version for the tip
  if (id.Version() == 0 || id.VersionStr() == "tip")
  {
    Model model = this->dataPtr->cache->MatchingModel(id);
    id.SetVersion(model.Identification().Version());
  }

  _path = gz::common::joinPaths(this->Config().CacheLocation(),
      id.UniqueName(), id.VersionStr());

  return result;
}

//////////////////////////////////////////////////
Result FuelClient::DownloadWorld(const common::URI &_worldUrl,
  std::string &_path)
{
  // Get data from URL
  WorldIdentifier id;
  if (!this->ParseWorldUrl(_worldUrl, id))
  {
    return Result(ResultType::FETCH_ERROR);
  }

  // Download
  auto result = this->DownloadWorld(id);
  if (!result)
    return result;

  _path = id.LocalPath();

  return result;
}

//////////////////////////////////////////////////
Result FuelClient::CachedModel(const ModelIdentifier &_id,
                               std::string &_path)
{
  auto modelIter = this->dataPtr->cache->MatchingModel(_id);
  if (modelIter)
  {
    _path = modelIter.PathToModel();
    return Result(ResultType::FETCH_ALREADY_EXISTS);
  }
  return Result(ResultType::FETCH_ERROR);
}

//////////////////////////////////////////////////
bool FuelClient::CachedModel(const common::URI &_modelUrl)
{
  std::string path;
  return this->CachedModel(_modelUrl, path).Type() != ResultType::FETCH_ERROR;
}

//////////////////////////////////////////////////
Result FuelClient::CachedModel(const common::URI &_modelUrl,
  std::string &_path)
{
  // Get data from URL
  ModelIdentifier id;
  if (!this->ParseModelUrl(_modelUrl, id))
  {
    return Result(ResultType::FETCH_ERROR);
  }

  return this->CachedModel(id, _path);
}

//////////////////////////////////////////////////
bool FuelClient::CachedWorld(const common::URI &_worldUrl)
{
  // Get data from URL
  WorldIdentifier id;
  if (!this->ParseWorldUrl(_worldUrl, id))
    return Result(ResultType::FETCH_ERROR);

  // Check local cache
  return this->dataPtr->cache->MatchingWorld(id);
}

//////////////////////////////////////////////////
Result FuelClient::CachedWorld(const common::URI &_worldUrl,
  std::string &_path)
{
  // Get data from URL
  WorldIdentifier id;
  if (!this->ParseWorldUrl(_worldUrl, id))
  {
    return Result(ResultType::FETCH_ERROR);
  }

  // Check local cache
  auto success = this->dataPtr->cache->MatchingWorld(id);
  if (success)
  {
    _path = id.LocalPath();
    return Result(ResultType::FETCH_ALREADY_EXISTS);
  }

  return Result(ResultType::FETCH_ERROR);
}

//////////////////////////////////////////////////
Result FuelClient::CachedModelFile(const common::URI &_fileUrl,
  std::string &_path)
{
  // Get data from URL
  ModelIdentifier id;
  std::string filePath;
  if (!this->ParseModelFileUrl(_fileUrl, id, filePath))
    return Result(ResultType::FETCH_ERROR);

  if (filePath.empty())
    return Result(ResultType::FETCH_ERROR);

  // Look for model
  auto modelIter = this->dataPtr->cache->MatchingModel(id);

  if (!modelIter)
    return Result(ResultType::FETCH_ERROR);

  auto modelPath = modelIter.PathToModel();

  // Check if file exists
  filePath = common::joinPaths(modelPath, filePath);

  std::vector<std::string> tokens = gz::common::split(filePath, "/");
  std::string sTemp;
  for (auto s : tokens)
    sTemp = gz::common::joinPaths(sTemp, s);
  filePath = sTemp;

  if (common::exists(filePath))
  {
    _path = filePath;
    return Result(ResultType::FETCH_ALREADY_EXISTS);
  }

  return Result(ResultType::FETCH_ERROR);
}

//////////////////////////////////////////////////
Result FuelClient::CachedWorldFile(const common::URI &_fileUrl,
  std::string &_path)
{
  // Get data from URL
  WorldIdentifier id;
  std::string filePath;
  if (!this->ParseWorldFileUrl(_fileUrl, id, filePath))
    return Result(ResultType::FETCH_ERROR);

  if (filePath.empty())
    return Result(ResultType::FETCH_ERROR);

  // Look for world
  auto success = this->dataPtr->cache->MatchingWorld(id);

  if (!success)
    return Result(ResultType::FETCH_ERROR);

  auto worldPath = id.LocalPath();

  // Check if file exists
  filePath = common::joinPaths(worldPath, filePath);

  if (common::exists(filePath))
  {
    _path = filePath;
    return Result(ResultType::FETCH_ALREADY_EXISTS);
  }

  return Result(ResultType::FETCH_ERROR);
}

//////////////////////////////////////////////////
Result FuelClient::PatchModel(
    const gz::fuel_tools::ModelIdentifier &_model,
    const std::vector<std::string> &_headers)
{
  return this->PatchModel(_model, _headers, "");
}

//////////////////////////////////////////////////
Result FuelClient::PatchModel(
    const gz::fuel_tools::ModelIdentifier &_model,
    const std::vector<std::string> &_headers,
    const std::string &_pathToModelDir)
{
  gz::fuel_tools::Rest rest;
  RestResponse resp;

  auto serverUrl = _model.Server().Url().Str();
  auto version = _model.Server().Version();
  common::URIPath path;
  path = path / _model.Owner() / "models" / _model.Name();

  std::multimap<std::string, std::string> form;

  if (!_pathToModelDir.empty() &&
      !this->dataPtr->FillModelForm(_pathToModelDir, _model,
        _model.Private(), _model.Owner(), form))
  {
    return Result(ResultType::UPLOAD_ERROR);
  }
  else
  {
    form.emplace("private", _model.Private() ? "1" : "0");
  }

  std::vector<std::string> headersIncludingServerConfig = _headers;
  AddServerConfigParametersToHeaders(
    _model.Server(), headersIncludingServerConfig);
  resp = rest.Request(HttpMethod::PATCH_FORM, serverUrl, version,
      path.Str(), {}, headersIncludingServerConfig, "", form);

  if (resp.statusCode != 200)
    return Result(ResultType::PATCH_ERROR);

  return Result(ResultType::PATCH);
}

//////////////////////////////////////////////////
void FuelClientPrivate::AllFiles(const std::string &_path,
    std::vector<std::string> &_files) const
{
  common::DirIter dirIter(_path);
  common::DirIter end;
  while (dirIter != end)
  {
    if (common::isDirectory(*dirIter))
      this->AllFiles(*dirIter, _files);
    else
      _files.push_back(*dirIter);

    ++dirIter;
  }
}

//////////////////////////////////////////////////
void FuelClient::PopulateLicenses(const ServerConfig &_server)
{
  this->dataPtr->PopulateLicenses(_server);
}

//////////////////////////////////////////////////
bool FuelClientPrivate::FillModelForm(const std::string &_pathToModelDir,
    const ModelIdentifier &_id, bool _private, const std::string &_owner,
    std::multimap<std::string, std::string> &_form)
{
  if (!common::exists(_pathToModelDir))
  {
    gzerr << "The model path[" << _pathToModelDir << "] doesn't exist.\n";
    return false;
  }

  gz::msgs::FuelMetadata meta;

  // Try the `metadata.pbtxt` file first since it contains more information
  // than `model.config`.
  if (common::exists(common::joinPaths(_pathToModelDir, "metadata.pbtxt")))
  {
    std::string filePath = common::joinPaths(_pathToModelDir, "metadata.pbtxt");

    gzdbg << "Parsing " << filePath  << std::endl;

    // Read the pbtxt file.
    std::ifstream inputFile(filePath);
    std::string inputStr((std::istreambuf_iterator<char>(inputFile)),
        std::istreambuf_iterator<char>());

    // Parse the file into the fuel metadata message
    if (!google::protobuf::TextFormat::ParseFromString(inputStr, &meta))
    {
      gzerr << "Unable to parse fuel metadata message [" << filePath << ']'
            << std::endl;
      return false;
    }
  }
  else if (common::exists(common::joinPaths(_pathToModelDir, "model.config")))
  {
    std::string filePath = common::joinPaths(_pathToModelDir, "model.config");

    gzdbg << "Parsing " << filePath << std::endl;

    std::ifstream inputFile(filePath);
    std::string inputStr((std::istreambuf_iterator<char>(inputFile)),
        std::istreambuf_iterator<char>());

    if (!gz::msgs::ConvertFuelMetadata(inputStr, meta))
    {
      gzerr << "Unable to convert model config[" << _pathToModelDir << "].\n";
      return false;
    }
  }
  else
  {
    gzerr << "Provided model directory[" <<  _pathToModelDir
      << "] needs a metadata.pbtxt or a model.config file.";
    return false;
  }

  _form =
  {
    {"name", meta.name()},
    {"description", meta.description()},
    {"private", _private ? "1" : "0"},
  };

  // Add owner if specified.
  if (!_owner.empty())
  {
    _form.emplace("owner", _owner);
  }

  // \todo(nkoenig) The gz-fuelserver expects an integer number for the
  // license information. The fuelserver should be modified to accept
  // a string. Otherwise, we have to bake into each client a mapping of
  // license name to integer.
  //
  // If we have legal, then attempt to fill in the correct license information.
  if (meta.has_legal())
  {
    // Attempt to retrieve the available licenses, if we have no available
    // licenses.
    if (this->licenses.empty())
    {
      this->PopulateLicenses(_id.Server());
      // Fail if a license has been requested, but we couldn't get the
      // available licenses.
      if (this->licenses.empty())
      {
        return false;
      }
    }

    // Find the license by name.
    std::map<std::string, unsigned int>::const_iterator licenseIt =
      this->licenses.find(meta.legal().license());
    if (licenseIt != this->licenses.end())
    {
      _form.emplace("license", std::to_string(licenseIt->second));
    }
    // No license found, print an error and return.
    else
    {
      std::string validLicenseNames;
      auto end = this->licenses.end();
      std::advance(end, -1);
      for (licenseIt = this->licenses.begin(); licenseIt != end; ++licenseIt)
      {
        validLicenseNames += "    " + licenseIt->first + "\n";
      }
      validLicenseNames += "    " + licenseIt->first;

      gzerr << "Invalid license[" << meta.legal().license() << "].\n"
             << "  Valid licenses include:\n"
             << validLicenseNames << std::endl;

      return false;
    }
  }
  // If there is no license information, then default to
  // "Creative Commons - Public Domain"
  else
  {
    _form.emplace("license", "1");
  }

  // Add tags
  std::string tags;
  for (int i = 0; i < meta.tags_size(); ++i)
    tags += meta.tags(i) + ",";
  if (!tags.empty())
    _form.emplace("tags", tags);

  // Add categories
  std::string categories;
  if (meta.has_categories())
  {
    // Add the first category, if present.
    if (!meta.categories().first().empty())
      categories = meta.categories().first();

    // Add the second category, if present.
    if (!meta.categories().second().empty())
    {
      // Add a comma separator if the first category was not empty.
      if (!categories.empty())
        categories += ", ";
      categories += meta.categories().second();
    }
  }
  if (!categories.empty())
    _form.emplace("categories", categories);

  // Add annotations as metadata.
  for (const auto &annotation : meta.annotations())
  {
    std::string formAnnotation = std::string("{\"key\":\"") +
      annotation.first + "\",\"value\":\"" + annotation.second + "\"}";
    _form.emplace("metadata", formAnnotation);
  }

  // Recursively get all the files.
  std::vector<std::string> files;
  this->AllFiles(_pathToModelDir, files);
  for (const std::string &file : files)
  {
    _form.emplace("file", std::string("@") + file + ";"
        + file.substr(_pathToModelDir.size()+1));
  }

  return true;
}

//////////////////////////////////////////////////
void FuelClientPrivate::PopulateLicenses(const ServerConfig &_server)
{
  RestResponse resp;

  // Send the request.
  resp = this->rest.Request(HttpMethod::GET, _server.Url().Str(),
      _server.Version(), "licenses", {}, {}, "");
  if (resp.statusCode != 200)
  {
    gzerr << "Failed to get license information from "
      << _server.Url().Str() << "/" << _server.Version() << std::endl;
  }
  else if (!JSONParser::ParseLicenses(resp.data, this->licenses))
  {
    gzerr << "Failed to parse license information[" << resp.data << "]\n";
  }
}

//////////////////////////////////////////////////
bool FuelClient::UpdateModels(const std::vector<std::string> &_headers)
{
  // Get a list of the most recent model versions in the cache.
  std::map<std::string, gz::fuel_tools::ModelIdentifier> toProcess;
  for (ModelIter iter = this->dataPtr->cache->AllModels(); iter; ++iter)
  {
    auto processIter = toProcess.find(iter->Identification().UniqueName());
    if (processIter == toProcess.end() ||
        processIter->second.Version() < iter->Identification().Version())
    {
      toProcess[iter->Identification().UniqueName()] = iter->Identification();
    }
  }

  // Attempt to update each model.
  for (const auto &id : toProcess)
  {
    gz::fuel_tools::ModelIdentifier cloudId;

    if (!this->ModelDetails(id.second, cloudId, _headers))
    {
      gzerr << "Failed to fetch model details for model["
        << id.second.Owner()  << "/" << id.second.Name() << "]\n";
    }
    else if (id.second.Version() < cloudId.Version())
    {
      gzmsg << "Updating model " << id.second.Owner() << "/"
        << id.second.Name() << " up to version "
        << cloudId.Version() << std::endl;
      this->DownloadModel(cloudId, _headers);
    }
    else
    {
      gzmsg << "Model " << id.second.Owner() << "/"
        << id.second.Name() << " is up to date." << std::endl;
    }
  }
  return true;
}

//////////////////////////////////////////////////
bool FuelClient::UpdateWorlds(const std::vector<std::string> &_headers)
{
  // Get a list of the most recent world versions in the cache.
  std::map<std::string, gz::fuel_tools::WorldIdentifier> toProcess;
  for (WorldIter iter = this->dataPtr->cache->AllWorlds(); iter; ++iter)
  {
    auto processIter = toProcess.find(iter->UniqueName());
    if (processIter == toProcess.end() ||
        processIter->second.Version() < iter->Version())
    {
      toProcess[iter->UniqueName()] = *iter;
    }
  }

  // Attempt to update each world.
  for (const auto &id : toProcess)
  {
    gz::fuel_tools::WorldIdentifier cloudId;

    if (!this->WorldDetails(id.second, cloudId, _headers))
    {
      gzerr << "Failed to fetch world details for world["
        << id.second.Owner() << "/" << id.second.Name() << "]\n";
    }
    else if (id.second.Version() < cloudId.Version())
    {
      gzmsg << "Updating world " << id.second.Owner() << "/"
        << id.second.Name() << " up to version "
        << cloudId.Version() << std::endl;
      this->DownloadWorld(cloudId, _headers);
    }
    else
    {
      gzmsg << "World " << id.second.Owner() << "/"
        << id.second.Name() << " is up to date." << std::endl;
    }
  }
  return true;
}

//////////////////////////////////////////////////
void FuelClientPrivate::ZipFromResponse(const RestResponse &_resp,
    std::string &_zip)
{
  // Check the content-type which could be empty (ideally not):
  //   * text/plain indicates the data is a download link.
  //   * application/zip indicates the data is a zip file.
  //   * binary/octet-stream indicates the data is a zip file.
  auto contentTypeIter = _resp.headers.find("Content-Type");
  if (contentTypeIter != _resp.headers.end())
  {
    // If text/plain then data might be a link to a zip file.
    if (contentTypeIter->second.find("text/plain") != std::string::npos)
    {
      std::string linkUri = _resp.data;

      // Check for valid URI
      if (common::URI::Valid(linkUri))
      {
        gzdbg << "Downloading from a referral link [" << linkUri << "]\n";
        // Get the zip data.
        RestResponse linkResp = rest.Request(HttpMethod::GET,
            // URL
            linkUri,
            // Version
            "",
            // Path
            "",
            // Query strings
            {},
            // Headers
            {},
            // Data
            "");

        return this->ZipFromResponse(linkResp, _zip);
      }
      else
      {
        gzerr << "Invalid referral link URI [" << linkUri << "]. "
          << "Unable to download.\n";
      }
    }
    else if (contentTypeIter->second.find("application/zip") !=
             std::string::npos ||
             contentTypeIter->second.find("binary/octet-stream") !=
             std::string::npos)
    {
      _zip = std::move(_resp.data);
    }
    else
    {
      gzerr << "Invalid content-type of [" << contentTypeIter->second << "]. "
        << "Unable to download.\n";
    }
  }
  else
  {
    // If content-type is missing, then assume the data is the zip file.
    _zip = std::move(_resp.data);
  }
}
}  // namespace gz::fuel_tools

