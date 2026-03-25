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

#ifndef GZ_FUEL_TOOLS_MODELITERPRIVATE_HH_
#define GZ_FUEL_TOOLS_MODELITERPRIVATE_HH_

#include <cstddef>
#include <string>
#include <vector>

#include "gz/fuel_tools/Model.hh"
#include "gz/fuel_tools/ModelIdentifier.hh"
#include "gz/fuel_tools/RestClient.hh"
#include "gz/fuel_tools/ServerConfig.hh"

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::vector
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

namespace gz::fuel_tools
{
  /// \brief forward declaration
  class ModelIter;

  /// \brief Private class, do not include or instantiate
  class GZ_FUEL_TOOLS_VISIBLE ModelIterFactory
  {
    /// \brief Create a model iterator from a vector of model identifiers
    /// \param[in] _ids Model identifiers
    /// \return Model iterator
    public: static ModelIter Create(const std::vector<ModelIdentifier> &_ids);

    /// \brief Create a model iterator from a vector of models
    /// \param[in] _ids Models
    /// \return Model iterator
    public: static ModelIter Create(const std::vector<Model> &_models);

    /// \brief Create a model iter that will make Rest api calls
    /// \param[in] _rest a Rest request
    /// \param[in] _server The server to request the operation
    /// \param[in] _api The path to request
    public: static ModelIter Create(const Rest &_rest,
                                    const ServerConfig &_server,
                                    const std::string &_api);

    /// \brief Create a model iterator that is empty
    /// \return An empty iterator
    public: static ModelIter Create();
  };

  /// \brief Private class, do not include or instantiate
  class GZ_FUEL_TOOLS_VISIBLE ModelIterPrivate
  {
    /// \brief Destructor
    public: virtual ~ModelIterPrivate();

    /// \brief Advance iterator to next model
    public: virtual void Next() = 0;

    /// \brief True if this iterator has reach the end
    /// \return True if reached end.
    public: virtual bool HasReachedEnd() = 0;

    /// \brief Current model for returning references
    public: Model model;
  };

  /// \brief class for iterating through model ids where all are known
  ///        in advance
  class GZ_FUEL_TOOLS_VISIBLE IterIds : public ModelIterPrivate
  {
    /// \brief Constructor
    public: explicit IterIds(std::vector<ModelIdentifier> _ids);

    /// \brief Destructor
    public: virtual ~IterIds();

    // Documentation inherited
    public: virtual void Next() override;

    // Documentation inherited
    public: virtual bool HasReachedEnd() override;

    /// \brief Model identifiers that have been requested
    protected: std::vector<ModelIdentifier> ids;

    /// \brief Where the current iterator is in the list of ids
    protected: std::vector<ModelIdentifier>::iterator idIter;
  };

  /// \brief class for iterating through model ids where all are known
  ///        in advance
  class GZ_FUEL_TOOLS_VISIBLE IterModels: public ModelIterPrivate
  {
    /// \brief Constructor
    public: explicit IterModels(std::vector<Model> _models);

    /// \brief Destructor
    public: virtual ~IterModels();

    // Documentation inherited
    public: virtual void Next() override;

    // Documentation inherited
    public: virtual bool HasReachedEnd() override;

    /// \brief Models to iterator through
    protected: std::vector<Model> models;

    /// \brief Where the current iterator is in the list of models
    protected: std::vector<Model>::iterator modelIter;
  };

  /// \brief class for iterating through model ids from a rest API
  class GZ_FUEL_TOOLS_HIDDEN IterRestIds: public ModelIterPrivate
  {
    /// \brief constructor
    public: IterRestIds(const Rest &_rest,
                        const ServerConfig &_server,
                        const std::string &_api);

    /// \brief destructor
    public: virtual ~IterRestIds();

    /// \brief Advance iterator to next model.
    public: virtual void Next() override;

    /// \brief True if this iterator has reach the end.
    /// \return True if iterator has reached the end.
    public: virtual bool HasReachedEnd() override;

    /// \brief Client configuration
    public: ServerConfig config;

    /// \brief RESTful client
    public: Rest rest;

    /// \brief The API (path) of the RESTful requests
    public: const std::string api;

    /// \brief Make a RESTful request for the given page
    /// \param[in] _page Page number to request
    /// \return Response from the request
    protected: RestResponse MakeRestRequest(std::size_t _page);

    /// \brief Parse model identifiers from a RESTful response
    /// \param[in] _resp RESTful response
    /// \return A vector of identifiers extracted from the response.
    protected: std::vector<ModelIdentifier> ParseIdsFromResponse(
        const RestResponse &_resp);

    /// \brief Model identifiers in the current page
    protected: std::vector<ModelIdentifier> ids;

    /// \brief Where the current iterator is in the list of ids
    protected: std::vector<ModelIdentifier>::iterator idIter;

    /// \brief Keep track of page number for pagination of response data from
    /// server.
    protected: std::size_t currentPage{0};
  };
}  // namespace gz::fuel_tools

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif  // GZ_FUEL_TOOLS_MODELITERPRIVATE_HH_
