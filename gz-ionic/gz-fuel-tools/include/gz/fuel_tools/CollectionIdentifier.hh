/*
 * Copyright (C) 2020 Open Source Robotics Foundation
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

#ifndef GZ_FUEL_TOOLS_COLLECTIONIDENTIFIER_HH_
#define GZ_FUEL_TOOLS_COLLECTIONIDENTIFIER_HH_

#include <memory>
#include <string>

#include "gz/fuel_tools/Helpers.hh"

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::unique_ptr
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

namespace gz::fuel_tools
{
  /// \brief Forward Declaration
  class CollectionIdentifierPrivate;
  class ServerConfig;

  /// \brief Defines how to identify a collection
  class GZ_FUEL_TOOLS_VISIBLE CollectionIdentifier
  {
    /// \brief Constructor.
    public: CollectionIdentifier();

    /// \brief Copy Constructor.
    /// \param[in] _orig CollectionIdentifier to copy.
    public: CollectionIdentifier(const CollectionIdentifier &_orig);

    /// \brief Destructor.
    public: ~CollectionIdentifier();

    /// \brief Assignment operator
    /// \param[in] _orig CollectionIdentifier to copy.
    /// \return Reference to this object.
    public: CollectionIdentifier
            &operator=(const CollectionIdentifier &_orig);

      /// \brief Equality operator.
      /// \param[in] _rhs CollectionIdentifier to compare.
      /// \return True if the CollectionIdentifier names are equal.
    public: bool operator==(const CollectionIdentifier &_rhs) const;

    /// \brief Returns the collection name
    /// \return Collection name.
    public: std::string Name() const;

    /// \brief Set the name of the collection.
    /// \param[in] _name The name to set.
    /// \return True if successful.
    public: bool SetName(const std::string &_name);

    /// \brief Returns owner to attribute collection to.
    /// \return Owner name.
    public: std::string Owner() const;

    /// \brief Set the owner of the collection
    /// \param[in] _name The name to set. Must be ascii and pass [-_a-z0-9]+
    /// \return true if successful
    public: bool SetOwner(const std::string &_name);

    /// \brief Returns server information to retrieve collection from.
    /// \return Server information.
    public: ServerConfig &Server() const;

    /// \brief Set the server from which this collection comes.
    /// \param[in] _server The server to retrieve the collection from.
    /// \return True if successful.
    public: bool SetServer(const ServerConfig &_server);

    /// \brief Returns a unique name for the collection.
    /// \remarks this is Server/Owner/Name.
    /// \return Unique collection name.
    public: std::string UniqueName() const;

    /// \brief Returns a URL for the collection.
    /// \remarks this is Server/Owner/Name.
    public: gz::common::URI Url() const;

    /// \brief Returns all the collection information as a string. Convenient
    /// for debugging.
    /// \param[in] _prefix Optional prefix for every line of the string.
    /// \return Collection information string
    public: std::string AsString(const std::string &_prefix = "") const;

    /// \brief Returns all the available collection information as a string
    /// using colors for better human parsing.
    /// \param[in] _prefix Optional prefix for every line of the string.
    /// \return Collection information string
    public: std::string AsPrettyString(const std::string &_prefix = "") const;

    /// \brief PIMPL
    private: std::unique_ptr<CollectionIdentifierPrivate> dataPtr;
  };
}  // namespace gz::fuel_tools

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif  // GZ_FUEL_TOOLS_COLLECTIONIDENTIFIER_HH_
