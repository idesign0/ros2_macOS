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

#ifndef GZ_FUEL_TOOLS_FUELCLIENT_HH_
#define GZ_FUEL_TOOLS_FUELCLIENT_HH_

#include <memory>
#include <string>
#include <tuple>
#include <vector>
#include <gz/common/URI.hh>

#include "gz/fuel_tools/ModelIter.hh"
#include "gz/fuel_tools/RestClient.hh"
#include "gz/fuel_tools/Result.hh"
#include "gz/fuel_tools/WorldIter.hh"

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::unique_ptr
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

namespace gz::fuel_tools
{
  /// \brief Forward Declaration
  class ClientConfig;
  class CollectionIdentifier;
  class FuelClientPrivate;
  class LocalCache;
  class ModelIdentifier;
  class ServerConfig;
  class WorldIdentifier;

  /// \brief High level interface to Gazebo Fuel
  class GZ_FUEL_TOOLS_VISIBLE FuelClient
  {
    /// \brief Default constructor.
    public: FuelClient();

    /// \brief Constructor accepts server and auth configuration
    /// \param[in] _config configuration about servers to connect to
    /// \param[in] _rest A REST request.
    /// \remarks the client saves a copy of the config passed into it
    public: FuelClient(const ClientConfig &_config,
                       const Rest &_rest = Rest());

    /// \brief Destructor
    public: ~FuelClient();

    /// \brief Get a mutable reference to the client configuration.
    /// \return Mutable reference to the client configuration.
    public: ClientConfig &Config();

    /// \brief Fetch the details of a model.
    /// \param[in] _id a partially filled out identifier used to fetch models
    /// \remarks Fulfills Get-One requirement
    /// \param[out] _model The requested model
    /// \return Result of the fetch operation.
    public: Result ModelDetails(const ModelIdentifier &_id,
                                ModelIdentifier &_model) const;

    /// \brief Fetch the details of a model.
    /// \param[in] _id a partially filled out identifier used to fetch models
    /// \remarks Fulfills Get-One requirement
    /// \param[out] _model The requested model
    /// \param[in] _headers Headers to set on the HTTP request.
    /// \return Result of the fetch operation.
    public: Result ModelDetails(const ModelIdentifier &_id,
                ModelIdentifier &_model,
                const std::vector<std::string> &_headers) const;


    /// \brief Returns an iterator that can return names of models
    /// \remarks Fulfills Get-All requirement
    /// \remarks an iterator instead of a list of names is returned in case
    ///          the model names api supports pagination in the future. The
    ///          iterator may fetch more names if code continues to request
    ///          it. The initial API appears to return all of the models, so
    ///          right now this iterator stores a list of names internally.
    /// \param[in] _server The server to request the operation.
    /// \return A model iterator
    public: ModelIter Models(const ServerConfig &_server);

    /// \brief Returns an iterator that can return names of models
    /// \remarks Fulfills Get-All requirement
    /// \remarks an iterator instead of a list of names is returned in case
    ///          the model names api supports pagination in the future. The
    ///          iterator may fetch more names if code continues to request
    ///          it. The initial API appears to return all of the models, so
    ///          right now this iterator stores a list of names internally.
    /// \param[in] _server The server to request the operation.
    /// \return A model iterator
    public: ModelIter Models(const ServerConfig &_server) const;

    /// \brief Fetch the details of a world.
    /// \param[in] _id a partially filled out identifier used to fetch worlds
    /// \param[out] _world The requested world
    /// \return Result of the fetch operation.
    public: Result WorldDetails(const WorldIdentifier &_id,
                                WorldIdentifier &_world) const;

    /// \brief Fetch the details of a world.
    /// \param[in] _id a partially filled out identifier used to fetch worlds
    /// \param[out] _world The requested world
    /// \param[in] _headers Headers to set on the HTTP request.
    /// \return Result of the fetch operation.
    public: Result WorldDetails(const WorldIdentifier &_id,
                              WorldIdentifier &_world,
                              const std::vector<std::string> &_headers) const;

    /// \brief Returns an iterator that can return information of worlds
    /// \remarks An iterator instead of a list of names, to be able to
    ///          handle pagination. The iterator may fetch more names if
    ///          code continues to request it.
    /// \param[in] _server The server to request the operation.
    /// \return A world iterator
    public: WorldIter Worlds(const ServerConfig &_server) const;

    /// \brief Returns models matching a given identifying criteria
    /// \param[in] _id a partially filled out identifier used to fetch models
    /// \remarks Fulfills Get-One requirement
    /// \remarks Model names are unique to the owner, so this API
    ///          allows the possibility of getting multiple models with the
    ///          same name if the owner is not specified.
    /// \return An iterator of models with names matching the criteria
    public: ModelIter Models(const ModelIdentifier &_id);

    /// \brief Returns models matching a given identifying criteria
    /// \param[in] _id a partially filled out identifier used to fetch models
    /// \remarks Fulfills Get-One requirement
    /// \remarks Model names are unique to the owner, so this API
    ///          allows the possibility of getting multiple models with the
    ///          same name if the owner is not specified.
    /// \return An iterator of models with names matching the criteria
    public: ModelIter Models(const ModelIdentifier &_id) const;

    /// \brief Returns models matching a given identifying criteria
    /// \param[in] _id a partially filled out identifier used to fetch models
    /// \param[in] _checkCache Whether to check the cache.
    /// \remarks Fulfills Get-One requirement
    /// \remarks Model names are unique to the owner, so this API
    ///          allows the possibility of getting multiple models with the
    ///          same name if the owner is not specified.
    /// \return An iterator of models with names matching the criteria
    public: ModelIter Models(const ModelIdentifier &_id, bool _checkCache);

    /// \brief Returns models matching a given identifying criteria
    /// \param[in] _id a partially filled out identifier used to fetch models
    /// \param[in] _checkCache Whether to check the cache.
    /// \remarks Fulfills Get-One requirement
    /// \remarks Model names are unique to the owner, so this API
    ///          allows the possibility of getting multiple models with the
    ///          same name if the owner is not specified.
    /// \return An iterator of models with names matching the criteria
    public: ModelIter Models(const ModelIdentifier &_id,
                             bool _checkCache) const;

     /// \brief Returns an iterator for the models found in a collection.
     /// \param[in] _id a partially filled out identifier used to fetch a
     /// collection.
     /// \return An iterator of models in the collection.
    public: ModelIter Models(const CollectionIdentifier &_id) const;

    /// \brief Returns worlds matching a given identifying criteria
    /// \param[in] _id A partially filled out identifier used to fetch worlds
    /// \return An iterator of worlds with names matching the criteria
    public: WorldIter Worlds(const WorldIdentifier &_id) const;

    /// \brief Returns an iterator for the worlds found in a collection.
    /// \param[in] _id a partially filled out identifier used to fetch a
    /// collection.
    /// \return An iterator of words in the collection.
    public: WorldIter Worlds(const CollectionIdentifier &_id) const;

    /// \brief Upload a directory as a new model
    /// \param[in] _pathToModelDir a path to a directory containing a model
    /// \param[in] _id An identifier to assign to this new model
    /// \param[in] _headers Headers to set on the HTTP request.
    /// \param[in] _private True to make the model private.
    /// \return Result of the upload operation
    public: Result UploadModel(const std::string &_pathToModelDir,
                               const ModelIdentifier &_id,
                               const std::vector<std::string> &_headers,
                               bool _private = false);

    /// \brief Upload a directory as a new model
    /// \param[in] _pathToModelDir a path to a directory containing a model
    /// \param[in] _id An identifier to assign to this new model
    /// \param[in] _headers Headers to set on the HTTP request.
    /// \param[in] _private True to make the model private.
    /// \param[in] _owner Name of the owner. Empty string indicates that
    /// the owner is specified by the private token in the headers.
    /// \return Result of the upload operation
    public: Result UploadModel(const std::string &_pathToModelDir,
                               const ModelIdentifier &_id,
                               const std::vector<std::string> &_headers,
                               bool _private,
                               const std::string &_owner);

    /// \brief Remove a resource, such as a model or world, from Gazebo Fuel
    /// \param[in] _uri The full URI of the resource, e.g:
    /// https://fuel.gazebosim.org/1.0/openrobotcs/model/my_model
    /// \param[in] _headers Headers to set on the HTTP request.
    /// \return Result of the delete operation
    public: Result DeleteUrl(const gz::common::URI &_uri,
                const std::vector<std::string> &_headers);

    /// \brief Download a model from Gazebo Fuel. This will override an
    /// existing local copy of the model.
    /// \param[in] _id The model identifier.
    /// \return Result of the download operation
    public: Result DownloadModel(const ModelIdentifier &_id);

    /// \brief Download a model from Gazebo Fuel. This will override an
    /// existing local copy of the model.
    /// \param[in] _id The model identifier.
    /// \param[in] _headers Headers to set on the HTTP request.
    /// \return Result of the download operation
    public: Result DownloadModel(const ModelIdentifier &_id,
                const std::vector<std::string> &_headers);

    /// \brief Download a model from Gazebo Fuel. This will override an
    /// existing local copy of the model.
    /// \param[in] _id The model identifier.
    /// \param[in] _headers Headers to set on the HTTP request.
    /// \param[out] _dependencies List of models that this model depends on.
    /// \return Result of the download operation
    public: Result DownloadModel(const ModelIdentifier &_id,
                const std::vector<std::string> &_headers,
                std::vector<ModelIdentifier> &_dependencies);

    /// \brief Retrieve the list of dependencies for a model.
    /// \param[in] _id The model identifier.
    /// \param[out] _dependencies The list of dependencies.
    /// \return Result of the operation
    public: Result ModelDependencies(const ModelIdentifier &_id,
                std::vector<ModelIdentifier> &_dependencies);

    /// \brief Retrieve the list of dependencies for a list of models.
    /// \param[in] _id The list of model identifiers.
    /// \param[out] _dependencies The list of dependencies.
    /// \return Result of the operation
    public: Result ModelDependencies(
                const std::vector<ModelIdentifier> &_id,
                std::vector<ModelIdentifier> &_dependencies);

    /// \brief Download a world from Gazebo Fuel. This will override an
    /// existing local copy of the world.
    /// \param[out] _id The world identifier, with local path updated.
    /// \return Result of the download operation
    public: Result DownloadWorld(WorldIdentifier &_id);

    /// \brief Download a world from Gazebo Fuel. This will override an
    /// existing local copy of the world.
    /// \param[out] _id The world identifier, with local path updated.
    /// \param[in] _headers Headers to set on the HTTP request.
    /// \return Result of the download operation
    public: Result DownloadWorld(WorldIdentifier &_id,
                const std::vector<std::string> &_headers);

    /// \brief Download a model from Gazebo Fuel. This will override an
    /// existing local copy of the model.
    /// \param[in] _modelUrl The unique URL of the model to download.
    /// E.g.: https://fuel.gazebosim.org/1.0/caguero/models/Beer
    /// \param[out] _path Path where the model was downloaded.
    /// \return Result of the download operation.
    public: Result DownloadModel(const common::URI &_modelUrl,
                                 std::string &_path);

    /// \brief Download a world from Gazebo Fuel. This will override an
    /// existing local copy of the world.
    /// \param[in] _worldUrl The unique URL of the world to download.
    /// E.g.: https://fuel.gazebosim.org/1.0/OpenRobotics/worlds/Empty
    /// \param[out] _path Path where the world was downloaded.
    /// \return Result of the download operation.
    public: Result DownloadWorld(const common::URI &_worldUrl,
                                 std::string &_path);

    using ModelResult = std::tuple<ModelIdentifier, Result>;

    /// \brief Download a list of models from Gazebo Fuel.
    /// \param[in] _ids The list of model ids to download.
    ///   This will also find all recursive dependencies of the models
    /// \param[in] _jobs Number of parallel jobs to use to download models
    /// \return Result of the download operation.
    //    The resulting vector will be at least the size of the _ids input
    //    vector, but may be larger depending on the number of dependencies
    //    downloaded
    public: std::vector<ModelResult> DownloadModels(
                const std::vector<ModelIdentifier> &_ids,
                size_t _jobs = 2);

    /// \brief Download a list of mworlds from Gazebo Fuel.
    /// \param[in] _ids The list of world ids to download.
    /// \param[in] _jobs Number of parallel jobs to use to download worlds.
    /// \return Result of the download operation.
    public: Result DownloadWorlds(
                const std::vector<WorldIdentifier> &_ids,
                size_t _jobs = 2);

    /// \brief Check if a model is already present in the local cache.
    /// \param[in] _id The model identifier
    /// \param[out] _path Local path where the model can be found.
    /// \return FETCH_ERROR if not cached, FETCH_ALREADY_EXISTS if cached.
    public: Result CachedModel(const ModelIdentifier &_id,
                               std::string &_path);

    /// \brief Check if a model is already present in the local cache.
    /// \param[in] _modelUrl The unique URL of the model on a Fuel server.
    /// E.g.: https://fuel.gazebosim.org/1.0/caguero/models/Beer
    /// \param[out] _path Local path where the model can be found.
    /// \return FETCH_ERROR if not cached, FETCH_ALREADY_EXISTS if cached.
    public: Result CachedModel(const common::URI &_modelUrl,
                               std::string &_path);

    /// \brief Check if a model exists in the cache.
    /// \param[in] _modelUrl The unique URL of the world on a Fuel server.
    /// E.g.: https://fuel.gazebosim.org/1.0/caguero/models/Beer
    /// \return True if the model exists in the cache, false otherwise.
    public: bool CachedModel(const common::URI &_modelUrl);

    /// \brief Check if a world is already present in the local cache.
    /// \param[in] _worldUrl The unique URL of the world on a Fuel server.
    /// E.g.: https://fuel.gazebosim.org/1.0/OpenRobotics/worlds/Empty
    /// \param[out] _path Local path where the world can be found.
    /// \return FETCH_ERROR if not cached, FETCH_ALREADY_EXISTS if cached.
    public: Result CachedWorld(const common::URI &_worldUrl,
                               std::string &_path);

    /// \brief Check if a world exists in the cache.
    /// \param[in] _worldUrl The unique URL of the world on a Fuel server.
    /// E.g.: https://fuel.gazebosim.org/1.0/OpenRobotics/worlds/Empty
    /// \return True if the world exists in the cache, false otherwise.
    public: bool CachedWorld(const common::URI &_worldUrl);

    /// \brief Check if a file belonging to a model is already present in the
    /// local cache.
    /// \param[in] _fileUrl The unique URL of the file on a Fuel server. E.g.:
    /// https://server.org/1.0/owner/models/model/files/meshes/mesh.dae
    /// \param[out] _path Local path where the file can be found.
    /// \return FETCH_ERROR if not cached, FETCH_ALREADY_EXISTS if cached.
    public: Result CachedModelFile(const common::URI &_fileUrl,
                                   std::string &_path);

    /// \brief Check if a file belonging to a world is already present in the
    /// local cache.
    /// \param[in] _fileUrl The unique URL of the file on a Fuel server. E.g.:
    /// https://server.org/1.0/owner/worlds/world/files/name.world
    /// \param[out] _path Local path where the file can be found.
    /// \return FETCH_ERROR if not cached, FETCH_ALREADY_EXISTS if cached.
    public: Result CachedWorldFile(const common::URI &_fileUrl,
                                   std::string &_path);

    /// \brief Parse model identifier from model URL or unique name.
    /// \param[in] _modelUrl The unique URL of a model. It may also be a
    /// unique name, which is a URL without the server version.
    /// \param[out] _id The model identifier. It may contain incomplete
    /// information based on the passed URL and the current client
    /// config.
    /// The server version will be overridden if that server is in the config
    /// file.
    /// \return True if parsed successfully.
    public: bool ParseModelUrl(const common::URI &_modelUrl,
                               ModelIdentifier &_id);

    /// \brief Parse world identifier from world URL or unique name.
    /// \param[in] _worldUrl The unique URL of a world. It may also be a
    /// unique name, which is a URL without the server version.
    /// \param[out] _id The world identifier. It may contain incomplete
    /// information based on the passed URL and the current client
    /// config.
    /// The server version will be overridden if that server is in the config
    /// file.
    /// \return True if parsed successfully.
    public: bool ParseWorldUrl(const common::URI &_worldUrl,
                               WorldIdentifier &_id);

    /// \brief Parse model file identifier from model file URL.
    /// \param[in] _modelFileUrl The unique URL of a model file. It may also
    /// be a unique name, which is a URL without the server version.
    /// \param[out] _id The model identifier. It may contain incomplete
    /// information based on the passed URL and the current client
    /// config.
    /// \param[out] _filePath Path to the file from the model's root
    /// directory, such as "meshes/mesh.dae" or "model.sdf".
    /// \return True if parsed successfully.
    public: bool ParseModelFileUrl(const common::URI &_modelFileUrl,
                                   ModelIdentifier &_id,
                                   std::string &_filePath);

    /// \brief Parse world file identifier from world file URL.
    /// \param[in] _worldFileUrl The unique URL of a world file. It may also
    /// be a unique name, which is a URL without the server version.
    /// \param[out] _id The world identifier. It may contain incomplete
    /// information based on the passed URL and the current client
    /// config.
    /// \param[out] _filePath Path to the file from the world's root
    /// directory, such as "name.world"
    /// \return True if parsed successfully.
    public: bool ParseWorldFileUrl(const common::URI &_worldFileUrl,
                                   WorldIdentifier &_id,
                                   std::string &_filePath);

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

    /// \brief Update a model using a PATCH request.
    ///
    /// Model fields that are patched by this function:
    ///   * private
    ///
    /// \param[in] _model The model to patch. The contents of this model
    ///            will be sent in the PATCH request.
    /// \param[in] _headers Headers to set on the HTTP request.
    /// \return Result of the patch operation.
    public: Result PatchModel(
                const gz::fuel_tools::ModelIdentifier &_model,
                const std::vector<std::string> &_headers);

    /// \brief Update a model using a PATCH request.
    ///
    /// Model fields that are patched by this function:
    ///   * private
    ///   * Model files contained in _pathToModelDir.
    ///   * Description, tags, license, and other attributes found in the
    ///     metadata.pbtxt or model.config file.
    ///
    /// \param[in] _model The model to patch. The contents of this model
    ///            will be sent in the PATCH request.
    /// \param[in] _headers Headers to set on the HTTP request.
    /// \param[in] _pathToModelDir a path to a directory containing a model.
    /// \return Result of the patch operation.
    public: Result PatchModel(
                const gz::fuel_tools::ModelIdentifier &_model,
                const std::vector<std::string> &_headers,
                const std::string &_pathToModelDir);

    /// \brief Parse Collection identifier from URL.
    /// \param[in] _url The unique URL of a collection. It may also be a
    /// unique name, which is a URL without the server version.
    /// \param[out] _id The collection identifier. It may contain incomplete
    /// information based on the passed URL and the current client config.
    /// The server version will be overridden if that server is in the config
    /// file.
    /// \return True if parsed successfully.
    public: bool ParseCollectionUrl(const common::URI &_url,
                                    CollectionIdentifier &_id);

    /// \brief Update all models in local cache.
    /// \param[in] _headers Headers to set on the HTTP request.
    /// \return True if everything updated successfully.
    public: bool UpdateModels(const std::vector<std::string> &_headers);

    /// \brief Update all worlds in local cache.
    /// \param[in] _headers Headers to set on the HTTP request.
    /// \return True if everything updated successfully.
    public: bool UpdateWorlds(const std::vector<std::string> &_headers);

    /// \brief Checked if there is any header already specify
    /// \param[in] _serverConfig Server configuration
    /// \param[in,out] _headers Vector with headers to check
    private: void AddServerConfigParametersToHeaders(
      const gz::fuel_tools::ServerConfig &_serverConfig,
      std::vector<std::string> &_headers) const;

    /// \brief PIMPL
    private: std::unique_ptr<FuelClientPrivate> dataPtr;
  };
}  // namespace gz::fuel_tools

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif  // GZ_FUEL_TOOLS_FUELCLIENT_HH_
