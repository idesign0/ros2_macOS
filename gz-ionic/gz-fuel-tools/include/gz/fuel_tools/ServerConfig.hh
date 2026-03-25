/*
 * Copyright (C) 2024 Open Source Robotics Foundation
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

#ifndef GZ_FUEL_TOOLS_SERVERCONFIG_HH_
#define GZ_FUEL_TOOLS_SERVERCONFIG_HH_

#include <memory>
#include <string>

#include <gz/common/URI.hh>

#include "gz/fuel_tools/Export.hh"

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::unique_ptr
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

namespace gz::fuel_tools
{
  /// \brief forward declaration
  class ServerConfigPrivate;

  /// \brief Describes options needed for a server.
  class GZ_FUEL_TOOLS_VISIBLE ServerConfig
  {
    /// \brief Constructor.
    public: ServerConfig();

    /// \brief Copy constructor.
    /// \param[in] _orig The server config to copy.
    public: ServerConfig(const ServerConfig &_orig);

    /// \brief Assignment operator overload.
    /// \param[in] _orig The server config to copy.
    public: ServerConfig &operator=(const ServerConfig &_orig);

    /// \brief Destructor.
    public: ~ServerConfig();

    /// \brief Clear the server config. This will set all values to empty
    /// strings, except the version string which will be set to its default
    /// value.
    public: void Clear();

    /// \brief Get the URL to access the server.
    /// \return The URL of this server.
    public: common::URI Url() const;

    /// \brief Set the URL of this server.
    /// \param[in] _url URL of this server.
    public: void SetUrl(const common::URI &_url);

    /// \brief Get the API key to auth with the server.
    /// \return The API key.
    public: std::string ApiKey() const;

    /// \brief Set the API key to auth with the server.
    /// \param[in] _key The API key.
    public: void SetApiKey(const std::string &_key);

    /// \brief Get the protocol version used with this server.
    /// \return The version. E.g.: "1.0".
    public: std::string Version() const;

    /// \brief Set the protocol version used with this server.
    /// \param[in] _version The version. E.g.: "1.0".
    public: void SetVersion(const std::string &_version);

    /// \brief Returns all the server information as a string.
    /// \param[in] _prefix Optional prefix for every line of the string.
    /// \return Server information string
    public: std::string AsString(const std::string &_prefix = "") const;

    /// \brief Returns all the available model information as a string using
    /// colors for better human parsing.
    /// \param[in] _prefix Optional prefix for every line of the string.
    /// \return Model information string
    public: std::string AsPrettyString(const std::string &_prefix = "") const;

    /// \brief PIMPL
    private: std::unique_ptr<ServerConfigPrivate> dataPtr;
  };
}  // namespace gz::fuel_tools

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif  // GZ_FUEL_TOOLS_SERVERCONFIG_HH_
