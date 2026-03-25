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

#ifndef GZ_FUEL_TOOLS_WORLDIDENTIFIER_HH_
#define GZ_FUEL_TOOLS_WORLDIDENTIFIER_HH_

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
  class WorldIdentifierPrivate;
  class ServerConfig;

  /// \brief Defines how to identify a world
  class GZ_FUEL_TOOLS_VISIBLE WorldIdentifier
  {
    /// \brief Constructor.
    public: WorldIdentifier();

    /// \brief Copy Constructor.
    /// \param[in] _orig WorldIdentifier to copy.
    public: WorldIdentifier(const WorldIdentifier &_orig);

    /// \brief Destructor.
    public: ~WorldIdentifier();

    /// \brief Assignment operator
    /// \param[in] _orig WorldIdentifier to copy.
    /// \return Reference to this object.
    public: WorldIdentifier &operator=(const WorldIdentifier &_orig);

    /// \brief Equality operator.
    /// \param[in] _rhs WorldIdentifier to compare.
    /// \return True if the WorldIdentifier names are equal.
    public: bool operator==(const WorldIdentifier &_rhs) const;

    /// \brief Returns the world name
    /// \return World name.
    public: std::string Name() const;

    /// \brief Set the name of the world.
    /// \param[in] _name The name to set.
    /// \return True if successful.
    public: bool SetName(const std::string &_name);

    /// \brief Returns owner to attribute world to.
    /// \return Owner name.
    public: std::string Owner() const;

    /// \brief Set the owner of the world
    /// \param[in] _name The name to set. Must be ascii and pass [-_a-z0-9]+
    /// \return true if successful
    public: bool SetOwner(const std::string &_name);

    /// \brief Returns server information to retrieve world from.
    /// \return Server information.
    public: ServerConfig &Server() const;

    /// \brief Set the server from which this world comes.
    /// \param[in] _server The server to retrieve the world from.
    /// \return True if successful.
    public: bool SetServer(const ServerConfig &_server);

    /// \brief Returns the world's version as a number. Versions are integers
    /// counting from 1. Version zero means the tip.
    /// \return World's version.
    /// \sa VersionStr
    /// \sa SetVersion
    public: unsigned int Version() const;

    /// \brief Returns the world's version as a string. Versions are integers
    /// counting from 1. The strings "tip" or "" mean the latest version.
    /// \return World's version.
    /// \sa Version
    /// \sa SetVersionStr
    public: std::string VersionStr() const;

    /// \brief Set the world's version. Versions are integers counting from
    /// 1.
    /// \param[in] _version The world's version
    /// \return True if successful.
    /// \sa Version
    /// \sa SetVersionStr
    public: bool SetVersion(const unsigned int _version);

    /// \brief Set the world's version. Versions are integers counting from
    /// 1 or "tip" for the latest version.
    /// \param[in] _version The world's version
    /// \return True if successful.
    /// \sa VersionStr
    /// \sa SetVersion
    public: bool SetVersionStr(const std::string &_version);

    /// \brief Returns a path to the world on disk if it is already cached
    /// \returns Path, or empty string if the path has not been set
    /// \sa SetLocalPath
    public: std::string LocalPath() const;

    /// \brief Sets the path of the world in the local cache.
    /// \param[in] _path Local path to world.
    /// \sa LocalPath
    public: bool SetLocalPath(const std::string &_path);

    /// \brief Returns a unique name for the world.
    /// \remarks this is Server/Owner/Name.
    /// \return Unique world name.
    public: std::string UniqueName() const;

    /// \brief Returns a URL for the world.
    /// \remarks this is Server/Owner/Name.
    public: gz::common::URI Url() const;

    // /// \brief Sets the SHA 2 256 hash of the world
    // /// \param[in] _hash a 256 bit SHA 2 hash
    // /// \returns true if successful
    // public: bool SHA_256(const std::array<std::uint8_t, 32> &_hash);

    /// \brief Returns all the world information as a string. Convenient for
    /// debugging.
    /// \param[in] _prefix Optional prefix for every line of the string.
    /// \return World information string
    public: std::string AsString(const std::string &_prefix = "") const;

    /// \brief Returns all the available world information as a string using
    /// colors for better human parsing.
    /// \param[in] _prefix Optional prefix for every line of the string.
    /// \return World information string
    public: std::string AsPrettyString(const std::string &_prefix = "") const;

    /// \brief Returns the privacy setting of the world.
    /// \return True if the world is private, false if the world is
    /// public.
    public: bool Private() const;

    /// \brief Set the privacy setting of the world.
    /// \param[in] _private True indicates the world is private,
    /// false indicates the world is public.
    public: void SetPrivate(bool _private);

    /// \brief PIMPL
    private: std::unique_ptr<WorldIdentifierPrivate> dataPtr;
  };
}  // namespace gz::fuel_tools

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif  // GZ_FUEL_TOOLS_WORLDIDENTIFIER_HH_
