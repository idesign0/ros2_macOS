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

#ifndef GZ_FUEL_TOOLS_WORLDITERPRIVATE_HH_
#define GZ_FUEL_TOOLS_WORLDITERPRIVATE_HH_

#include <string>
#include <vector>

#include "gz/fuel_tools/ClientConfig.hh"
#include "gz/fuel_tools/WorldIdentifier.hh"
#include "gz/fuel_tools/RestClient.hh"

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::vector
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

namespace gz::fuel_tools
{
  /// \brief forward declaration
  class WorldIter;

  /// \brief Private class, do not include or instantiate
  class GZ_FUEL_TOOLS_VISIBLE WorldIterFactory
  {
    /// \brief Create a world iterator from a vector of world identifiers
    /// \param[in] _ids World identifiers
    /// \return World iterator
    public: static WorldIter Create(const std::vector<WorldIdentifier> &_ids);

    /// \brief Create a world iter that will make REST api calls
    /// \param[in] _rest a REST request
    /// \param[in] _server The server to request the operation
    /// \param[in] _path The path to request
    /// \return World iterator
    public: static WorldIter Create(const Rest &_rest,
                                    const ServerConfig &_server,
                                    const std::string &_path);

    /// \brief Create a world iterator that is empty
    /// \return An empty iterator
    public: static WorldIter Create();
  };

  /// \brief Private class, do not include or instantiate
  class GZ_FUEL_TOOLS_VISIBLE WorldIterPrivate
  {
    /// \brief Destructor
    public: virtual ~WorldIterPrivate();

    /// \brief Advance iterator to next world
    public: virtual void Next() = 0;

    /// \brief True if this iterator has reach the end
    /// \return True if reached end.
    public: virtual bool HasReachedEnd() = 0;

    /// \brief Current world for returning references
    public: WorldIdentifier worldId;
  };

  /// \brief Class for iterating through world ids
  class GZ_FUEL_TOOLS_VISIBLE WorldIterIds : public WorldIterPrivate
  {
    /// \brief Constructor
    /// \param[in] _ids Vector of ids
    public: explicit WorldIterIds(std::vector<WorldIdentifier> _ids);

    /// \brief Destructor
    public: virtual ~WorldIterIds();

    // Documentation inherited
    public: virtual void Next() override;

    // Documentation inherited
    public: virtual bool HasReachedEnd() override;

    /// \brief World identifiers that have been requested
    protected: std::vector<WorldIdentifier> ids;

    /// \brief Where the current iterator is in the list of ids
    protected: std::vector<WorldIdentifier>::iterator idIter;
  };

  /// \brief class for iterating through world ids from a rest API
  class GZ_FUEL_TOOLS_VISIBLE WorldIterRestIds: public WorldIterPrivate
  {
    /// \brief Constructor
    /// \param[in] _rest REST client
    /// \param[in] _server Server configuration
    /// \param[in] _path The path to request
    public: WorldIterRestIds(const Rest &_rest,
                             const ServerConfig &_server,
                             const std::string &_path);

    /// \brief Destructor
    public: virtual ~WorldIterRestIds();

    // Documentation inherited
    public: virtual void Next() override;

    // Documentation inherited
    public: virtual bool HasReachedEnd() override;

    /// \brief Server configuration
    public: ServerConfig config;

    /// \brief RESTful client
    public: Rest rest;

    /// \brief World identifiers in the current page
    protected: std::vector<WorldIdentifier> ids;

    /// \brief Where the current iterator is in the list of ids
    protected: std::vector<WorldIdentifier>::iterator idIter;
  };
}  // namespace gz::fuel_tools
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif  // GZ_FUEL_TOOLS_WORLDITERPRIVATE_HH_
