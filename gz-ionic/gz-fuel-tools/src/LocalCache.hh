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

#ifndef GZ_FUEL_TOOLS_LOCALCACHE_HH_
#define GZ_FUEL_TOOLS_LOCALCACHE_HH_

#include <memory>
#include <string>

#include "gz/fuel_tools/Helpers.hh"
#include "gz/fuel_tools/Model.hh"
#include "gz/fuel_tools/ModelIter.hh"
#include "gz/fuel_tools/WorldIter.hh"

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::shared_ptr
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

namespace gz::fuel_tools
{
  /// \brief Forward declaration
  class ClientConfig;
  class LocalCachePrivate;
  class ModelIdentifier;

  /// \brief Class for managing stuff in the local cache
  class GZ_FUEL_TOOLS_VISIBLE LocalCache
  {
    /// \brief Constructor
    /// \param[in] _config The configuration for the client
    public: explicit LocalCache(const ClientConfig *_config);

    /// \brief destructor
    public: virtual ~LocalCache();

    /// \brief Get all models in offline cache
    /// \return Model iterator
    public: virtual ModelIter AllModels();

    /// \brief Get all worlds in offline cache
    /// \return World iterator
    public: virtual WorldIter AllWorlds() const;

    /// \brief Get the first model matching all fields on an id.
    /// \param[in] _id An id with ServerUrl, Owner, and Name all set
    /// \return A model which matches all of _id's parameters.
    public: virtual Model MatchingModel(const ModelIdentifier &_id);

    /// \brief Get the first world matching all fields on an id.
    /// \param[in] _id An id with ServerUrl, Owner, and Name all set
    /// \return A world which matches all of _id's parameters.
    public: virtual bool MatchingWorld(WorldIdentifier &_id) const;

    /// \brief Get all models partially matching an ID
    /// \param[in] _id An id with at least one of ServerURL, Owner, and Name
    /// \return An iterator with all models that match all fields that are
    /// set on _id.
    public: virtual ModelIter MatchingModels(const ModelIdentifier &_id);

    /// \brief Get all worlds partially matching an ID
    /// \param[in] _id An id with at least one of ServerUrl, Owner, and Name
    /// \return An iterator with all worlds that match all fields that are
    /// set on _id.
    public: virtual WorldIter MatchingWorlds(
        const WorldIdentifier &_id) const;

    /// \brief Add a model from packed data to the local cache
    /// \param[in] _id A completely populated ID
    /// \param[in] _data Compressed content of the model
    /// \param[in] _overwrite Overwrite model if already exists.
    /// \returns True if the model was successfully added to the local cache,
    /// and the model contains a model.config file.
    public: virtual bool SaveModel(
        const ModelIdentifier &_id,
        const std::string &_data,
        const bool _overwrite);

    /// \brief Add a world from packed data to the local cache
    /// \param[out] _id A completely populated ID
    /// \param[in] _data Compressed content of the world
    /// \param[in] _overwrite Overwrite world if already exists.
    /// \returns True if the world was successfully added to the local cache
    public: virtual bool SaveWorld(
        WorldIdentifier &_id,
        const std::string &_data,
        const bool _overwrite);

    /// \brief Internal data.
    private: std::shared_ptr<LocalCachePrivate> dataPtr;
  };
}  // namespace gz::fuel_tools

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif  // GZ_FUEL_TOOLS_LOCALCACHE_HH_
