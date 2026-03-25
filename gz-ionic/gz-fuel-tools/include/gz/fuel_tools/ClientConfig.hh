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

#ifndef GZ_FUEL_TOOLS_CLIENTCONFIG_HH_
#define GZ_FUEL_TOOLS_CLIENTCONFIG_HH_

#include <memory>
#include <string>
#include <vector>

#include <gz/common/URI.hh>

#include "gz/fuel_tools/Export.hh"
#include "gz/fuel_tools/ServerConfig.hh"

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::unique_ptr
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

namespace gz::fuel_tools
{
  /// \brief Forward Declaration
  class ClientConfigPrivate;

  /// \brief High level interface to Gazebo Fuel.
  ///
  class GZ_FUEL_TOOLS_VISIBLE ClientConfig
  {
    /// \brief Constructor.
    public: ClientConfig();

    /// \brief Copy constructor.
    /// \param[in] _copy ClientConfig to copy.
    public: ClientConfig(const ClientConfig &_copy);

    /// \brief Assignment operator overload.
    /// \param[in] _copy ClientConfig to copy.
    public: ClientConfig &operator=(const ClientConfig &_copy);

    /// \brief Destructor.
    public: ~ClientConfig();

    /// \brief Clear the client config. This will set all values to empty
    /// strings, except the user agent which will be set to its default
    /// value.
    public: void Clear();

    /// \brief Set the user agent name.
    /// \param[in] _agent User agent name.
    public: void SetUserAgent(const std::string &_agent);

    /// \brief Get the user agent name.
    /// \return Name of the user agent.
    public: const std::string &UserAgent() const;

    /// \brief Load a YAML configuration file.
    /// \param[in] _file Config file to load.
    /// \return True if the configuration was loaded correctly.
    /// \sa ConfigPath
    public: bool LoadConfig(const std::string &_file);

    /// \brief Get the location of the configuration file.
    /// \return Path to the configuration file, which is set via
    /// LoadConfig. The default return value is an empty string.
    /// \sa LoadConfig
    public: std::string ConfigPath() const;

    /// \brief List of servers the client will connect to.
    /// \return The list of servers.
    public: std::vector<ServerConfig> Servers() const;

    /// \brief List of servers the client will connect to.
    /// \return The list of servers.
    public: std::vector<ServerConfig> & MutableServers() const;

    /// \brief Add a server to the list.
    /// \param[in] _srv The server config.
    public: void AddServer(const ServerConfig &_srv);

    /// \brief Where are models and stuff stored locally?
    /// \return The location where assets are stored locally.
    public: std::string CacheLocation() const;

    /// \brief Set where models and stuff are saved.
    /// \param[in] _path path on disk where models are saved.
    public: void SetCacheLocation(const std::string &_path);

    /// \brief Returns all the client information as a string.
    /// \param[in] _prefix Optional prefix for every line of the string.
    /// \return Client information string
    public: std::string AsString(const std::string &_prefix = "") const;

    /// \brief PIMPL
    private: std::unique_ptr<ClientConfigPrivate> dataPtr;
  };
}  // namespace gz::fuel_tools

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif  // GZ_FUEL_TOOLS_CLIENTCONFIG_HH_
